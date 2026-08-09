# -*- coding: utf-8 -*-
"""与 Qt 解耦的后台操作管理器及 SSE 事件来源。"""

from __future__ import annotations

import json
import queue
import threading
import time
import uuid
from dataclasses import dataclass, field
from typing import Any, Callable, Generator, Optional

import common


@dataclass
class Operation:
    """记录单个长耗时设备操作的状态、日志和取消信号。"""

    operation_id: str
    kind: str
    cancel_event: threading.Event = field(default_factory=threading.Event)
    events: "queue.Queue[dict[str, Any]]" = field(default_factory=queue.Queue)
    status: str = "running"
    result: Any = None
    error: Optional[str] = None
    created_at: float = field(default_factory=time.time)
    finished_at: Optional[float] = None
    thread: Optional[threading.Thread] = field(default=None, repr=False, compare=False)

    def emit(self, event_type: str, **payload: Any) -> None:
        """向订阅者发布操作日志、进度或最终状态事件。"""
        self.events.put({"type": event_type, "timestamp": time.time(), **payload})

    def snapshot(self) -> dict[str, Any]:
        """返回可供 REST 查询的操作状态快照。"""
        return {
            "operationId": self.operation_id,
            "kind": self.kind,
            "status": self.status,
            "result": self.result,
            "error": self.error,
            "createdAt": self.created_at,
            "finishedAt": self.finished_at,
            "cancellable": self.status == "running",
        }


class OperationManager:
    """使用标准线程执行硬件任务，避免 WebUI 依赖 Qt 的 QThread。"""

    def __init__(self) -> None:
        self._operations: dict[str, Operation] = {}
        self._lock = threading.RLock()

    def start(self, kind: str, target: Callable[[Operation], Any]) -> Operation:
        """创建操作记录、启动后台线程并立即返回操作标识。"""
        operation = Operation(operation_id=uuid.uuid4().hex, kind=kind)
        with self._lock:
            self._operations[operation.operation_id] = operation
        thread = threading.Thread(
            target=self._run_operation,
            args=(operation, target),
            daemon=True,
            name=f"snailheater-{kind}-{operation.operation_id[:8]}",
        )
        operation.thread = thread
        operation.emit("started", operationId=operation.operation_id, kind=kind)
        thread.start()
        return operation

    def get(self, operation_id: str) -> Optional[Operation]:
        """按标识查询仍被管理器保留的操作对象。"""
        with self._lock:
            return self._operations.get(operation_id)

    def cancel(self, operation_id: str) -> Optional[Operation]:
        """按 QT_Tool 的取消语义立即终止后台操作线程。"""
        operation = self.get(operation_id)
        if operation is None or operation.status != "running":
            return operation
        operation.cancel_event.set()
        operation.status = "cancelled"
        operation.finished_at = time.time()
        operation.result = {"message": "任务已被手动终止"}
        operation.emit("log", level="warning", message="手动停止操作，正在立即终止后台线程。")
        operation.emit("cancelled", result=operation.result)
        thread = operation.thread
        if thread is not None and thread.is_alive() and thread.ident is not None:
            try:
                common.kill_thread(thread, SystemExit)
            except Exception:
                # 与旧 Qt 的 terminate 一样以尽快停止为目标；线程已结束时无需再报错。
                pass
        return operation

    def event_stream(self, operation_id: str) -> Generator[str, None, None]:
        """将队列事件转换为符合 EventSource 格式的 SSE 文本流。"""
        operation = self.get(operation_id)
        if operation is None:
            yield self._sse({"type": "failed", "message": "未找到指定操作"})
            return
        while True:
            try:
                event = operation.events.get(timeout=10)
                yield self._sse(event)
                if event.get("type") in {"completed", "failed", "cancelled"}:
                    return
            except queue.Empty:
                yield ": keep-alive\n\n"

    @staticmethod
    def _sse(event: dict[str, Any]) -> str:
        """序列化单个服务器推送事件。"""
        return "data: " + json.dumps(event, ensure_ascii=False) + "\n\n"

    @staticmethod
    def _run_operation(operation: Operation, target: Callable[[Operation], Any]) -> None:
        """在线程内执行任务；已被立即取消的任务不得再覆盖取消状态。"""
        try:
            result = target(operation)
            if operation.status == "cancelled":
                return
            operation.result = result
            operation.finished_at = time.time()
            if operation.cancel_event.is_set():
                operation.status = "cancelled"
                operation.emit("cancelled", result=result)
            else:
                operation.status = "completed"
                operation.emit("completed", result=result)
        except BaseException as error:
            # WebUI 取消使用 SystemExit 注入，与旧 Qt 的 QThread.terminate 对齐。
            if operation.status == "cancelled" or operation.cancel_event.is_set():
                return
            operation.finished_at = time.time()
            operation.error = str(error)
            operation.status = "failed"
            operation.emit("failed", error={"code": type(error).__name__, "message": str(error)})


