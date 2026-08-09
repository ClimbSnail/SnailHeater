# -*- coding: utf-8 -*-
import io
import os
import re
import traceback
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from threading import Event, RLock
from typing import Callable, List, Optional, Tuple
import esptool
from esptool import loader
import requests
import common
from .config import ToolConfig
from .models import DownloadMode, FlashPlan, FlashResult, OperationState
from .paths import RuntimePaths
from .profiles import CHIP_ID_S2, CHIP_ID_S3, ProductProfile

loader.STUBS_DIR = str(RuntimePaths.discover().stub_flasher_dir)
LogCallback = Callable[[str], None]

_ESPTOOL_RUN_LOCK = RLock()


class EsptoolLogWriter(io.TextIOBase):
    """把 esptool 的 stdout/stderr 按行转发给日志回调。"""

    def __init__(self, callback: Optional[LogCallback], capture: io.StringIO):
        """
        初始化对象状态和运行所需的依赖。

        Args:
            callback: 结果回调函数。
            capture: 输出捕获配置。

        """
        super().__init__()
        self._callback = callback
        self._capture = capture
        self._pending = ""

    def write(self, text: str) -> int:
        """
        向日志回调输出一条文本消息。

        Args:
            text: 待输出文本。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if not text:
            return 0
        self._capture.write(text)
        self._pending += text.replace("\r", "\n")
        while "\n" in self._pending:
            line, self._pending = self._pending.split("\n", 1)
            self._emit(line)
        return len(text)

    def flush(self) -> None:
        if self._pending:
            self._emit(self._pending)
            self._pending = ""

    def _emit(self, line: str) -> None:
        cleaned = line.replace("\b", "").strip()
        if cleaned and self._callback:
            self._callback(cleaned)


class FirmwareService:
    """固件扫描、设备探测、刷机计划及 esptool 执行。"""

    def __init__(self, config: ToolConfig, profile: ProductProfile, paths: RuntimePaths):
        self.config = config
        self.profile = profile
        self.paths = paths

    def firmware_dir(self) -> Path:
        configured_path = Path(self.config.main_appdir_rules or ".")
        if configured_path.is_absolute():
            return configured_path

        # 打包后 YAML 位于 exe 同目录，固件也通常由用户放在该目录或其子目录。
        # 优先使用该外部路径，缺失时仍可兼容被打进包内的资源目录。
        external_directory = (self.paths.external_root / configured_path).resolve()
        return external_directory if external_directory.is_dir() else self.paths.resolve(str(configured_path))

    def scan_firmware(self) -> List[str]:
        """
        扫描可用的固件文件。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        directory = self.firmware_dir()
        if not directory.is_dir():
            return []
        pattern = re.compile(self.config.main_app_rules)
        return sorted((name for name in os.listdir(str(directory)) if pattern.search(name)))

    def firmware_path(self, filename: str) -> str:
        return str(self.firmware_dir() / filename)

    def get_tool_version_text(self, state: OperationState) -> str:
        """
        根据当前状态生成工具版本提示文本。

        Args:
            state: 当前运行状态。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            if not self.config.get_tool_new_ver_url:
                return "[未配置版本服务]"
            response = requests.get(
                self.config.get_tool_new_ver_url.rstrip("/") + "/" + common.TOOL_VERSION, timeout=3
            )
            response.raise_for_status()
            response_text = response.text.strip()
            # 服务端按“当前工具版本”路由查询。它返回 unknown 时并没有提供最新版本，
            # 不能误报成网络故障；这通常意味着服务端尚未收录当前发布版。
            if re.search(r"\b(?:this\s+)?version\s+unknown\b", response_text, re.IGNORECASE):
                return f"[版本服务未收录当前版本 {common.TOOL_VERSION}]"
            new_version_info = re.findall("SH_TOOL v\d{1,2}\.\d{1,2}\.\d{1,2}", response_text)
            if not new_version_info:
                raise ValueError(f"版本服务返回格式不正确：{response_text[:80]}")
            new_version = new_version_info[-1].split(" ")[1].strip()
            bracket = re.search("\\[([^]]+~[^]]+)\\]", response.text)
            if bracket:
                start_text, end_text = bracket.group(1).split("~", 1)
                state.support_versions = [
                    start_text.strip().split(" ")[-1],
                    end_text.strip().split(" ")[-1],
                ]
            return (
                "[已是最新版本]"
                if common.TOOL_VERSION == new_version
                else f"[推荐升级最新版本 {new_version}]"
            )
        except Exception:
            print(traceback.format_exc())
            return "[无法获取到最新版本]"

    def get_latest_firmware_text(self) -> Optional[str]:
        """
        获取并生成最新固件版本提示文本。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if not self.config.get_firmware_new_ver_url:
            return None
        response = requests.get(self.config.get_firmware_new_ver_url, timeout=3)
        response.raise_for_status()
        text = response.text.strip()
        if self.profile.is_el:
            return text if "SEL" in text else None
        return text if "SnailHeater_v" in text or "SH_SW v" in text else None

    @staticmethod
    def _run_esptool(args: List[str], log: Optional[LogCallback] = None) -> str:
        """执行 esptool，并把标准输出和错误输出转发到日志回调。"""
        output = io.StringIO()
        writer = EsptoolLogWriter(log, output)
        with _ESPTOOL_RUN_LOCK:
            try:
                with redirect_stdout(writer), redirect_stderr(writer):
                    esptool.main(args)
            finally:
                writer.flush()
        return output.getvalue()

    def get_flash_size(
        self, port: str, log: Optional[LogCallback] = None
    ) -> Tuple[int, str]:
        """
        读取设备可用的闪存容量。

        Args:
            port: 串口名称或串口对象。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        text = self._run_esptool(["--port", port, "flash_id"], log)
        match = re.search("Detected flash size:\\s*(\\d+)MB", text)
        if not match:
            return (0, "0MB")
        size_mb = int(match.group(1))
        return (size_mb * 1024 * 1024, f"{size_mb}MB")

    def get_chip_id(self, port: str, log: Optional[LogCallback] = None) -> str:
        """
        读取并识别设备芯片型号。

        Args:
            port: 串口名称或串口对象。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        text = self._run_esptool(["--port", port, "chip_id"], log)
        upper = text.upper()
        if "ESP32-S2" in upper or "ESP32S2" in upper:
            return CHIP_ID_S2
        if "ESP32-S3" in upper or "ESP32S3" in upper:
            return CHIP_ID_S3
        return ""

    def prepare_plan(
        self,
        port: str,
        firmware_path: str,
        mode: DownloadMode,
        state: OperationState,
        log: Optional[LogCallback] = None,
    ) -> FlashPlan:
        """
        根据设备和固件信息准备刷写计划。

        Args:
            port: 串口名称或串口对象。
            firmware_path: 固件路径。
            mode: 运行模式。
            state: 当前运行状态。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        chip_id = self.get_chip_id(port, log)
        flash_size, flash_size_text = self.get_flash_size(port, log)
        if flash_size == 0:
            raise RuntimeError("储存空间为 0，无法继续刷机")
        if self.profile.is_el and flash_size < 8 * 1024 * 1024:
            raise RuntimeError(f"EL 不支持该 Flash 容量：{flash_size_text}")
        state.current_chip_id = chip_id
        return self.profile.build_flash_plan(
            port=port,
            baud_rate=self.config.baud_rate,
            chip_id=chip_id,
            flash_size_text=flash_size_text,
            firmware_path=firmware_path,
            mode=mode,
            state=state,
            paths=self.paths,
        )

    def write_entries(
        self,
        port: str,
        entries,
        flash_size: Optional[str] = None,
        log: Optional[LogCallback] = None,
    ) -> None:
        """
        按刷写条目将数据写入设备闪存。

        Args:
            port: 串口名称或串口对象。
            entries: 刷写条目序列。
            flash_size: 闪存容量。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        args = [
            "--port",
            port,
            "--baud",
            self.config.baud_rate,
            "--after",
            "hard_reset",
            "write_flash",
        ]
        if flash_size:
            args.extend(["--flash_size", flash_size])
        for entry in entries:
            path = Path(entry.path)
            if not path.is_file():
                raise FileNotFoundError(f"缺少待烧录文件：{path}")
            args.extend([entry.address, str(path)])
        self._run_esptool(args, log)

    def read_flash(
        self,
        port: str,
        address: str,
        size: str,
        output: str,
        log: Optional[LogCallback] = None,
    ) -> None:
        """
        从设备闪存读取指定范围的数据。

        Args:
            port: 串口名称或串口对象。
            address: 闪存起始地址。
            size: 读取长度。
            output: 输出文件路径。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        Path(output).parent.mkdir(parents=True, exist_ok=True)
        self._run_esptool(
            ["--port", port, "read_flash", address, size, output], log
        )

    def execute_plan(self, plan: FlashPlan, cancel_event: Event, log: LogCallback) -> FlashResult:
        """
        执行完整的固件或资源刷写计划。

        Args:
            plan: 待执行的刷写计划。
            cancel_event: 取消事件对象。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            if cancel_event.is_set():
                return FlashResult(False, "刷机已取消", cancelled=True)
            missing = [entry.path for entry in plan.entries if not Path(entry.path).is_file()]
            if missing:
                preview = "；".join(missing[:3])
                suffix = "……" if len(missing) > 3 else ""
                raise FileNotFoundError(f"缺少刷机资源：{preview}{suffix}")
            if plan.erase_region:
                if cancel_event.is_set():
                    return FlashResult(False, "刷机已取消", cancelled=True)
                log("正在清空主机数据...")
                self._run_esptool(
                    ["--port", plan.port, "erase_region", *plan.erase_region], log
                )
                log("完成清空！")
            if cancel_event.is_set():
                return FlashResult(False, "刷机已取消", cancelled=True)
            log("开始刷写固件...")
            self._run_esptool(plan.to_esptool_args(), log)
            if cancel_event.is_set():
                return FlashResult(False, "当前刷写步骤已完成，后续操作已取消", cancelled=True)
            if self.profile.is_el:
                log("刷机流程完毕，请手动开机。")
            else:
                log("刷机流程完毕，请保持 Type-C 通电，等待设备屏幕亮起后再断电。")
                log(
                    "更新式通常约 2 秒亮屏，清空式约 10 秒；如长时间未亮屏，请复位或重新插拔 Type-C。"
                )
            return FlashResult(True, "刷机结束！")
        except Exception as error:
            return FlashResult(False, str(error), type(error).__name__)
