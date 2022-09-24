# encoding: utf-8

# pip install pyserial -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyqt5  -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyqt5-tools -i https://mirrors.aliyun.com/pypi/simple/
# pyinstaller --icon ./images/SnailHeater_256.ico -w -F SnailHeater_WinTool.py

# 环境搭建学习文旦 https://github.com/Jacob-xyb/PyQt_Notes/blob/master/PyQt5.md
# QT官网：https://doc.qt.io/qt-5/index.html
# QT教程  https://b23.tv/9R6dbDA
# QT项目学习课件 https://doc.itprojects.cn/0001.zhishi/python.0008.pyqt5rumen/index.html

import sys
import os
import time
import threading

import serial  # pip install pyserial
import serial.tools.list_ports
# from PyQt5.Qt import *
from PyQt5.Qt import QWidget, QApplication
from PyQt5 import uic, QtCore
from PyQt5.QtWidgets import QMessageBox
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtCore import Qt

from esptoolpy import esptool
# from esptoolpy import espefuse
from download import Ui_SanilHeaterTool
import common

COLOR_RED = '<span style=\" color: #ff0000;\">%s</span>'
BAUD_RATE = 921600


class DownloadController(object):

    def __init__(self):
        self.progress_bar_time_cnt = 0
        self.progress_bar_timer = QtCore.QTimer()
        self.progress_bar_timer.timeout.connect(self.schedule_display_time)

        self.download_thread = None

    def run(self):
        """
        下载页面的主界面生成函数
        :return:
        """
        self.app = QApplication(sys.argv)
        self.win_main = QWidget()
        self.form = Ui_SanilHeaterTool()
        self.form.setupUi(self.win_main)

        _translate = QtCore.QCoreApplication.translate
        self.win_main.setWindowTitle(_translate("SanilHeaterTool",
                                                "蜗牛台SnailHeater刷机工具 " + common.VERSION))

        # 设置文本可复制
        self.form.LinkInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.QQInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)

        self.form.ComComboBox.clicked.connect(self.scan_com)  # 信号与槽函数的绑定
        self.form.FirmwareComboBox.clicked.connect(self.scan_firmware)
        self.form.UICPushButton.clicked.connect(self.uic_button_click)
        self.form.ActivatePushButton.clicked.connect(self.act_button_click)
        # self.form.UpdatePushButton.clicked.connect(self.update_button_click)
        self.form.UpdatePushButton.clicked.connect(self.show_message)
        self.form.CanclePushButton.clicked.connect(self.cancle_button_click)

        # 设置提示信息
        self.form.UICPushButton.setToolTip("获取机器码")
        self.form.UpdateModeMethodRadioButton.setToolTip("保留用户的设置信息，只在固件上做更新")
        self.form.ClearModeMethodRadioButton.setToolTip("将会清空芯片内所有可清空的信息，想要完全纯净的刷固件可选此项")

        #
        self.form.UICLineEdit.setReadOnly(True)
        # x = self.form.x()

        # self.win_main.move(self.win_main)
        self.win_main.show()
        sys.exit(self.app.exec_())

    def scan_com(self):
        """
        搜索串口
        """
        self.print_log("搜索串口号...")
        self.form.ComComboBox.clear()
        com_obj_list = list(serial.tools.list_ports.comports())
        com_list = [com_obj[0] for com_obj in com_obj_list]
        if com_list == []:
            com_list = ["未识别到"]
        self.form.ComComboBox.addItems(com_list)

    def scan_firmware(self):
        """
        搜索固件
        """
        self.print_log("搜索同目录下的可用固件...")
        self.form.FirmwareComboBox.clear()
        # 列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        firmware_path_list = []
        for file_name in list_file:
            if 'SnailHeater_v' in file_name:
                firmware_path_list.append(file_name.strip())

        if len(firmware_path_list) == 0:
            firmware_path_list = ["未找到固件"]
        self.form.FirmwareComboBox.addItems(firmware_path_list)

    def act_button_click(self):
        self.print_log("正在激活设备...")
        # todo

    def uic_button_click(self):
        """
        获取用户识别码 显示在用户识别码的信息框里
        :return: None
        """
        self.print_log("获取机器码（用户识别码）...")
        self.form.UICLineEdit.setText("功能未开通")
        # todo

    def update_button_click(self):
        """
        按下 刷机 按键后触发的检查、刷机操作
        :return: None
        """
        self.print_log("准备更新固件...")
        self.form.UpdateModeMethodRadioButton.setEnabled(False)
        self.form.ClearModeMethodRadioButton.setEnabled(False)
        self.form.UpdatePushButton.setEnabled(False)

        select_com = self.form.ComComboBox.currentText().strip()
        firmware_path = self.form.FirmwareComboBox.currentText().strip()
        mode = "更新式" if self.form.UpdateModeMethodRadioButton.isChecked() else "清空式"

        com_list = [com_obj[0] for com_obj in list(serial.tools.list_ports.comports())]
        # if select_com == "未找到固件" or firmware_path == "":
        if select_com not in com_list or firmware_path == "":
            if select_com not in com_list:
                self.print_log((COLOR_RED % "错误提示：") + "无法检测到串口设备，先确认 CH340 驱动是否正常或尝试 typec 调换方向。\n")
            if firmware_path == "":
                self.print_log((COLOR_RED % "错误提示：") + "未查询到固件文件！")
            self.form.UpdatePushButton.setEnabled(True)
            self.form.UpdateModeMethodRadioButton.setEnabled(True)
            self.form.ClearModeMethodRadioButton.setEnabled(True)
            return False

        self.print_log("串口号：" + (COLOR_RED % select_com))
        self.print_log("固件文件：" + (COLOR_RED % firmware_path))
        self.print_log("刷机模式：" + (COLOR_RED % mode))

        all_time = 0  # 粗略认为连接并复位芯片需要0.5s钟
        if mode == "清空式":
            all_time += 23
        else:
            all_time += 4
        file_list = ["./boot_app0.bin",
                     "./bootloader.bin",
                     "./partitions.bin",
                     "./tinyuf2.bin",
                     firmware_path]
        for filepath in file_list:
            all_time = all_time + os.path.getsize(filepath) * 10 / BAUD_RATE

        self.print_log("刷机预计需要：" + str(all_time)[0:5] + "s")

        # 进度条进程要在下载进程之前启动（为了在下载失败时可以立即查杀进度条进程）
        self.download_thread = threading.Thread(target=self.down_action,
                                                args=(mode, select_com, firmware_path))
        self.progress_bar_timer.start(all_time / 0.1)

        self.download_thread.setDaemon(True)  # 设置守护线程目的尽量防止意外中断掉主线程程序
        self.download_thread.start()

    def down_action(self, mode, select_com, firmware_path):
        """
        下载操作主体
        :param mode:下载模式
        :param select_com:串口号
        :param firmware_path:固件文件路径
        :return:None
        """
        try:
            self.progress_bar_time_cnt = 1  # 间接启动进度条更新

            if mode == "清空式":
                self.print_log("正在清空主机数据...")
                # esptool.py erase_region 0x20000 0x4000
                # esptool.py erase_flash
                cmd = ['erase_flash']
                esptool.main(cmd)
                self.print_log("完成清空！")

            #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_40m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000
            cmd = ['SnailHeater_TOOL.py', '--port', select_com,
                   '--baud', str(BAUD_RATE),
                   'write_flash',
                   '0x00001000', "bootloader.bin",
                   '0x00008000', "partitions.bin",
                   '0x0000e000', "boot_app0.bin",
                   '0x002d0000', "tinyuf2.bin",
                   '0x00010000', firmware_path
                   ]

            self.print_log("开始刷写固件...")
            # sys.argv = cmd
            esptool.main(cmd[1:])
            self.esp_reboot()  # 手动复位芯片
            self.print_log("刷机结束！")
            self.print_log("\n\n刷机流程完毕，请保持typec通电等待焊台屏幕将会亮起后才能断电。\n注：更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待20s左右。\n")

        except Exception as err:
            self.print_log(COLOR_RED % "未释放资源，请15s后再试。如无法触发下载，拔插type-c接口再试。")
            print(err)

        self.progress_bar_time_cnt = 0  # 复位进度条

        self.form.UpdatePushButton.setEnabled(True)
        self.form.UpdateModeMethodRadioButton.setEnabled(True)
        self.form.ClearModeMethodRadioButton.setEnabled(True)

    def cancle_button_click(self):
        """
        取消下载固件
        :return: None
        """

        self.print_log("手动停止更新固件...")

        if self.download_thread != None:
            try:
                # 杀线程
                # common.kill_thread(self.download_thread, self.down_action)
                common._async_raise(self.download_thread)
                self.download_thread = None
            except Exception as err:
                print(err)

        self.scan_com()

        # 复位进度条
        self.progress_bar_time_cnt = 0
        self.form.progressBar.setValue(0)

        self.form.UpdatePushButton.setEnabled(True)
        self.form.UpdateModeMethodRadioButton.setEnabled(True)
        self.form.ClearModeMethodRadioButton.setEnabled(True)

    def print_log(self, info):
        self.form.LogInfoTextBrowser.append(info + '\n')
        QApplication.processEvents()
        # self.form.LogInfoTextBrowser.

    def esp_reboot(self):
        """
        重启芯片(控制USB-TLL的rst dst引脚)
        :return:
        """

        select_com = self.form.ComComboBox.currentText().strip()
        port = serial.Serial(select_com, BAUD_RATE, timeout=10)
        port.setRTS(True)  # EN->LOW
        port.setDTR(port.dtr)
        time.sleep(0.05)
        port.setRTS(False)
        port.setDTR(port.dtr)
        port.close()  # 关闭串口

    def schedule_display_time(self):
        if self.progress_bar_time_cnt > 0 and self.progress_bar_time_cnt < 99:
            self.progress_bar_time_cnt += 1
        self.form.progressBar.setValue(self.progress_bar_time_cnt)

    def show_message(self):
        """
        警告拔掉AC220V消息框
        :return: None
        """
        # # 最后的Yes表示弹框的按钮显示为Yes，默认按钮显示为OK,不填QMessageBox.Yes即为默认
        # reply = QMessageBox.warning(self.win_main, "重要提示",
        #                                COLOR_RED % "刷机一定要拔掉220V电源线！",
        #                                QMessageBox.Yes | QMessageBox.Cancel,
        #                                QMessageBox.Cancel)

        # 创建自定义消息框
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示",
                                COLOR_RED % "刷机一定要拔掉220V电源线！")
        # 添加自定义按钮
        do = self.mbox.addButton('确定', QMessageBox.YesRole)
        cancle = self.mbox.addButton('取消', QMessageBox.NoRole)
        # 设置消息框中内容前面的图标
        self.mbox.setIcon(2)
        do.clicked.connect(self.update_button_click)
        self.mbox.show()


def main():
    # QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)  # 和designer设计的窗口比例一致
    app = QApplication([])
    download_ui = uic.loadUi("download.ui")
    # download_ui.ComComboBox.
    download_ui.show()
    app.exec_()


if __name__ == '__main__':
    # main()
    downloader = DownloadController()
    downloader.run()
