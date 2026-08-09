# -*- coding: utf-8 -*-
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional
import re
import yaml
from .paths import RuntimePaths


@dataclass(frozen=True)
class InfoLink:
    """工具信息链接的显示名称与目标内容。"""

    name: str = ""
    url: str = ""

    def to_dict(self) -> Dict[str, str]:
        """转换为可通过 FastAPI 返回给 WebUI 的普通字典。"""
        return {"name": self.name, "url": self.url}


def normalize_info_link(value: Any, default_name: str = "") -> InfoLink:
    """兼容解析旧版字符串和新版 name/url 配置。"""
    if isinstance(value, InfoLink):
        return value
    if isinstance(value, dict):
        name = str(value.get("name", "") or "").strip()
        url = str(value.get("url", "") or "").strip()
        return InfoLink(name or url or default_name, url)

    text = str(value or "").strip()
    match = re.search(r"https?://[^\s<>'\"]+", text)
    if not match:
        return InfoLink(default_name, text)
    url = match.group(0).rstrip(".,，。；;")
    name = text[: match.start()].strip(" ：:") or default_name or url
    return InfoLink(name, url)


@dataclass(frozen=True)
class ToolConfig:
    tool_name: str = ""
    info_url_0: InfoLink = field(default_factory=lambda: InfoLink("使用教程", ""))
    info_url_1: InfoLink = field(default_factory=lambda: InfoLink("更新日志", ""))
    qq_info: List[str] = None
    temp_sn_recode_path: Optional[str] = None
    search_sn_registrant_url: Optional[str] = None
    activate_sn_url: Optional[str] = None
    get_firmware_new_ver_url: Optional[str] = None
    get_tool_new_ver_url: Optional[str] = None
    baud_rate: str = "921600"
    info_baud_rate: str = "921600"
    main_appdir_rules: str = "./"
    main_app_rules: str = ".*\\.bin"

    def __post_init__(self):
        object.__setattr__(self, "info_url_0", normalize_info_link(self.info_url_0, "使用教程"))
        object.__setattr__(self, "info_url_1", normalize_info_link(self.info_url_1, "更新日志"))
        if self.qq_info is None:
            object.__setattr__(self, "qq_info", ["", ""])

    @property
    def qq_pair(self) -> List[str]:
        values = list(self.qq_info or [])
        return (values + ["", ""])[:2]


def load_tool_config(filename: str, paths: Optional[RuntimePaths] = None) -> ToolConfig:
    """加载并解析工具配置文件。"""
    paths = paths or RuntimePaths.discover()
    external_config_path = paths.external_root / filename
    config_path = external_config_path if external_config_path.is_file() else paths.root / filename
    with config_path.open("r", encoding="utf-8") as stream:
        root = yaml.safe_load(stream) or {}
    values = root.get("windows_tool", {}) or {}
    qq_info = str(values.get("qq_info", "")).split(",")
    return ToolConfig(
        tool_name=str(values.get("tool_name", "")),
        info_url_0=normalize_info_link(values.get("info_url_0"), "使用教程"),
        info_url_1=normalize_info_link(values.get("info_url_1"), "更新日志"),
        qq_info=qq_info,
        temp_sn_recode_path=values.get("temp_sn_recode_path"),
        search_sn_registrant_url=values.get("search_sn_registrant_url"),
        activate_sn_url=values.get("activate_sn_url"),
        get_firmware_new_ver_url=values.get("get_firmware_new_ver_url"),
        get_tool_new_ver_url=values.get("get_tool_new_ver_url"),
        baud_rate=str(values.get("baud_rate", "921600")),
        info_baud_rate=str(values.get("info_baud_rate", "921600")),
        main_appdir_rules=str(values.get("main_appdir_rules", "./")),
        main_app_rules=str(values.get("main_app_rules", ".*\\.bin")),
    )
