# -*- coding: utf-8 -*-
from download import Ui_SanilHeaterTool
from .base import BaseToolController
from ..profiles import SNAILHEATER_PROFILE


class SnailHeaterController(BaseToolController):
    """SnailHeater 两个 Qt 入口共用的产品控制器。"""

    def __init__(self):
        super().__init__(SNAILHEATER_PROFILE, Ui_SanilHeaterTool)

    def configure_product_ui(self):
        self.form.resolutionComboBox.show()
        if hasattr(self.form, "resolutionlabel"):
            self.form.resolutionlabel.show()
