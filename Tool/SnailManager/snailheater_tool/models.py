# -*- coding: utf-8 -*-
from dataclasses import dataclass, field
from enum import Enum
from typing import List, Optional, Tuple


class DownloadMode(str, Enum):
    """保持旧界面使用的刷机模式文本。"""

    CLEAR = "清空式"
    UPDATE = "更新式"


@dataclass(frozen=True)
class FlashEntry:
    address: str
    path: str


@dataclass
class FlashPlan:
    port: str
    baud_rate: str
    flash_size: str
    firmware_path: str
    mode: DownloadMode
    entries: List[FlashEntry] = field(default_factory=list)
    erase_region: Optional[Tuple[str, str]] = None
    chip_id: str = ""
    firmware_version: str = ""

    def to_esptool_args(self) -> List[str]:
        """
        将刷写条目转换为 esptool 命令参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        args = [
            "--port",
            self.port,
            "--baud",
            str(self.baud_rate),
            "--after",
            "hard_reset",
            "write_flash",
            "--flash_size",
            self.flash_size,
        ]
        for entry in self.entries:
            args.extend([entry.address, entry.path])
        return args


@dataclass(frozen=True)
class FlashResult:
    success: bool
    message: str = ""
    error_type: str = ""
    cancelled: bool = False


@dataclass
class OperationState:
    current_chip_id: str = ""
    auto_activate: bool = True
    download_mode: DownloadMode = DownloadMode.UPDATE
    default_wallpaper: str = ""
    default_background: str = ""
    support_versions: Optional[List[str]] = None
