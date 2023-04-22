# -*- coding: utf-8 -*-

from PyQt5.QtWidgets import QComboBox as QtQComboBox
from PyQt5.QtCore import pyqtSignal

# import mywidgets

class QComboBox(QtQComboBox):
    clicked = pyqtSignal()  # 创建一个信号

    def showPopup(self):  # 重写showPopup函数,"弹出下拉列表"
        self.clicked.emit()  # 弹出前发送信号
        super(QComboBox, self).showPopup()  # 调用父类的showPopup()
