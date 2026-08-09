# -*- coding: utf-8 -*-
"""FastAPI 本地接口：向 React 前端提供统一的设备和任务 API。"""

from __future__ import annotations

import asyncio
import functools
import secrets
from pathlib import Path
from typing import Any, Callable, Dict, Optional

from fastapi import Body, FastAPI, Request
from fastapi.responses import FileResponse, JSONResponse, PlainTextResponse, StreamingResponse
from fastapi.staticfiles import StaticFiles

from ..ui_preferences import ThemeKind, UiPreferences, load_preferences, save_preferences
from .session import WebToolSession


def ok(data: Any) -> Dict[str, Any]:
    """构造统一的成功响应包装。"""
    return {"ok": True, "data": data}


def fail(code: str, message: str) -> Dict[str, Any]:
    """构造统一的失败响应包装。"""
    return {"ok": False, "error": {"code": code, "message": message}}


async def run_blocking(func: Callable[..., Any], *args: Any) -> Any:
    """在线程池中执行串口、esptool 与网络等阻塞调用，保持本地 API 可响应。"""
    loop = asyncio.get_running_loop()
    return await loop.run_in_executor(None, functools.partial(func, *args))


def create_token() -> str:
    """生成仅供当前桌面窗口使用的本地 API 访问令牌。"""
    return secrets.token_urlsafe(32)


def create_app(session: WebToolSession, token: str, static_dir: Optional[Path] = None) -> FastAPI:
    """创建绑定单个产品会话、令牌与静态资源目录的 FastAPI 应用。"""
    app = FastAPI(title="SnailHeater Modern UI API", docs_url=None, redoc_url=None)
    static_root = static_dir if static_dir and static_dir.is_dir() else None

    @app.middleware("http")
    async def verify_local_token(request: Request, call_next: Callable[..., Any]):
        """限制 API 仅能由当前 pywebview 窗口通过随机令牌访问。"""
        path = request.url.path
        if path.startswith("/api"):
            supplied = request.headers.get("X-Snail-Token") or request.query_params.get("token")
            if not secrets.compare_digest(supplied or "", token):
                return JSONResponse(fail("unauthorized", "本地接口令牌无效"), status_code=401)
        return await call_next(request)

    @app.exception_handler(ValueError)
    async def handle_value_error(_: Request, error: ValueError) -> JSONResponse:
        """将可预期的输入与硬件校验问题转换为客户端可显示的错误。"""
        return JSONResponse(fail("invalid_request", str(error)), status_code=400)

    @app.exception_handler(Exception)
    async def handle_unexpected_error(_: Request, error: Exception) -> JSONResponse:
        """避免未处理异常以 HTML 页面形式泄漏给桌面前端。"""
        return JSONResponse(fail(type(error).__name__, str(error) or "后台服务发生未预期错误"), status_code=500)

    if static_root:
        assets = static_root / "assets"
        if assets.is_dir():
            app.mount("/assets", StaticFiles(directory=str(assets)), name="assets")

    @app.get("/api/bootstrap")
    async def bootstrap() -> Dict[str, Any]:
        """返回产品能力、工具元数据和当前初始状态。"""
        return ok(session.bootstrap())

    @app.get("/api/ports")
    async def list_ports() -> Dict[str, Any]:
        """返回可选择的串口列表。"""
        return ok(await run_blocking(session.list_ports))

    @app.get("/api/firmware")
    async def list_firmware() -> Dict[str, Any]:
        """返回符合当前产品规则的本地固件列表。"""
        return ok(await run_blocking(session.scan_firmware))

    @app.get("/api/versions")
    async def versions() -> Dict[str, Any]:
        """查询工具与固件的在线版本提示。"""
        return ok(await run_blocking(session.latest_versions))

    @app.post("/api/device/probe")
    async def probe_device(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """使用 esptool 识别指定设备的芯片和闪存容量。"""
        return ok(await run_blocking(session.probe_hardware, str(payload.get("port", ""))))

    @app.post("/api/device/query")
    async def query_device(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """读取当前串口设备的机器码并查询对应激活信息。"""
        return ok(await run_blocking(session.query_device, str(payload.get("port", ""))))

    @app.post("/api/device/flash-info")
    async def flash_info(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """识别设备芯片型号与闪存容量。"""
        return ok(await run_blocking(session.flash_info, str(payload.get("port", ""))))
    @app.post("/api/device/activate")
    async def activate_device(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """写入激活码并返回设备确认结果。"""
        return ok(await run_blocking(session.activate, str(payload.get("port", "")), str(payload.get("sn", ""))))

    @app.get("/api/device/colors")
    async def read_colors(port: str) -> Dict[str, Any]:
        """读取指定设备的 UI 文本颜色设置。"""
        return ok(await run_blocking(session.read_colors, port))

    @app.put("/api/device/colors")
    async def write_colors(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """保存设备的普通与按下状态颜色。"""
        return ok(
            await run_blocking(
                session.write_colors,
                str(payload.get("port", "")),
                str(payload.get("normal", "")),
                str(payload.get("pressed", "")),
            )
        )

    @app.post("/api/device/hard-reset")
    async def hard_reset(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """执行设备硬复位。"""
        return ok(await run_blocking(session.hard_reset, str(payload.get("port", ""))))

    @app.post("/api/operations/{kind}")
    async def start_operation(kind: str, payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """创建一项耗时操作并立即返回其操作标识。"""
        operation = session.start_operation(kind, payload)
        return ok(operation.snapshot())

    @app.get("/api/operations/{operation_id}")
    async def operation_status(operation_id: str) -> JSONResponse:
        """查询一项后台操作的当前状态。"""
        operation = session.operations.get(operation_id)
        if operation is None:
            return JSONResponse(fail("not_found", "未找到指定操作"), status_code=404)
        return JSONResponse(ok(operation.snapshot()))

    @app.post("/api/operations/{operation_id}/cancel")
    async def cancel_operation(operation_id: str) -> JSONResponse:
        """立即终止一项仍在运行的后台操作。"""
        operation = session.operations.cancel(operation_id)
        if operation is None:
            return JSONResponse(fail("not_found", "未找到指定操作"), status_code=404)
        return JSONResponse(ok(operation.snapshot()))

    @app.get("/api/operations/{operation_id}/events")
    async def operation_events(operation_id: str) -> StreamingResponse:
        """以 SSE 长连接持续推送日志、进度和最终结果。"""
        return StreamingResponse(
            session.operations.event_stream(operation_id),
            media_type="text/event-stream",
            headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"},
        )

    @app.get("/api/preferences")
    async def preferences() -> Dict[str, Any]:
        """读取当前用户持久化的现代界面主题偏好。"""
        return ok(load_preferences().to_dict())

    @app.put("/api/preferences")
    async def update_preferences(payload: Dict[str, Any] = Body(default={})) -> Dict[str, Any]:
        """校验并保存用户请求的现代界面主题。"""
        current = load_preferences()
        theme: ThemeKind = payload.get("theme", current.theme)
        if theme not in {"system", "light", "dark"}:
            raise ValueError("theme 只允许为 system、light 或 dark")
        saved = save_preferences(UiPreferences(theme=theme))
        return ok(saved.to_dict())

    @app.get("/", response_model=None)
    async def index():
        """返回 React 构建入口；资源缺失时显示可执行的故障提示。"""
        if static_root and (static_root / "index.html").is_file():
            return FileResponse(static_root / "index.html")
        return PlainTextResponse("未找到 WebUI 静态资源，请先在 webui 目录执行 npm.cmd run build。", status_code=503)

    return app





