# -*- coding: utf-8 -*-
from choose_ver import QtCore, QtWidgets


def configure_high_dpi():
    """在 QApplication 创建前配置 Qt5 高 DPI，兼容两套绑定。"""
    qt = QtCore.Qt
    policy = getattr(qt, "HighDpiScaleFactorRoundingPolicy", None)
    if policy is not None and hasattr(
        QtWidgets.QApplication, "setHighDpiScaleFactorRoundingPolicy"
    ):
        QtWidgets.QApplication.setHighDpiScaleFactorRoundingPolicy(policy.PassThrough)
    app_attr = getattr(qt, "ApplicationAttribute", qt)
    scaling = getattr(app_attr, "AA_EnableHighDpiScaling", None)
    if scaling is not None:
        QtCore.QCoreApplication.setAttribute(scaling)
        QtWidgets.QApplication.setAttribute(scaling)


def exec_application(app) -> int:
    return (getattr(app, "exec", None) or getattr(app, "exec_"))()
