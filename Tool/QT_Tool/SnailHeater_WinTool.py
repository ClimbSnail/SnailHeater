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
import re
import json
import requests
import traceback
import struct
import shutil
from PIL import Image

import serial  # pip install pyserial
import serial.tools.list_ports
# from PyQt5.Qt import *
from PyQt5.Qt import QWidget, QApplication
from PyQt5 import uic, QtCore
from PyQt5.QtWidgets import QMessageBox, QApplication, QMainWindow, QFileDialog
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtCore import Qt

import massagehead as mh
from esptoolpy import esptool
# from esptoolpy import espefuse
from download import Ui_SanilHeaterTool
import common

SH_SN = None
if SH_SN == None and os.path.exists("SnailHeater_SN.py"):
    import SnailHeater_SN as SH_SN

    print("激活模块已添加")

COLOR_RED = '<span style=\" color: #ff0000;\">%s</span>'
BAUD_RATE = 921600
INFO_BAUD_RATE = 115200

cur_dir = os.getcwd()  # 当前目录
# 默认壁纸
default_wallpaper_280 = os.path.join(cur_dir, "./base_data/Wallpaper_280x240.lsw")
default_wallpaper_320 = os.path.join(cur_dir, "./base_data/Wallpaper_320x240.lsw")
default_wallpaper_clean = os.path.join(cur_dir, "./base_data/WallpaperClean.lsw")
default_wallpaper = default_wallpaper_280
# 文件缓存目录
wallpaper_cache_dir = os.path.join(cur_dir, "Wallpaper", "Cache")
# 文件生存目录
wallpaper_path = os.path.join(cur_dir, "Wallpaper")
# 壁纸文件
wallpaper_name = os.path.join(wallpaper_path, "Wallpaper.lsw")
wallpaperAddrInFlash = '0x00200000'
TYPE_JPG = 0
TYPE_MJPEG = 1
IMAGE_FORMAT = ["jpg", "JPG", "jpeg", "JPEG", "png", "PNG"]
MOVIE_FORMAT = ["mp4", "MP4", "avi", "AVI", "mov", "MOV"]

# 读取配置信息
cfg_fp = open("SnailHeater_Tool.cfg", "r", encoding="utf-8")
cfg = json.load(cfg_fp)["windows_tool"]
temp_sn_recode_path = cfg["temp_sn_recode_path"] \
    if "temp_sn_recode_path" in cfg.keys() else None
search_sn_url = cfg["search_sn_url"] \
    if "search_sn_url" in cfg.keys() else None
activate_sn_url = cfg["activate_sn_url"] \
    if "activate_sn_url" in cfg.keys() else None
get_firmware_new_ver_url = cfg["get_firmware_new_ver_url"] \
    if "get_firmware_new_ver_url" in cfg.keys() else None

cfg_fp.close()


class DownloadController(object):

    def __init__(self):
        self.progress_bar_time_cnt = 0
        self.ser = None  # 串口
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
                                                "蜗牛台SnailHeater管理工具 " + common.VERSION))

        # 设置文本可复制
        self.form.LinkInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.UpdateLogLinkInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.QQInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.QQInfolabel_2.setTextInteractionFlags(Qt.TextSelectableByMouse)

        self.form.ComComboBox.clicked.connect(self.scan_com)  # 信号与槽函数的绑定
        self.form.FirmwareComboBox.clicked.connect(self.scan_firmware)
        self.form.QueryPushButton.clicked.connect(self.query_button_click)
        self.form.chooseWPButton.clicked.connect(self.chooseFile)
        self.form.ActivatePushButton.clicked.connect(self.act_button_click)
        # self.form.UpdatePushButton.clicked.connect(self.update_button_click)
        self.form.UpdatePushButton.clicked.connect(self.UpdatePushButton_show_message)
        # self.form.WriteWallpaperButton.clicked.connect(self.writeWallpaper)
        self.form.WriteWallpaperButton.clicked.connect(self.WriteWallpaperButton_show_message)
        # self.form.reflushWallpaperButton.clicked.connect(self.cleanWallpaper)
        self.form.reflushWallpaperButton.clicked.connect(self.reflushWallpaperButton_show_message)
        self.form.CanclePushButton.clicked.connect(self.cancle_button_click)

        # 设置提示信息
        self.form.QueryPushButton.setToolTip("获取机器码(SN)")
        self.form.ActivatePushButton.setToolTip("填入SN，点此激活")
        self.form.UpdateModeMethodRadioButton.setToolTip("保留用户的设置信息，只在固件上做更新")
        self.form.ClearModeMethodRadioButton.setToolTip("将会清空芯片内所有可清空的信息，想要完全纯净的刷固件可选此项")
        # self.form.autoScaleBox.setToolTip("自适应长宽。若未勾选则以指定长宽比截取中心区域")
        self.form.chooseWPButton.setToolTip("选择素材文件的路径（可选多项）")
        self.form.WriteWallpaperButton.setToolTip("将选择好的素材转换并刷写到焊台上")
        self.form.PictureModeRadioButton_0.setToolTip("保持指定的分辨率比例，在中心区域最大面积裁剪")
        self.form.PictureModeRadioButton_1.setToolTip("全图范围内适配最佳比例缩放，使每个区域都会保存")

        self.form.reflushWallpaperButton.setToolTip("清除焊台上的壁纸信息")
        self.form.timeLabel_0.setToolTip("只允许填写正整数。全量截取请设置\"0 0\"")
        self.form.qualitylabel.setToolTip("数字越小，质量最高")
        self.form.qualityComboBox.setToolTip("数字越小，质量最高")
        self.form.fpslabel.setToolTip("性能一定，帧率越大越卡顿")
        self.form.fpsEdit.setToolTip("性能一定，帧率越大越卡顿")
        self.form.startTimeEdit.setToolTip("需要截取时间范围才需要设置")
        self.form.endTimeEdit.setToolTip("需要截取时间范围才需要设置")

        self.form.resolutionComboBox.addItems(["280x240 (一、二车)", "320x240 (三车)"])
        self.form.qualityComboBox.addItems([str(num) for num in range(1, 20)])
        self.form.qualityComboBox.setCurrentText("5");
        self.form.fpsEdit.setText("12")
        self.form.startTimeEdit.setText("0")
        self.form.endTimeEdit.setText("0")
        self.form.VerInfolabel.setStyleSheet('color: red')
        # self.form.autoScaleBox.setChecked(True)

        #
        self.form.UICLineEdit.setReadOnly(True)

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
        com_list = []
        for com_obj in com_obj_list:
            com_num = com_obj[0]
            info = com_obj[1].split("(")
            com_info = com_obj[1].split("(")[0].strip()
            com_list.append(com_num + " -> " + com_info)

        # print(com_obj_list[0][1])
        if com_list == []:
            com_list = ["未识别到"]
        self.form.ComComboBox.addItems(com_list)

    def scan_firmware(self):
        """
        搜索固件
        """
        ver = self.get_firmware_version()
        self.print_log("搜索同目录下的可用固件...")
        self.form.FirmwareComboBox.clear()
        # 列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        firmware_path_list = []
        for file_name in list_file:
            if 'SnailHeater_v' in file_name or 'SH_SW_v' in file_name:
                firmware_path_list.append(file_name.strip())

        if len(firmware_path_list) == 0:
            firmware_path_list = ["未找到固件"]
        self.form.FirmwareComboBox.addItems(firmware_path_list)

    def getSafeCom(self):
        """
        获取安全的串口
        :return: Com / None
        """
        if self.ser != None:  # 串口打开标志
            return None

        select_com = self.form.ComComboBox.currentText().split(" -> ")[0].strip()

        com_list = [com_obj[0] for com_obj in list(serial.tools.list_ports.comports())]
        if select_com not in com_list:
            self.print_log((COLOR_RED % "错误提示：") +
                           "无法检测到指定串口设备，先确认 CH340 驱动是否正常或尝试 typec 调换方向。\n")
            return None
        return select_com

    def act_button_click(self):
        self.print_log("正在激活设备...")

        select_com = self.getSafeCom()
        if select_com == None:
            self.print_log(COLOR_RED % "激活操作异常，激活中止...")
            return None

        self.ser = serial.Serial(select_com, INFO_BAUD_RATE, timeout=10)

        act_ret = False

        # 判断是否打开成功
        if self.ser.is_open:
            # self.machine_code_thread = threading.Thread(target=self.read_data,
            #                                         args=(self.ser,))
            # self.machine_code_thread.start()

            # 循环接收数据，此为死循环，可用线程实现
            send_data = mh.SettingMsg()
            send_data.action_type = mh.AT.AT_SETTING_SET
            # send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
            key = ""
            send_data.key = bytes(key, encoding='utf8')
            send_data.type = mh.VT.VALUE_TYPE_SN.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            value = self.form.SNLineEdit.text().strip()
            print(value)
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    STRGLO = self.ser.read(self.ser.in_waiting)
                    print("\nSTRGLO = ", STRGLO)
                    match_info = re.findall(r"Activate Success", STRGLO.decode("utf8"))
                    if match_info != []:
                        act_ret = True
                except Exception as err:
                    print(str(traceback.format_exc()))

            if act_ret == True:
                # sn_record = open(temp_sn_recode_path, 'a', encoding="utf-8")
                # sn_record.write(value+"\n")
                # sn_record.close()
                self.print_log("激活成功")
            else:
                self.print_log("激活失败")
        self.ser.close()  # 关闭串口
        del self.ser
        self.ser = None

    def query_button_click(self):
        """
        获取用户识别码 显示在用户识别码的信息框里
        获取激活码 显示在激活码的信息框里
        :return: None
        """
        self.print_log("获取机器码（用户识别码）...")
        machine_code = self.get_machine_code()
        self.form.UICLineEdit.setText(machine_code)

        self.print_log("\n获取本地激活码（SN）...")
        sn = self.get_sn()
        # 尝试联网查询
        if sn == "":
            try:
                if activate_sn_url != None and activate_sn_url != "":
                    self.print_log("联网查询激活码（管理员模式）...")
                    response = requests.get(activate_sn_url + machine_code, timeout=3)  # , verify=False
                else:
                    self.print_log("联网查询激活码...")
                    response = requests.get(search_sn_url + machine_code, timeout=3)  # , verify=False
                # sn = re.findall(r'\d+', response.text)
                sn = response.text.strip()
                self.print_log("sn " + str(sn))
            except Exception as err:
                print(str(traceback.format_exc()))
                self.print_log("联网异常")

        self.form.SNLineEdit.setText(sn)

        try:
            if sn != "":
                sn_record = open(temp_sn_recode_path, 'a', encoding="utf-8")
                sn_record.write(machine_code + "\t" + sn + "\n")
                sn_record.close()
        except Exception as err:
            print(str(traceback.format_exc()))
            self.print_log("获取异常异常")

    def update_button_click(self):
        """
        按下 刷机 按键后触发的检查、刷机操作
        :return: None
        """
        global default_wallpaper
        self.print_log("准备更新固件...")
        self.form.UpdateModeMethodRadioButton.setEnabled(False)
        self.form.ClearModeMethodRadioButton.setEnabled(False)
        self.form.UpdatePushButton.setEnabled(False)

        firmware_path = self.form.FirmwareComboBox.currentText().strip()
        mode = "更新式" if self.form.UpdateModeMethodRadioButton.isChecked() else "清空式"

        select_com = self.getSafeCom()
        if select_com == None or firmware_path == "":
            if firmware_path == "":
                self.print_log((COLOR_RED % "错误提示：") + "未查询到固件文件！")
            self.form.UpdatePushButton.setEnabled(True)
            self.form.UpdateModeMethodRadioButton.setEnabled(True)
            self.form.ClearModeMethodRadioButton.setEnabled(True)
            return False

        self.print_log("串口号：" + (COLOR_RED % select_com))
        self.print_log("固件文件：" + (COLOR_RED % firmware_path))
        self.print_log("刷机模式：" + (COLOR_RED % mode))

        if "③" in firmware_path:
            default_wallpaper = default_wallpaper_320
        elif "①" in firmware_path or "②" in firmware_path:
            default_wallpaper = default_wallpaper_280
        else:
            default_wallpaper = default_wallpaper_clean

        if not os.path.exists(default_wallpaper):
            default_wallpaper = default_wallpaper_clean

        all_time = 0  # 粗略认为连接并复位芯片需要0.5s钟
        if mode == "清空式":
            all_time += 24
        else:
            all_time += 5
        file_list = ["./base_data/boot_app0.bin",
                     "./base_data/bootloader.bin",
                     "./base_data/partitions.bin",
                     #  "./base_data/tinyuf2.bin",
                     firmware_path,
                     default_wallpaper]
        for filepath in file_list:
            all_time = all_time + os.path.getsize(filepath) * 10 / BAUD_RATE

        if os.path.exists(default_wallpaper):
            all_time = all_time + os.path.getsize(default_wallpaper) * 10 / BAUD_RATE + 2

        self.print_log("刷机预计需要：" + (COLOR_RED % (str(all_time)[0:5] + "s")))

        # 进度条进程要在下载进程之前启动（为了在下载失败时可以立即查杀进度条进程）
        self.download_thread = threading.Thread(target=self.down_action,
                                                args=(mode, select_com, firmware_path))
        self.progress_bar_timer.start(int(all_time / 0.1))

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
        global default_wallpaper
        try:
            if self.ser != None:
                return

            self.ser = 1
            self.progress_bar_time_cnt = 1  # 间接启动进度条更新

            if mode == "清空式":
                self.print_log("正在清空主机数据...")
                # esptool.py erase_region 0x20000 0x4000
                # esptool.py erase_flash
                cmd = ['--port', select_com, 'erase_flash']
                try:
                    esptool.main(cmd)
                    self.print_log("完成清空！")
                except Exception as e:
                    self.print_log(COLOR_RED % "错误：通讯异常。")
                    pass

            #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_40m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000
            cmd = ['SnailHeater_TOOL.py', '--port', select_com,
                   '--baud', str(BAUD_RATE),
                   'write_flash',
                   '0x00001000', "./base_data/bootloader.bin",
                   '0x00008000', "./base_data/partitions.bin",
                   '0x0000e000', "./base_data/boot_app0.bin",
                   # '0x002d0000', "./base_data/tinyuf2.bin",
                   '0x00010000', firmware_path,
                   wallpaperAddrInFlash, default_wallpaper
                   ]

            # self.print_log("cmd = "+ str(cmd))

            self.print_log("开始刷写固件...")
            try:
                esptool.main(cmd[1:])
            except Exception as e:
                self.print_log(COLOR_RED % "错误：通讯异常。")
                return False
            self.ser = None

            # self.esp_reboot()  # 手动复位芯片
            self.print_log(COLOR_RED % "刷机结束！")
            self.print_log("刷机流程完毕，请保持typec通电等待焊台屏幕将会亮起后才能断电。")
            self.print_log((COLOR_RED % "注：") + "更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待10s左右。")
            self.print_log("如25s后始终未能自动亮屏，请手动拔插一次typec接口再次等待10s。\n")

        except Exception as err:
            self.ser = None
            self.print_log(COLOR_RED % "未释放资源，请15s后再试。如无法触发下载，拔插type-c接口再试。")
            print(err)

        # global SH_SN
        # if SH_SN != None:
        #     # 自动激活
        #     time.sleep(22)
        #     self.print_log("获取机器码（用户识别码）...")
        #     machine_code = self.get_machine_code()
        #     self.form.UICLineEdit.setText(machine_code)

        #     ecdata = SH_SN.getSnForMachineCode(machine_code)
        #     self.print_log("\n生成的序列号为: " + ecdata)
        #     self.form.SNLineEdit.setText(ecdata)
        #     self.act_button_click()

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

    def get_firmware_version(self):
        """
        获取最新版
        """
        global get_firmware_new_ver_url
        new_ver = None
        try:
            self.print_log("联网查询最新固件版本...")
            response = requests.get(get_firmware_new_ver_url, timeout=3)  # , verify=False
            # sn = re.findall(r'\d+', response.text)
            if 'SnailHeater_v' in response.text.strip() or 'SH_SW_v' in response.text.strip():
                new_ver = response.text.strip()
                self.form.VerInfolabel.setText("最新固件版本 " + str(new_ver))
                self.print_log("最新固件版本 " + (COLOR_RED % str(new_ver)))
            else:
                self.print_log((COLOR_RED % "最新固件版本查询异常"))

        except Exception as err:
            print(str(traceback.format_exc()))
            self.print_log((COLOR_RED % "联网异常"))
        
        return new_ver

    def get_machine_code(self):
        '''
        查询机器码
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        self.ser = serial.Serial(select_com, INFO_BAUD_RATE, timeout=10)
        machine_code = "查询失败"

        # 判断是否打开成功
        if self.ser.is_open:
            # self.machine_code_thread = threading.Thread(target=self.read_data,
            #                                         args=(self.ser,))
            # self.machine_code_thread.start()

            # 循环接收数据，此为死循环，可用线程实现
            send_data = mh.SettingMsg()
            send_data.action_type = mh.AT.AT_SETTING_GET
            # send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
            key = ""
            send_data.key = bytes(key, encoding='utf8')
            send_data.type = mh.VT.VALUE_TYPE_MC.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            value = ""
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    STRGLO = self.ser.read(self.ser.in_waiting).decode("utf8")
                    print(STRGLO)
                    machine_code = re.findall(r"AT_SETTING_GET VALUE_TYPE_MC = \d*", STRGLO)[0] \
                        .split(" ")[-1]
                except Exception as err:
                    machine_code = "查询失败"
                print(machine_code)

            if machine_code == "查询失败":
                self.print_log((COLOR_RED % "机器码查询失败"))
            else:
                self.print_log("机器码查询成功")

        self.ser.close()  # 关闭串口
        del self.ser
        self.ser = None
        return machine_code

    def get_sn(self):
        '''
        查询SN
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        self.ser = serial.Serial(select_com, INFO_BAUD_RATE, timeout=10)
        sn = ""

        # 判断是否打开成功
        if self.ser.is_open:
            # self.sn_thread = threading.Thread(target=self.read_data,
            #                                         args=(self.ser,))
            # self.sn_thread.start()

            # 循环接收数据，此为死循环，可用线程实现
            send_data = mh.SettingMsg()
            send_data.action_type = mh.AT.AT_SETTING_GET
            # send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
            key = ""
            send_data.key = bytes(key, encoding='utf8')
            send_data.type = mh.VT.VALUE_TYPE_SN.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            value = ""
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    STRGLO = self.ser.read(self.ser.in_waiting).decode("utf8")
                    print(STRGLO)
                    sn = re.findall(r"AT_SETTING_GET VALUE_TYPE_SN = \S*", STRGLO)[0] \
                        .split(" ")[-1]
                except Exception as err:
                    sn = ""
                print(sn)

            if sn == "":
                self.print_log((COLOR_RED % "SN查询失败"))
            else:
                self.print_log("SN查询成功")
        self.ser.close()  # 关闭串口
        del self.ser
        self.ser = None
        return sn

    def chooseFile(self):
        '''
        打开资源管理器 选择文件
        '''
        fileNames, fileType = QFileDialog.getOpenFileNames(None, '可选择多个素材文件', os.getcwd(),
                                                           '视频文件(*.mp4 *.MP4 *.avi *.AVI *.mov *.MOV *.gif *.GIF, *.jpg *.png *.jpeg *.lsw);;所有文件(*)')
        self.print_log((COLOR_RED % "已选择以下素材：\n") + str(fileNames))

        path_text = ""
        for fileName in fileNames:
            path_text = path_text + fileName + ";"
        self.form.choosePathEdit.setText(path_text)

    def writeWallpaper(self):
        '''
        刷写壁纸
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return False

        self.form.WriteWallpaperButton.setEnabled(False)
        try:
            os.makedirs(wallpaper_cache_dir)
        except Exception as e:
            self.form.WriteWallpaperButton.setEnabled(True)
            pass

        param = self.get_output_param()
        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            self.form.WriteWallpaperButton.setEnabled(True)
            return False
        if param["format"][0] == "lsw":
            self.print_log((COLOR_RED % "正在使用已打包好的壁纸文件"))
            shutil.copy(param["src_path"][0], wallpaper_name)
        else:
            if self.trans_format() == False:
                self.form.WriteWallpaperButton.setEnabled(True)
                return False

            if self.generateWallpaperBin() == None:
                self.form.WriteWallpaperButton.setEnabled(True)
                return False

        # 50为预留值
        rate = int(os.path.getsize(wallpaper_name) / (2097152 - 50) * 100)
        self.print_log((COLOR_RED % "本次壁纸占用全容量的 ") + str(rate) + "%")
        if os.path.getsize(wallpaper_name) > (2097152 - 50):
            self.print_log(COLOR_RED % "异常终止：壁纸数据过大，请适当降低帧率或截取更短的时间。")
            self.form.WriteWallpaperButton.setEnabled(True)
            return False

        cmd = ['SnailHeater_TOOL.py', '--port', select_com,
               '--baud', str(BAUD_RATE),
               'write_flash',
               wallpaperAddrInFlash, wallpaper_name
               ]

        self.print_log("正在烧入壁纸数据到主机，请等待（35s）......")
        try:
            esptool.main(cmd[1:])
            self.print_log("成功烧入壁纸数据到主机")
        except Exception as e:
            self.print_log(COLOR_RED % "错误：通讯异常。")
            pass
        self.form.WriteWallpaperButton.setEnabled(True)

        return True

    def generateWallpaperBin(self):
        """
        生成壁纸的bin文件
        """
        self.print_log("正在生成壁纸文件......")
        param = self.get_output_param()
        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            return None

        wallpapers = param["dst_path"]
        total = 0  # 壁纸总数（1字节）
        type = []  # 壁纸类型（1字节）jpg图片0 mjpeg视频1
        startAddr = []  # 4字节
        dataLen = []  # 4字节
        isLegal = []  # 处理过程中的标志位
        dataAddrOffset = 256  # 标记当前的数据可存地址
        for ind in range(len(wallpapers)):
            suffix = os.path.basename(wallpapers[ind]).split(".")[-1]
            if suffix == "mjpeg":
                type.append(TYPE_MJPEG)
            else:
                type.append(TYPE_JPG)
            startAddr.append(dataAddrOffset)
            fileSize = os.path.getsize(wallpapers[ind])
            dataLen.append(fileSize)  # 壁纸数据长度
            if suffix != "mjpeg" and fileSize >= 20000:
                # 超出最大长度
                isLegal.append(False)
                self.print_log(COLOR_RED % ("此图片文件过大（已被忽略）：" + param["src_path"][ind]))
            else:
                isLegal.append(True)
                dataAddrOffset += fileSize
                total += 1

        self.print_log("数据长度->" + str(dataLen))
        self.print_log("即将刷入%s张壁纸文件" % str(total))
        try:
            print(wallpaper_name)
            os.remove(wallpaper_name)
        except Exception as err:
            pass

        with open(wallpaper_name, "wb") as fbin:
            binaryData = b'' + struct.pack('=' + "1B", *[total])
            for ind in range(len(wallpapers)):
                if isLegal[ind] == False:
                    continue
                format = "1B1I1I"
                byteOrder = '='
                params = [type[ind], startAddr[ind], dataLen[ind]]
                binaryData = binaryData + struct.pack(byteOrder + format, *params)

            binaryData = binaryData + b'\x00' * (256 - len(binaryData))
            for ind in range(len(wallpapers)):
                if isLegal[ind] == False:
                    continue
                with open(wallpapers[ind], "rb") as f:
                    binaryData = binaryData + f.read()
            fbin.write(binaryData)

        self.print_log("壁纸文件生成成功：" + wallpaper_name)
        return wallpaper_name

    def trans_format(self):
        """
        格式转化
        """
        self.print_log((COLOR_RED % "正在转换（注:若视频比较大，界面会卡顿一段时间）"))
        param = self.get_output_param()
        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            return False

        cmd_resize = 'ffmpeg -i "%s" -vf "scale=-1:%s:flags=lanczos" "%s"'  # 缩放转化
        #  -c:v libx264 -crf 18
        #  -c copy
        cmd_time = 'ffmpeg -ss %s -to %s -i "%s" -c:v copy "%s"'  # 时间片截取 -i一定要放在时间参数后

        # cmd_to_rgb 的倒数第二个参数其实没什么作用，因为rgb本身就是实际的像素点
        cmd_to_rgb = 'ffmpeg -i "%s" -vf "fps=%s,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -c:v rawvideo -pix_fmt rgb565be -q:v %s "%s"'

        # （这个是为了跟cmd_to_mjpeg统一格式才加的参数）
        # fps 帧率过滤器
        # scale 缩放过滤器
        # crop 裁剪过滤器
        cmd_to_mjpeg = 'ffmpeg -i "%s" -vf "fps=%s,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -q:v %s "%s"'

        for ind in range(len(param["src_path"])):

            name = os.path.basename(param["src_path"][ind]).split(".")[0]
            suffix = os.path.basename(param["src_path"][ind]).split(".")[-1]  # 后缀名
            # 生成的中间文件名
            wallpaper_cache_name = name + "_cache." + suffix
            # 带上路径
            wallpaper_cache_path = os.path.join(wallpaper_cache_dir, wallpaper_cache_name)

            # 清理之前的记录
            try:
                os.remove(wallpaper_cache_path)
            except Exception as err:
                pass

            try:
                os.remove(param["dst_path"][ind])
            except Exception as err:
                pass

            if param["format"][ind] == "mjpeg":
                if param["end_time"] != '0':
                    middle_cmd = cmd_time % (
                        param["start_time"], param["end_time"],
                        param["src_path"][ind],
                        wallpaper_cache_path)
                    print(middle_cmd)
                    os.system(middle_cmd)
                else:
                    # 未剪切时间
                    wallpaper_cache_path = param["src_path"][ind]

                # 最终输出的文件
                trans_cmd = cmd_to_mjpeg
                # 最后的转换命令
                out_cmd = trans_cmd % (wallpaper_cache_path, param["fps"], param["height"],
                                       param["width"], param["width"], param["quality"][ind],
                                       param["dst_path"][ind])
                print(out_cmd)
                os.system(out_cmd)
            elif param["format"][ind] in IMAGE_FORMAT:
                wallpaper_cache_path = param["src_path"][ind]
                src_im: Image.Image = Image.open(wallpaper_cache_path)
                
                mode = "保持比例裁剪" if self.form.PictureModeRadioButton_0.isChecked() else "全尺寸缩放"
                if mode == "保持比例裁剪":
                    # 裁剪
                    new_width = src_im.size[1] * (int(param["width"]) / int(param["height"]))
                    # self.print_log((COLOR_RED % "宽高") + str(src_im.size[0]) + " " + str(src_im.size[1]))
                    # self.print_log((COLOR_RED % "新的宽") + str(new_width) + " " + str(src_im.size[1]))
                    rect = ((src_im.size[0] - new_width) / 2, 0, new_width, src_im.size[1])
                    src_im = src_im.crop(rect)
                if suffix == "png" or suffix == "PNG":
                    # 由于PNG是RGBA四个通道 而jpg只有RGB三个通道
                    src_im = src_im.convert('RGB')
                # 缩放
                new_im = src_im.resize((int(param["width"]), int(param["height"])), Image.BICUBIC)
                new_im.save(param["dst_path"][ind])  # , format='JPEG', quality=95

            try:
                if os.path.getsize(param["dst_path"][ind]) == 0:
                    self.print_log((COLOR_RED % "生成文件失败：") + param["src_path"][ind])
                    self.print_log((COLOR_RED % "注：") + "要求原视频的宽高比大于屏幕的宽高比")
                    return False
            except Exception as err:
                pass
                return False

        self.print_log("转换完成")

        return True

    def get_output_param(self):
        """
        得到输出参数
        """
        resolutionW, resolutionH = self.form.resolutionComboBox.currentText().split(" ")[0].split("x")

        fileNameText = self.form.choosePathEdit.text().strip()
        if fileNameText == "":
            self.print_log(COLOR_RED % "未选择素材文件")
            return False
        fileNames = fileNameText.split(";")[0:-1]
        outFileNames = []
        qualitys = []
        formats = []
        # 文件转化的创建输出目录
        for fileName in fileNames:
            name_suffix = os.path.basename(fileName).split(".")
            if name_suffix[1] in MOVIE_FORMAT:
                outFileNames.append(
                    os.path.join(wallpaper_cache_dir,
                                 name_suffix[0] + "_" + resolutionW + "x" + resolutionH + ".mjpeg"))
                formats.append("mjpeg")
                qualitys.append(self.form.qualityComboBox.currentText().strip())
            elif name_suffix[1] in IMAGE_FORMAT:
                outFileNames.append(
                    os.path.join(wallpaper_cache_dir, name_suffix[0] + "_" + resolutionW + "x" + resolutionH + ".jpeg"))
                formats.append("jpeg")
                qualitys.append("10")
            elif name_suffix[1] == "lsw":
                outFileNames.append(
                    os.path.join(wallpaper_cache_dir, wallpaper_name))
                formats.append("lsw")
                qualitys.append("10")

        print(fileNames)
        print(outFileNames)

        startTime = 0
        endTime = 0
        try:
            startTime = int(self.form.startTimeEdit.text().strip())
            endTime = int(self.form.endTimeEdit.text().strip())
        except Exception as err:
            self.print_log((COLOR_RED % "截取时间填写错误\n"))
            return False

        if startTime > endTime:
            self.print_log((COLOR_RED % "不允许开始时间大于结束时间\n"))
            return False

        return {
            "src_path": fileNames,
            "dst_path": outFileNames,
            "width": resolutionW,
            "height": resolutionH,
            "start_time": str(startTime),
            "end_time": str(endTime),
            "fps": self.form.fpsEdit.text().strip(),
            "quality": qualitys,
            "format": formats
        }

    def cleanWallpaper(self):
        '''
        清空壁纸
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        self.print_log("正在清空壁纸...")
        # esptool.py erase_region 0x20000 0x4000
        # esptool.py erase_flash
        cmd = ['SnailHeater_TOOL.py', '--port', select_com,
               'erase_region', wallpaperAddrInFlash, '0x200000']
        try:
            esptool.main(cmd[1:])
        except Exception as e:
            self.print_log(COLOR_RED % "错误：通讯异常。")
            pass

        cmd = ['SnailHeater_TOOL.py', '--port', select_com,
               '--baud', str(BAUD_RATE),
               'write_flash',
               wallpaperAddrInFlash, default_wallpaper_clean
               ]
        try:
            esptool.main(cmd[1:])
            self.print_log("成功清空壁纸.")
        except Exception as e:
            self.print_log(COLOR_RED % "错误：通讯异常。")
            pass

    def print_log(self, info):
        self.form.LogInfoTextBrowser.append(info + '\n')
        QApplication.processEvents()
        # self.form.LogInfoTextBrowser.

    def esp_reboot(self):
        """
        重启芯片(控制USB-TLL的rst dst引脚)
        :return:
        """

        time.sleep(0.1)
        select_com = self.getSafeCom()
        if select_com == None:
            return None
        self.ser = serial.Serial(select_com, BAUD_RATE, timeout=10)

        # self.ser.setRTS(True)  # EN->LOW
        # self.ser.setDTR(self.ser.dtr)
        # time.sleep(0.2)
        # self.ser.setRTS(False)
        # self.ser.setDTR(self.ser.dtr) 

        self._setDTR(False)  # IO0=HIGH
        self._setRTS(True)  # EN=LOW, chip in reset
        time.sleep(0.1)
        self._setDTR(True)  # IO0=LOW
        self._setRTS(False)  # EN=HIGH, chip out of reset
        # 0.5 needed for ESP32 rev0 and rev1
        time.sleep(0.05)  # 0.5 / 0.05
        self._setDTR(False)  # IO0=HIGH, done

        self.ser.close()  # 关闭串口
        del self.ser
        self.ser = None

    def schedule_display_time(self):
        if self.progress_bar_time_cnt > 0 and self.progress_bar_time_cnt < 99:
            self.progress_bar_time_cnt += 1
        self.form.progressBar.setValue(self.progress_bar_time_cnt)

    def UpdatePushButton_show_message(self):
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

    def WriteWallpaperButton_show_message(self):
        """
        警告拔掉AC220V消息框
        :return: None
        """

        # 创建自定义消息框
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示",
                                COLOR_RED % "管理壁纸前一定要拔掉220V电源线！")
        # 添加自定义按钮
        do = self.mbox.addButton('确定', QMessageBox.YesRole)
        cancle = self.mbox.addButton('取消', QMessageBox.NoRole)
        # 设置消息框中内容前面的图标
        self.mbox.setIcon(2)
        do.clicked.connect(self.writeWallpaper)
        self.mbox.show()

    def reflushWallpaperButton_show_message(self):
        """
        警告拔掉AC220V消息框
        :return: None
        """

        # 创建自定义消息框
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示",
                                COLOR_RED % "管理壁纸前一定要拔掉220V电源线！")
        # 添加自定义按钮
        do = self.mbox.addButton('确定', QMessageBox.YesRole)
        cancle = self.mbox.addButton('取消', QMessageBox.NoRole)
        # 设置消息框中内容前面的图标
        self.mbox.setIcon(2)
        do.clicked.connect(self.cleanWallpaper)
        self.mbox.show()


def main():
    # app = QApplication([])
    app = QApplication(sys.argv)
    download_ui = uic.loadUi("download.ui")
    # download_ui.ComComboBox.
    download_ui.show()
    app.exec_()


if __name__ == '__main__':
    # 解决不同电脑不同缩放比例问题
    # QGuiApplication.setAttribute(Qt.HighDpiScaleFactorRoundingPolicy.PassThrough)
    QApplication.setHighDpiScaleFactorRoundingPolicy(Qt.HighDpiScaleFactorRoundingPolicy.PassThrough)
    # 和designer设计的窗口比例一致
    QtCore.QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
    # 适应高DPI设备
    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
    # 解决图片在不同分辨率显示模糊问题
    # QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps)

    # main()

    downloader = DownloadController()
    downloader.run()
