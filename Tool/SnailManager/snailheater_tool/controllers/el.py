# -*- coding: utf-8 -*-
from SnailManagerToolUi_EL import Ui_ManagerTool
from .base import BaseToolController
from ..profiles import EL_PROFILE


class ELController(BaseToolController):
    """EL 两个 Qt 入口共用的产品控制器。"""

    def __init__(self):
        super().__init__(EL_PROFILE, Ui_ManagerTool)

    def configure_product_ui(self):
        """
        根据产品配置初始化界面。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.form.resolutionComboBox.hide()
        if hasattr(self.form, "resolutionlabel"):
            self.form.resolutionlabel.hide()
        self.form.ActivateGroupBox.hide()
        self.form.PictureModeGroupBox_2.hide()
        self.form.tabWidget.setCurrentIndex(1)
        self.form.tabWidget.tabBar().setEnabled(False)

    def power_warning_message(self):
        return "开始前一定要拔掉除刷机线外的其他电源线，并按住旋钮中键！"
