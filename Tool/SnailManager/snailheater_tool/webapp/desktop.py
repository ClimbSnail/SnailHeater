# -*- coding: utf-8 -*-
"""pywebview 桌面容器、FastAPI 生命周期与本地文件选择桥接。"""

from __future__ import annotations

import socket
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Optional, Tuple

import uvicorn
import common

from .api import create_app, create_token
from .session import WebToolSession


@dataclass(frozen=True)
class WindowGeometry:
    """根据主显示器逻辑像素计算出的窗口初始尺寸与最小尺寸。"""

    width: int
    height: int
    min_width: int
    min_height: int


def _clamp(value: int, lower: int, upper: int) -> int:
    """将数值限制在闭区间内。"""
    return max(lower, min(value, upper))


def calculate_window_geometry(
    screen_width: Optional[int] = None, screen_height: Optional[int] = None
) -> WindowGeometry:
    """根据主显示器的逻辑分辨率生成适合桌面 WebUI 的窗口尺寸。

    pywebview 的 Screen 使用 Windows DPI 缩放后的逻辑像素。计算时为任务栏和
    标题栏预留空间，并将默认高度控制在约 80%，因此 720p 或 150% DPI 下不会再使用超过屏幕的固定尺寸。
    """
    if not screen_width or not screen_height or screen_width < 1 or screen_height < 1:
        return WindowGeometry(width=1440, height=864, min_width=960, min_height=600)

    width_limit = max(1, int(round(screen_width * 0.90)))
    height_limit = max(1, int(round(screen_height * 0.88)))
    preferred_width = _clamp(int(round(screen_width * 0.75)), 900, 1920)
    preferred_height = _clamp(int(round(screen_height * 0.80)), 600, 1200)
    width = min(width_limit, preferred_width)
    height = min(height_limit, preferred_height)

    min_width = min(width, _clamp(int(round(screen_width * 0.65)), 720, 960))
    min_height = min(height, _clamp(int(round(screen_height * 0.65)), 420, 600))
    return WindowGeometry(width=width, height=height, min_width=min_width, min_height=min_height)


def primary_screen(webview_module: Any) -> Optional[Any]:
    """返回 pywebview 的主显示器对象；不可用时返回 None。"""
    try:
        screens = list(webview_module.screens)
        if not screens:
            return None
        return next((screen for screen in screens if screen.x == 0 and screen.y == 0), screens[0])
    except Exception:
        return None


def primary_screen_size(webview_module: Any) -> Tuple[Optional[int], Optional[int]]:
    """读取 pywebview 主显示器的逻辑宽高；任何失败都会回退到固定窗口方案。"""
    screen = primary_screen(webview_module)
    if screen is None:
        return (None, None)
    return (int(screen.width), int(screen.height))


class DesktopBridge:
    """仅向 React 前端提供原生文件选择能力。"""

    def __init__(self) -> None:
        # pywebview 会递归扫描 js_api 的所有公开属性。窗口对象必须保持私有，
        # 否则扫描会进入 Windows AccessibilityObject 并无限递归，导致界面卡死。
        self._window = None

    def select_files(self, multiple: bool = True, background_only: bool = False) -> list[str]:
        """使用原生文件对话框选择素材；背景入口只允许图片或 RGB565 BIN 文件。"""
        if self._window is None:
            return []
        import webview

        file_types = (
            ("背景文件 (*.jpg;*.jpeg;*.png;*.bin)",)
            if background_only
            else ("素材文件 (*.*)",)
        )
        result = self._window.create_file_dialog(
            webview.OPEN_DIALOG,
            allow_multiple=multiple,
            file_types=file_types,
        )
        return [str(item) for item in result or ()]


def find_available_port() -> int:
    """获取供本机 FastAPI 服务使用的短生命周期空闲 TCP 端口。"""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def web_static_dir() -> Path:
    """返回 React/Vite 构建产物所在目录。"""
    return Path(__file__).resolve().parents[2] / "webui" / "dist"


def run_web_desktop(product: str) -> int:
    """启动本机 API、创建 WebView2 窗口，并在关闭后停止后台服务。"""
    try:
        import webview
    except ImportError as error:
        raise RuntimeError("缺少 pywebview，请先安装现代界面依赖") from error
    static_dir = web_static_dir()
    if not (static_dir / "index.html").is_file():
        raise RuntimeError("未找到 WebUI 静态资源，请先在 webui 目录运行 npm.cmd run build")
    session = WebToolSession(product)
    token = create_token()
    app = create_app(session, token, static_dir)
    port = find_available_port()
    server = uvicorn.Server(
        uvicorn.Config(app, host="127.0.0.1", port=port, log_level="warning", access_log=False)
    )
    server_thread = threading.Thread(target=server.run, daemon=True, name="snailheater-web-api")
    server_thread.start()
    for _ in range(50):
        if server.started:
            break
        time.sleep(0.1)
    if not server.started:
        server.should_exit = True
        raise RuntimeError("本地 WebUI 服务启动失败")
    bridge = DesktopBridge()
    screen = primary_screen(webview)
    screen_width = int(screen.width) if screen is not None else None
    screen_height = int(screen.height) if screen is not None else None
    geometry = calculate_window_geometry(screen_width, screen_height)
    title = f"{session.config.tool_name.strip()} {common.TOOL_VERSION}"
    bridge._window = webview.create_window(
        title,
        f"http://127.0.0.1:{port}/?token={token}",
        min_size=(geometry.min_width, geometry.min_height),
        width=geometry.width,
        height=geometry.height,
        screen=screen,
    )
    # 只注册明确的函数，避免 pywebview 递归扫描原生 Window 对象。
    bridge._window.expose(bridge.select_files)
    try:
        webview.start(debug=False, http_server=False)
        return 0
    finally:
        server.should_exit = True
        server_thread.join(timeout=3)
