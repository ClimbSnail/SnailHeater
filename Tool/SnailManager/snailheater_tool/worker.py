# -*- coding: utf-8 -*-
from threading import Event
from typing import Optional
from choose_ver import QtCore

try:
    Signal = QtCore.Signal
except AttributeError:
    Signal = QtCore.pyqtSignal
from .firmware_service import FirmwareService
from .models import DownloadMode, FlashPlan, FlashResult, OperationState


class FirmwareWorker(QtCore.QThread):
    """四个项目共用的固件后台任务，设备探测也在线程中完成。"""

    print_signal = Signal(str)
    ret_finish = Signal(bool)
    result_signal = Signal(object)

    def __init__(
        self,
        service: FirmwareService,
        plan: Optional[FlashPlan] = None,
        port: str = "",
        firmware_path: str = "",
        mode: DownloadMode = DownloadMode.UPDATE,
        state: Optional[OperationState] = None,
        parent=None,
    ):
        """
        初始化对象状态和运行所需的依赖。

        Args:
            service: 对应的输入参数。
            plan: 待执行的刷写计划。
            port: 串口名称或串口对象。
            firmware_path: 固件路径。
            mode: 运行模式。
            state: 当前运行状态。
            parent: 父对象。

        """
        super().__init__(parent)
        self._service = service
        self._plan = plan
        self._port = port
        self._firmware_path = firmware_path
        self._mode = mode
        self._state = state
        self._cancel_event = Event()

    def request_cancel(self):
        self._cancel_event.set()

    @property
    def cancel_requested(self):
        return self._cancel_event.is_set()

    def run(self):
        """
        执行当前任务并返回执行结果。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            if self._cancel_event.is_set():
                result = FlashResult(False, "刷机已取消", cancelled=True)
            else:
                plan = self._plan or self._service.prepare_plan(
                    self._port,
                    self._firmware_path,
                    self._mode,
                    self._state,
                    self.print_signal.emit,
                )
                self.print_signal.emit(
                    "已识别芯片 %s，Flash 容量 %s" % (plan.chip_id or "未知", plan.flash_size)
                )
                result = self._service.execute_plan(
                    plan, self._cancel_event, self.print_signal.emit
                )
        except Exception as error:
            result = FlashResult(False, str(error), type(error).__name__)
        if result.message:
            self.print_signal.emit(result.message)
        self.result_signal.emit(result)
        self.ret_finish.emit(result.success)
