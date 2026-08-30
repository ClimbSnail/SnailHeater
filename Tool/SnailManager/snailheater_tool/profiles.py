# -*- coding: utf-8 -*-
import re
from dataclasses import dataclass
from pathlib import Path
from typing import List, Mapping, Tuple
from common import getVerValue
from .config import ToolConfig, load_tool_config
from .models import DownloadMode, FlashEntry, FlashPlan, OperationState
from .paths import RuntimePaths

CHIP_ID_KNOWN = ""
CHIP_ID_S2 = "S2"
CHIP_ID_S3 = "S3"


@dataclass(frozen=True)
class ProductProfile:
    name: str
    config_filename: str
    firmware_version_pattern: str
    supports_rtttl: bool
    auto_activation: bool
    resolutions: Tuple[str, ...]
    bootloaderAddr: Mapping[str, str]
    tableAddr: Mapping[str, str]
    otaInitAddr: Mapping[str, str]
    userFwAddr: Mapping[str, str]
    backgroundAddr: Mapping[str, str]
    backgroundSize: Mapping[str, str]
    wallpaperAddr: Mapping[str, str]
    coredump: Mapping[str, str]
    coredumpSize: Mapping[str, str]

    @property
    def is_el(self) -> bool:
        return self.name == "el"

    def background_address(self, chip_id: str) -> str:
        return self.backgroundAddr[chip_id]

    def wallpaper_address(self, chip_id: str) -> str:
        return self.wallpaperAddr[chip_id]

    def load_config(self, paths: RuntimePaths) -> ToolConfig:
        return load_tool_config(self.config_filename, paths)

    def firmware_version(self, firmware_path: str) -> str:
        """
        计算当前配置对应的固件版本标识。

        Args:
            firmware_path: 固件路径。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        match = re.search(self.firmware_version_pattern, str(firmware_path))
        if not match:
            raise ValueError(f"无法从固件文件名解析版本：{firmware_path}")
        value = match.group(0)
        version = re.search("v\\d{1,2}\\.\\d{1,2}\\.\\d{1,2}", value)
        if not version:
            raise ValueError(f"固件版本格式错误：{firmware_path}")
        return version.group(0)

    def initial_state(self, paths: RuntimePaths) -> OperationState:
        """
        构造产品的初始运行状态。

        Args:
            paths: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if self.is_el:
            background = paths.external_resource_dir("base_data") / "Backgroud_320x240.bin"
            wallpaper = paths.external_resource_dir("base_data") / "Wallpaper_320x240.lsw"
        else:
            background = paths.external_resource_dir("base_data_new") / "Backgroud_280x240.bin"
            wallpaper = paths.external_resource_dir("base_data_new") / "Wallpaper_280x240.lsw"
        return OperationState(
            current_chip_id=CHIP_ID_KNOWN,
            auto_activate=self.auto_activation,
            download_mode=DownloadMode.UPDATE,
            default_wallpaper=str(wallpaper),
            default_background=str(background),
        )

    def wallpaper_clean_path(self, paths: RuntimePaths) -> str:
        directory = "base_data" if self.is_el else "base_data_new"
        return str(paths.external_resource_dir(directory) / "WallpaperClean.lsw")

    def choose_default_media(self, firmware_name: str, paths: RuntimePaths) -> Tuple[str, str]:
        """
        选择产品默认使用的媒体资源。

        Args:
            firmware_name: 对应的输入参数。
            paths: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if self.is_el:
            return (
                str(paths.external_resource_dir("base_data") / "Wallpaper_320x240.lsw"),
                str(paths.external_resource_dir("base_data") / "Backgroud_320x240.bin"),
            )
        if "③" in firmware_name:
            return (
                str(paths.external_resource_dir("base_data_new") / "Wallpaper_320x240.lsw"),
                str(paths.external_resource_dir("base_data_new") / "Backgroud_320x240.bin"),
            )
        if "①" in firmware_name or "②" in firmware_name:
            return (
                str(paths.external_resource_dir("base_data_new") / "Wallpaper_280x240.lsw"),
                str(paths.external_resource_dir("base_data_new") / "Backgroud_280x240.bin"),
            )
        return (
            self.wallpaper_clean_path(paths),
            str(paths.external_resource_dir("base_data_new") / "Backgroud_280x240.bin"),
        )

    def build_flash_plan(
        self,
        port: str,
        baud_rate: str,
        chip_id: str,
        flash_size_text: str,
        firmware_path: str,
        mode: DownloadMode,
        state: OperationState,
        paths: RuntimePaths,
    ) -> FlashPlan:
        """
        根据产品配置构建完整刷写计划。

        Args:
            port: 串口名称或串口对象。
            baud_rate: 对应的输入参数。
            chip_id: 对应的输入参数。
            flash_size_text: 对应的输入参数。
            firmware_path: 固件路径。
            mode: 运行模式。
            state: 当前运行状态。
            paths: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        version = self.firmware_version(firmware_path)
        entries: List[FlashEntry] = []
        if self.is_el:
            if chip_id != CHIP_ID_S3:
                raise ValueError(f"EL 固件仅支持 S3，当前芯片：{chip_id or '未知'}")
            normalized_size = (
                flash_size_text if flash_size_text in {"4MB", "8MB", "16MB", "32MB"} else "32MB"
            )
            base = paths.external_resource_dir("base_data")
            entries.extend(
                [
                    FlashEntry(self.bootloaderAddr[chip_id], str(base / f"bootloader_{normalized_size}.bin")),
                    FlashEntry(self.tableAddr[chip_id], str(base / f"partition-table_{normalized_size}.bin")),
                    FlashEntry(self.otaInitAddr[chip_id], str(base / f"ota_data_initial_{normalized_size}.bin")),
                    FlashEntry(self.userFwAddr[chip_id], str(firmware_path)),
                ]
            )
        else:
            normalized_size = (
                flash_size_text
                if flash_size_text in {"4MB", "8MB", "16MB", "32MB", "64MB"}
                else "16MB"
            )
            if getVerValue(version) > getVerValue("v2.1.17"):
                partition_number = 1 if getVerValue(version) > getVerValue("v2.5.30") else 0
                base = paths.external_resource_dir("base_data_new")
                if chip_id not in {CHIP_ID_S2, CHIP_ID_S3}:
                    raise ValueError(f"暂不支持的芯片：{chip_id or '未知'}")
                boot_address = self.bootloaderAddr[chip_id]
                entries.extend(
                    [
                        FlashEntry(
                            boot_address, str(base / f"{chip_id}_bootloader_{normalized_size}.bin")
                        ),
                        FlashEntry(
                            self.tableAddr[chip_id],
                            str(
                                base
                                / f"{chip_id}_partitions_{normalized_size}_{partition_number}.bin"
                            ),
                        ),
                        FlashEntry(self.otaInitAddr[chip_id], str(base / f"{chip_id}_ota_data_initial.bin")),
                        FlashEntry(self.userFwAddr[chip_id], str(firmware_path)),
                    ]
                )
            elif getVerValue(version) > getVerValue("v1.9.8"):
                base = paths.external_resource_dir("base_data_2117")
                entries.extend(
                    [
                        FlashEntry(
                            self.bootloaderAddr["legacy"], str(base / f"{chip_id}_bootloader_{normalized_size}.bin")
                        ),
                        FlashEntry(
                            self.tableAddr["legacy"], str(base / f"{chip_id}_partitions_{normalized_size}.bin")
                        ),
                        FlashEntry(self.otaInitAddr["legacy"], str(base / f"{chip_id}_ota_data_initial.bin")),
                        FlashEntry(self.userFwAddr["legacy"], str(firmware_path)),
                        FlashEntry(
                            self.wallpaperAddr[chip_id],
                            state.default_wallpaper.replace("base_data_new", "base_data_2117"),
                        ),
                    ]
                )
            else:
                raise ValueError(f"不支持的旧固件版本：{version}")
        is_legacy_snailheater = not self.is_el and getVerValue(version) <= getVerValue("v2.1.17")
        if mode == DownloadMode.CLEAR and (not is_legacy_snailheater):
            entries.extend(
                [
                    FlashEntry(self.backgroundAddr[chip_id], state.default_background),
                    FlashEntry(self.wallpaperAddr[chip_id], state.default_wallpaper),
                ]
            )
        return FlashPlan(
            port=port,
            baud_rate=baud_rate,
            flash_size=normalized_size,
            firmware_path=str(firmware_path),
            mode=mode,
            entries=entries,
            erase_region=("0x00000", "0x400000") if mode == DownloadMode.CLEAR else None,
            chip_id=chip_id,
            firmware_version=version,
        )


SNAILHEATER_PROFILE = ProductProfile(
    name="snailheater",
    config_filename="SnailHeater_Tool.yaml",
    firmware_version_pattern="(?:SnailHeater_v|SH_SW_v)\\d{1,2}\\.\\d{1,2}\\.\\d{1,2}",
    supports_rtttl=True,
    auto_activation=True,
    resolutions=("280x240 (一、二车)", "320x240 (三车/三车Pro)"),
    bootloaderAddr={CHIP_ID_S2: "0x00001000", CHIP_ID_S3: "0x00000000", "legacy": "0x00001000"},
    tableAddr={CHIP_ID_S2: "0x00008000", CHIP_ID_S3: "0x00008000", "legacy": "0x00008000"},
    otaInitAddr={CHIP_ID_S2: "0x0002E000", CHIP_ID_S3: "0x0002E000", "legacy": "0x0000e000"},
    userFwAddr={CHIP_ID_S2: "0x00030000", CHIP_ID_S3: "0x00030000", "legacy": "0x00010000"},
    backgroundAddr={CHIP_ID_S2: "0x190000", CHIP_ID_S3: "0x4B0000"},
    backgroundSize={CHIP_ID_S2: "0x50000", CHIP_ID_S3: "0x50000"},
    wallpaperAddr={CHIP_ID_S2: "0x001E0000", CHIP_ID_S3: "0x00500000"},
    coredump={CHIP_ID_S2: "0x0001E000", CHIP_ID_S3: "0x0001E000"},
    coredumpSize={CHIP_ID_S2: "0x10000", CHIP_ID_S3: "0x10000"},
)
EL_PROFILE = ProductProfile(
    name="el",
    config_filename="SnailHeater_Tool_EL.yaml",
    firmware_version_pattern="SEL\\S*_SW_v\\d{1,2}\\.\\d{1,2}\\.\\d{1,2}",
    supports_rtttl=False,
    auto_activation=False,
    resolutions=("320x240",),
    bootloaderAddr={CHIP_ID_S3: "0x00000000"},
    tableAddr={CHIP_ID_S3: "0x00008000"},
    otaInitAddr={CHIP_ID_S3: "0x0001e000"},
    userFwAddr={CHIP_ID_S3: "0x00020000"},
    backgroundAddr={CHIP_ID_S3: "0x480000"},
    backgroundSize={CHIP_ID_S3: "0x50000"},
    wallpaperAddr={CHIP_ID_S3: "0x4D0000"},
    coredump={CHIP_ID_S3: "0x460000"},
    coredumpSize={CHIP_ID_S3: "0x20000"},
)
