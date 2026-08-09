# -*- coding: utf-8 -*-

# Resource object code
#
# Created by: The Resource Compiler for PyQt5 (Qt v5.15.2)
#
# WARNING! All changes made in this file will be lost!

from enum import Enum

class UI_VERSION(Enum):
    # 层级类型
    PYQT5 = 0
    PYSIDE2 = 1
    PYSIDE6 = 2
    UNKNOW = 127

ui_lib_version = UI_VERSION.PYSIDE2

if ui_lib_version == UI_VERSION.PYQT5:
    from PyQt5 import QtCore, QtGui, QtWidgets
    class ClickableComboBox(QtWidgets.QComboBox):
        clicked = QtCore.pyqtSignal()  # 创建一个信号

        def showPopup(self):  # 重写showPopup函数,"弹出下拉列表"
            self.clicked.emit()  # 弹出前发送信号
            super(ClickableComboBox, self).showPopup()  # 调用父类的showPopup()
elif ui_lib_version == UI_VERSION.PYSIDE2:
    from PySide2 import QtCore, QtGui, QtWidgets
    class ClickableComboBox(QtWidgets.QComboBox):
        clicked = QtCore.Signal()

        def showPopup(self):
            self.clicked.emit()
            super().showPopup()
elif ui_lib_version == UI_VERSION.PYSIDE6:
    from PySide6 import QtCore, QtGui, QtWidgets
    class ClickableComboBox(QtWidgets.QComboBox):
        clicked = QtCore.Signal()

        def showPopup(self):
            self.clicked.emit()
            super().showPopup()
