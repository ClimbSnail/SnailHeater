# -*- coding: utf-8 -*-
"""现代 WebUI 的用户外观偏好读写。"""

from __future__ import annotations

import json
import os
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Literal

ThemeKind = Literal["system", "light", "dark"]


@dataclass(frozen=True)
class UiPreferences:
    """保存现代 WebUI 的外观主题。"""

    theme: ThemeKind = "system"

    def to_dict(self) -> dict[str, str]:
        """将偏好对象转换为可序列化的字典。"""
        return asdict(self)


def preferences_path() -> Path:
    """返回用户可写入的现代界面偏好文件路径。"""
    app_data = os.getenv("APPDATA")
    base = Path(app_data) if app_data else Path.home() / ".snailheater"
    return base / "SnailHeaterTool" / "webui-preferences.json"


def load_preferences(path: Path | None = None) -> UiPreferences:
    """读取主题偏好；文件缺失或内容无效时返回安全默认值。"""
    target = path or preferences_path()
    try:
        raw = json.loads(target.read_text(encoding="utf-8"))
        theme = raw.get("theme", "system")
        if theme not in {"system", "light", "dark"}:
            theme = "system"
        return UiPreferences(theme=theme)
    except (OSError, TypeError, ValueError, json.JSONDecodeError):
        return UiPreferences()


def save_preferences(preferences: UiPreferences, path: Path | None = None) -> UiPreferences:
    """原子写入现代界面主题偏好，并返回已保存的对象。"""
    target = path or preferences_path()
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = target.with_suffix(".tmp")
    temporary.write_text(
        json.dumps(preferences.to_dict(), ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(target)
    return preferences
