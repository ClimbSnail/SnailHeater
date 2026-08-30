# -*- coding: utf-8 -*-
"""现代 WebUI 调用的产品业务编排层，复用现有硬件与媒体服务。"""

from __future__ import annotations

import datetime
import os
import re
import shutil
import time
from pathlib import Path
from typing import Any, Callable

import common
import massagehead as mh

from ..activation_service import ActivationService
from ..device_service import DeviceService
from ..firmware_service import FirmwareService
from ..media_service import MediaOptions, MediaService
from ..models import DownloadMode, FlashEntry, FlashResult
from ..paths import RuntimePaths
from ..profiles import CHIP_ID_KNOWN, CHIP_ID_S2, CHIP_ID_S3, EL_PROFILE, SNAILHEATER_PROFILE, ProductProfile
from .operations import Operation, OperationManager

PROFILES: dict[str, ProductProfile] = {
    "snailheater": SNAILHEATER_PROFILE,
    "el": EL_PROFILE,
}


class WebToolSession:
    """面向单个产品的无界面应用服务，负责协调现有业务模块。"""

    def __init__(self, product: str) -> None:
        """加载指定产品的配置，并初始化可复用的设备、固件与媒体服务。"""
        if product not in PROFILES:
            raise ValueError(f"不支持的产品类型：{product}")
        self.profile = PROFILES[product]
        self.paths = RuntimePaths.discover()
        self.config = self.profile.load_config(self.paths)
        self.state = self.profile.initial_state(self.paths)
        self.firmware_service = FirmwareService(self.config, self.profile, self.paths)
        self.device_service = DeviceService(self.config.info_baud_rate, self.config.baud_rate)
        self.activation_service = ActivationService(self.config, self.paths)
        self.media_service = MediaService(
            self.paths, self.profile, background_lvgl_version=9 if product == "el" else 8
        )
        self.media_service.ensure_directories()
        self.default_wallpaper_clean = self.profile.wallpaper_clean_path(self.paths)
        self.operations = OperationManager()

    @property
    def product_name(self) -> str:
        """返回当前产品在界面中展示的稳定标识。"""
        return self.profile.name

    def bootstrap(self) -> dict[str, Any]:
        """返回前端初始化页面、功能开关和产品元数据所需的信息。"""
        return {
            "product": self.profile.name,
            "toolName": self.config.tool_name.strip(),
            "toolVersion": common.TOOL_VERSION,
            "resolutions": list(self.profile.resolutions),
            "supportsActivation": self.profile.auto_activation,
            "supportsRtttl": self.profile.supports_rtttl,
            "defaultPage": "firmware" if self.profile.is_el else "device",
            "links": {"guide": self.config.info_url_0.to_dict(), "updates": self.config.info_url_1.to_dict()},
            "qq": self.config.qq_pair,
            "state": {"chipId": self.state.current_chip_id, "downloadMode": self.state.download_mode.value},
        }

    def list_ports(self) -> list[dict[str, str]]:
        """列出当前可用串口，并转换为前端易于渲染的结构。"""
        return [
            {"name": name, "description": description, "display": f"{name} -> {description}"}
            for name, description in self.device_service.list_ports()
        ]

    def scan_firmware(self) -> list[str]:
        """扫描当前产品规则下可选的固件文件。"""
        return self.firmware_service.scan_firmware()

    def latest_versions(self) -> dict[str, str | None]:
        """查询工具与固件版本提示，并把网络错误转换为可显示文本。"""
        try:
            tool = self.firmware_service.get_tool_version_text(self.state)
        except Exception as error:
            tool = f"无法查询工具版本：{error}"
        try:
            firmware = self.firmware_service.get_latest_firmware_text()
        except Exception as error:
            firmware = f"无法查询固件版本：{error}"
        return {"tool": tool, "firmware": firmware}

    def probe_hardware(self, port: str) -> dict[str, Any]:
        """通过 esptool 识别芯片型号与闪存容量，并更新当前会话状态。"""
        port = self._require_port(port)
        chip_id = self.firmware_service.get_chip_id(port)
        flash_bytes, flash_text = self.firmware_service.get_flash_size(port)
        self.state.current_chip_id = chip_id or CHIP_ID_KNOWN
        return {
            "port": port,
            "chipId": chip_id or "未知",
            "flashBytes": flash_bytes,
            "flashSize": flash_text,
        }

    def query_device(self, port: str) -> dict[str, Any]:
        """读取机器码并查询对应的在线激活信息。"""
        port = self._require_port(port)
        machine_code = self._read_machine_code(port)
        if not machine_code:
            raise ValueError("设备未返回有效的机器码")

        activation: dict[str, str] | None = None
        activation_error: str | None = None
        try:
            info = self.activation_service.query(machine_code)
            activation = {"sn": info.sn, "registrant": info.registrant}
            if info.sn:
                self.activation_service.append_cache(machine_code, info.sn)
        except Exception as error:
            activation_error = str(error) or "联网查询激活码失败"

        return {
            "port": port,
            "machineCode": machine_code,
            "activation": activation,
            "activationError": activation_error,
        }

    def flash_info(self, port: str) -> dict[str, Any]:
        """识别当前设备芯片型号和闪存容量，并同步更新会话状态。"""
        port = self._require_port(port)
        chip_id = self.firmware_service.get_chip_id(port)
        flash_size, flash_size_name = self.firmware_service.get_flash_size(port)
        self.state.current_chip_id = chip_id or CHIP_ID_KNOWN
        return {"port": port, "chipId": chip_id, "flashSize": flash_size, "flashSizeName": flash_size_name}

    def activate(self, port: str, sn: str) -> dict[str, Any]:
        """向设备写入激活码，并返回明确的设备响应状态。"""
        if not sn.strip():
            raise ValueError("请输入有效的 SN 激活码")
        port = self._require_port(port)
        success = self.device_service.set_value(port, mh.VT.VALUE_TYPE_SN, sn.strip())
        return {"port": port, "success": success, "message": "激活成功" if success else "设备未确认激活"}

    def read_colors(self, port: str) -> dict[str, str]:
        """读取设备的普通与按下状态前景颜色。"""
        value = self._query_value(
            self._require_port(port),
            mh.VT.VALUE_TYPE_FORWARD_COLOR,
            r"VALUE_TYPE_FORWARD_COLOR = (\S+ \S+)",
        )
        colors = value.split()
        if len(colors) != 2:
            raise ValueError("设备未返回有效的文本颜色配置")
        return {"normal": format(int(colors[0], 0), "06X"), "pressed": format(int(colors[1], 0), "06X")}

    def write_colors(self, port: str, normal: str, pressed: str) -> dict[str, Any]:
        """校验并写入设备的两种 UI 前景颜色。"""
        normal, pressed = normal.strip().upper(), pressed.strip().upper()
        if not re.fullmatch(r"[0-9A-F]{6}", normal) or not re.fullmatch(r"[0-9A-F]{6}", pressed):
            raise ValueError("RGB 颜色必须是 6 位十六进制，例如 7AFE89")
        success = self.device_service.set_value(
            self._require_port(port), mh.VT.VALUE_TYPE_FORWARD_COLOR, f"{normal} {pressed}"
        )
        return {"success": success, "normal": normal, "pressed": pressed}

    def hard_reset(self, port: str) -> dict[str, str]:
        """对指定设备执行硬复位，并返回实际使用的串口。"""
        port = self._require_port(port)
        self.device_service.hard_reset(port)
        return {"port": port, "message": "已发送硬复位信号"}

    def start_operation(self, kind: str, payload: dict[str, Any]) -> Operation:
        """根据前端请求创建媒体、刷机或诊断类后台操作。"""
        handlers: dict[str, Callable[[Operation], Any]] = {
            "flash": lambda operation: self._flash(payload, operation),
            "background": lambda operation: self._write_background(payload, operation),
            "wallpaper": lambda operation: self._write_wallpaper(payload, operation),
            "clean-wallpaper": lambda operation: self._clean_wallpaper(payload, operation),
            "convert-media": lambda operation: self._convert_media(payload, operation),
            "convert-rtttl": lambda operation: self._convert_rtttl(payload, operation),
            "pack-wallpaper": lambda operation: self._pack_wallpaper(payload, operation),
            "coredump": lambda operation: self._read_coredump(payload, operation),
        }
        if kind not in handlers:
            raise ValueError(f"不支持的操作类型：{kind}")
        return self.operations.start(kind, handlers[kind])

    def _flash(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """构建刷机计划并使用现有固件服务执行协作式刷写。"""
        port = self._require_port(str(payload.get("port", "")))
        firmware_name = str(payload.get("firmware", "")).strip()
        if not firmware_name or firmware_name == "未找到固件":
            raise ValueError("请选择有效固件")
        firmware_path = self.firmware_service.firmware_path(firmware_name)
        if not os.path.isfile(firmware_path):
            raise FileNotFoundError(f"固件文件不存在：{firmware_path}")
        mode = DownloadMode.CLEAR if payload.get("mode") == DownloadMode.CLEAR.value else DownloadMode.UPDATE
        self.state.download_mode = mode
        wallpaper, background = self.profile.choose_default_media(firmware_name, self.paths)
        self.state.default_wallpaper, self.state.default_background = wallpaper, background
        if not os.path.isfile(self.state.default_wallpaper):
            self.state.default_wallpaper = self.default_wallpaper_clean
        if not self.profile.is_el and self.state.support_versions:
            current = self.profile.firmware_version(firmware_name)
            start, end = self.state.support_versions
            if common.getVerValue(current) < common.getVerValue(start) or common.getVerValue(current) > common.getVerValue(end):
                raise ValueError("当前版本管理工具不支持该固件")
        self.state.auto_activate = self.profile.auto_activation and "Pro" not in firmware_name
        self._log(operation, f"串口号：{port}")
        self._log(operation, f"固件文件：{firmware_name}")
        self._log(operation, f"刷机模式：{mode.value}")
        plan = self.firmware_service.prepare_plan(port, firmware_path, mode, self.state, self._logger(operation))
        self._log(operation, f"已识别芯片 {plan.chip_id or '未知'}，Flash 容量 {plan.flash_size}")
        result: FlashResult = self.firmware_service.execute_plan(plan, operation.cancel_event, self._logger(operation))
        self.state.current_chip_id = CHIP_ID_KNOWN
        if not result.success:
            if result.cancelled:
                operation.cancel_event.set()
            raise RuntimeError(result.message or "刷机失败")
        if mode == DownloadMode.CLEAR and self.state.auto_activate and self.profile.auto_activation:
            self._auto_activate(port, operation)
        return {"success": True, "chipId": plan.chip_id, "flashSize": plan.flash_size, "message": result.message}

    def _auto_activate(self, port: str, operation: Operation) -> None:
        """在清空式刷机完成后尝试自动查询并写入激活码。"""
        self._log(operation, "刷机完成，等待设备重启后尝试自动激活……")
        time.sleep(2)
        if operation.cancel_event.is_set():
            return
        machine_code = self._read_machine_code(port)
        if not machine_code:
            self._log(operation, "未读取到机器码，跳过自动激活。")
            return
        info = self.activation_service.query(machine_code)
        if not info.sn:
            self._log(operation, "未查询到激活码，请稍后手动激活。")
            return
        success = self.device_service.set_value(port, mh.VT.VALUE_TYPE_SN, info.sn)
        self._log(operation, "自动激活成功。" if success else "自动激活失败，请稍后手动激活。")

    def _write_background(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """转换背景图片并写入当前芯片对应的背景闪存区域。"""
        port = self._require_port(str(payload.get("port", "")))
        chip_id = self.firmware_service.get_chip_id(port, self._logger(operation))
        capacity = int(self.profile.backgroundSize[chip_id], 16)
        params = self._media_params(payload)
        background = self.media_service.prepare_background(
            params, bool(payload.get("cropToFill", True)), self._logger(operation), capacity
        )
        rate = self.media_service.validate_capacity(background, capacity)
        address = self.profile.background_address(chip_id)
        self._log(operation, f"正在将背景写入 {address}……")
        self.firmware_service.write_entries(
            port, [FlashEntry(address, str(background))], log=self._logger(operation)
        )
        self._log(operation, f"背景已写入 {address}，设备将自动复位加载新背景。")
        return {"success": True, "path": str(background), "usage": rate, "chipId": chip_id, "address": address}

    def _write_wallpaper(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """处理壁纸素材、计算容量并写入当前芯片的壁纸区域。"""
        port = self._require_port(str(payload.get("port", "")))
        chip_id = self.firmware_service.get_chip_id(port, self._logger(operation))
        flash_size_real, _ = self.firmware_service.get_flash_size(port, self._logger(operation))
        flash_size_max = 16 * 1024 * 1024 if chip_id == CHIP_ID_S2 else 32 * 1024 * 1024
        if not flash_size_real or chip_id not in {CHIP_ID_S2, CHIP_ID_S3}:
            raise RuntimeError("无法识别芯片或 Flash 容量")
        capacity = min(flash_size_real, flash_size_max) - (int(self.profile.wallpaper_address(chip_id), 16) + 50)
        params = self._media_params(payload)
        if params["format"][0] == "lsw":
            shutil.copy(params["src_path"][0], self.paths.wallpaper_file)
            self._log(operation, "正在使用已打包好的壁纸文件")
        else:
            self.media_service.convert(params, bool(payload.get("cropToFill", True)), self._logger(operation))
            self.media_service.pack_wallpaper(params, self._logger(operation))
        rate = self.media_service.validate_capacity(self.paths.wallpaper_file, capacity)
        self.firmware_service.write_entries(
            port,
            [FlashEntry(self.profile.wallpaper_address(chip_id), str(self.paths.wallpaper_file))],
            log=self._logger(operation),
        )
        return {"path": str(self.paths.wallpaper_file), "usage": rate, "capacity": capacity, "chipId": chip_id}

    def _clean_wallpaper(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """写入默认空壁纸文件以清除设备中的现有壁纸数据。"""
        port = self._require_port(str(payload.get("port", "")))
        chip_id = self.firmware_service.get_chip_id(port, self._logger(operation))
        self.firmware_service.write_entries(
            port,
            [FlashEntry(self.profile.wallpaper_address(chip_id), self.default_wallpaper_clean)],
            log=self._logger(operation),
        )
        return {"success": True, "chipId": chip_id}

    def _convert_media(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """仅转换素材而不执行任何设备写入操作。"""
        params = self._media_params(payload)
        self.media_service.convert(params, bool(payload.get("cropToFill", True)), self._logger(operation))
        return {"files": list(params["dst_path"])}

    def _convert_rtttl(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """执行与旧 Qt 工具相同的 MP4→WAV→RTTTL 专用转换。"""
        source = str(payload.get("source") or "").strip()
        if not source:
            raise ValueError("请先选择一个 MP4 视频文件")
        output = self.media_service.convert_mp4_to_rtttl(source, self._logger(operation))
        return {"path": str(output)}

    def _pack_wallpaper(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """转换选中的壁纸素材并打包为 LSW 文件。"""
        params = self._media_params(payload)
        logger = self._logger(operation)
        if params["format"][0] == "lsw":
            source = Path(params["src_path"][0])
            if source.resolve() != self.paths.wallpaper_file.resolve():
                shutil.copy(source, self.paths.wallpaper_file)
            self._log(operation, "正在使用已打包好的壁纸文件")
            result = self.paths.wallpaper_file
        else:
            self.media_service.convert(params, bool(payload.get("cropToFill", True)), logger)
            result = self.media_service.pack_wallpaper(params, logger)
        return {"path": str(result)}

    def _read_coredump(self, payload: dict[str, Any], operation: Operation) -> dict[str, Any]:
        """通过串口读取当前芯片的 coredump 分区并保存到本地。"""
        port = self._require_port(str(payload.get("port", "")))
        chip_id = self.firmware_service.get_chip_id(port, self._logger(operation))
        if chip_id not in self.profile.coredump or chip_id not in self.profile.coredumpSize:
            raise ValueError(f"当前芯片不支持读取 Coredump：{chip_id or '未知'}")
        output = self.paths.coredump_dir / f"{datetime.datetime.now():%Y%m%d_%H%M%S}.coredump"
        self._log(operation, "正在读取 Coredump，请等待……")
        self.firmware_service.read_flash(
            port,
            self.profile.coredump[chip_id],
            self.profile.coredumpSize[chip_id],
            str(output),
            self._logger(operation),
        )
        self._log(operation, f"Coredump 已保存至：{output}")
        return {"path": str(output), "chipId": chip_id}

    def _media_params(self, payload: dict[str, Any]) -> dict[str, object]:
        """验证 WebUI 媒体选项并复用现有媒体参数构造逻辑。"""
        files = [str(item).strip() for item in payload.get("files", []) if str(item).strip()]
        if not files:
            raise ValueError("请先选择至少一个媒体文件")
        options = MediaOptions(
            resolution=str(payload.get("resolution") or self.profile.resolutions[0]),
            start_time=int(payload.get("startTime", 0)),
            end_time=int(payload.get("endTime", 0)),
            fps=str(payload.get("fps", "20")),
            quality=str(payload.get("quality", "5")),
        )
        return self.media_service.build_output_params(";".join(files) + ";", options)

    def _require_port(self, selected: str) -> str:
        """校验前端传入的串口仍存在，避免把任意文本传给硬件层。"""
        port = self.device_service.validate_port(selected)
        if port:
            return port
        if self.state.current_chip_id == CHIP_ID_S3 and len(self.device_service.list_ports()) == 1:
            return self.device_service.list_ports()[0][0]
        raise ValueError("未检测到指定串口，请检查设备连接和驱动")

    def _read_machine_code(self, port: str) -> str:
        """读取机器码，并在设备返回空值时返回空字符串。"""
        return self._query_value(port, mh.VT.VALUE_TYPE_MC, r"VALUE_TYPE[_MC]* = (\d*)")

    def _query_value(self, port: str, value_type: int, pattern: str) -> str:
        """包装底层串口读取，使页面调用获得一致的字符串结果。"""
        return self.device_service.query_value(port, value_type, pattern)

    @staticmethod
    def _logger(operation: Operation) -> Callable[[str], None]:
        """生成将底层服务日志桥接到 SSE 事件的回调函数。"""
        return lambda message: WebToolSession._log(operation, message)

    @staticmethod
    def _log(operation: Operation, message: object) -> None:
        """规范化底层日志文本并推送到对应的操作事件流。"""
        text = re.sub(r"<[^>]+>", "", str(message)).strip()
        if text:
            operation.emit("log", level="info", message=text)







