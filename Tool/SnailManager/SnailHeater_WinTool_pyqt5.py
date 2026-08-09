#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""SnailHeater PyQt5 入口。业务实现位于 snailheater_tool 包。"""

import sys

from choose_ver import QtWidgets
from snailheater_tool.controllers.snailheater import SnailHeaterController
from snailheater_tool.qt_compat import configure_high_dpi, exec_application


def main() -> int:
    """
    执行程序入口逻辑。

    Returns:
        函数处理结果；具体类型由调用场景决定。
    """
    configure_high_dpi()
    app = QtWidgets.QApplication(sys.argv)
    controller = SnailHeaterController()
    controller.run()
    # 保持控制器生命周期，避免窗口和后台任务被垃圾回收。
    app.tool_controller = controller
    return exec_application(app)


if __name__ == "__main__":
    sys.exit(main())
