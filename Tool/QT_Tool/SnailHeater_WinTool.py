# encoding: utf-8
# python 3.8.6

# pip install -r requirements.txt -i https://mirrors.aliyun.com/pypi/simple/

# pip install requests -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyserial -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyqt5==5.15.10  -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyqt5-tools -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyyaml -i https://mirrors.aliyun.com/pypi/simple/
# pip install Pillow -i https://mirrors.aliyun.com/pypi/simple/
# pip install pyinstaller==5.5.0 -i https://mirrors.aliyun.com/pypi/simple/
# pip install esptool==4.7.0 -i https://mirrors.aliyun.com/pypi/simple/

# 环境搭建学习文旦 https://github.com/Jacob-xyb/PyQt_Notes/blob/master/PyQt5.md
# QT官网：https://doc.qt.io/qt-5/index.html
# QT教程  https://b23.tv/9R6dbDA
# QT项目学习课件 https://doc.itprojects.cn/0001.zhishi/python.0008.pyqt5rumen/index.html

# pyinstaller --icon ./images/SnailHeater_256.ico -w -F SnailHeater_WinTool.py

import sys
import os
import time
# import threading
import re
import yaml  # pip install pyyaml
import requests
import traceback
import struct
import shutil
import io
from PIL import Image
import datetime

import serial  # pip install pyserial
import serial.tools.list_ports
# from PyQt5.Qt import *
from PyQt5.Qt import QWidget, QApplication
from PyQt5 import uic, QtCore
from PyQt5.QtWidgets import QMessageBox, QApplication, QMainWindow, QFileDialog
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtCore import pyqtSignal, QThread
from PyQt5.QtCore import Qt

import massagehead as mh
import MyRtttl as rtttl
# import spiffsgen
# import lvgl_image_converter.lv_img_conv as image_conv
import lvgl_image_converter as image_conv
# from lvgl_image_converter import lv_img_conv as image_conv

import esptool  # sys.path.append("./esptool_v41") or pip install esptool==4.1
# 需要修改esptool源码loader.py中得一个文件路径
# STUBS_DIR = os.path.join(os.path.dirname(__file__), "targets", "stub_flasher")
# 修改为如下
# STUBS_DIR = os.path.join(os.getcwd(), "stub_flasher")


from download import Ui_SanilHeaterTool
import common

SH_SN = None
if SH_SN == None and os.path.exists("SnailHeater_SN.py"):
    import SnailHeater_SN as SH_SN

    print("激活模块已添加")

COLOR_RED = '<span style=\" color: #ff0000;\">%s</span>'
ERR_UART_TEXT = "错误：通讯异常。检查设备，或拔插TypeC数据线重试！"

cur_dir = os.getcwd()  # 当前目录
# 生成的文件目录
gen_path = os.path.join(cur_dir, "Generate")
# 背景图片
backgroud_base = os.path.join(gen_path, "Backgroud")
# 背景缓存目录
backgroud_cache_dir = os.path.join(gen_path, "Cache", "Backgroud")
# 默认背景
default_backgroud_280 = os.path.join(cur_dir, "base_data/Backgroud_280x240.bin")
default_backgroud_320 = os.path.join(cur_dir, "base_data/Backgroud_320x240.bin")
default_backgroud = default_backgroud_280
# 默认壁纸
default_wallpaper_280 = os.path.join(cur_dir, "base_data/Wallpaper_280x240.lsw")
default_wallpaper_320 = os.path.join(cur_dir, "base_data/Wallpaper_320x240.lsw")
default_wallpaper_clean = os.path.join(cur_dir, "base_data/WallpaperClean.lsw")
default_wallpaper = default_wallpaper_280
# coredump目录
coredump_dir = os.path.join(gen_path, "Coredump")
# 文件缓存目录
wallpaper_cache_dir = os.path.join(gen_path, "Cache", "Wallpaper")
# 文件生存目录
wallpaper_path = os.path.join(gen_path, "Wallpaper")
# 壁纸文件
wallpaper_name = os.path.join(wallpaper_path, "Wallpaper.lsw")
waLLPAPER_JPG_MAX_SIZE = 17500

TYPE_JPG = 0
TYPE_MJPEG = 1
TYPE_RTTTL = 127
IMAGE_FORMAT = ["jpg", "JPG", "jpeg", "JPEG", "png", "PNG"]
MOVIE_FORMAT = ["mp4", "MP4", "avi", "AVI", "mov", "MOV"]

CHIP_ID_KNOWN = ""
CHIP_ID_S2 = "S2"
CHIP_ID_S3 = "S3"

g_curr_chip_id = CHIP_ID_KNOWN
g_autoActivate = True

DOWN_CLEAR_FLAG_CLEAR = "清空式"
DOWN_CLEAR_FLAG_UPDATE = "更新式"
g_DownloadClearFlag = None

count = 0

# 读取配置信息
cfg_fp = open("SnailHeater_Tool.yaml", "r", encoding="utf-8")

win_cfg = yaml.load(cfg_fp, Loader=yaml.SafeLoader)["windows_tool"]

tool_name = win_cfg["tool_name"] \
    if "tool_name" in win_cfg.keys() else ""
info_url_0 = win_cfg["info_url_0"] \
    if "info_url_0" in win_cfg.keys() else ""
info_url_1 = win_cfg["info_url_1"] \
    if "info_url_1" in win_cfg.keys() else ""
qq_info = win_cfg["qq_info"].split(",") \
    if "qq_info" in win_cfg.keys() else ["", ""]
temp_sn_recode_path = win_cfg["temp_sn_recode_path"] \
    if "temp_sn_recode_path" in win_cfg.keys() else None
search_sn_registrant_url = win_cfg["search_sn_registrant_url"] \
    if "search_sn_registrant_url" in win_cfg.keys() else None
activate_sn_url = win_cfg["activate_sn_url"] \
    if "activate_sn_url" in win_cfg.keys() else None
get_firmware_new_ver_url = win_cfg["get_firmware_new_ver_url"] \
    if "get_firmware_new_ver_url" in win_cfg.keys() else None
get_tool_new_ver_url = win_cfg["get_tool_new_ver_url"] \
    if "get_tool_new_ver_url" in win_cfg.keys() else None
baud_rate = win_cfg["baud_rate"] \
    if "baud_rate" in win_cfg.keys() else ""
info_baud_rate = win_cfg["info_baud_rate"] \
    if "info_baud_rate" in win_cfg.keys() else ""
firmware_dir = win_cfg["firmware_dir"] \
    if "firmware_dir" in win_cfg.keys() else None

cfg_fp.close()

support = None


def get_wallpaper_addr_in_flash(chip_id):
    # 背景图
    if chip_id == CHIP_ID_S2:
        return '0x00200000'
    elif chip_id == CHIP_ID_S3:
        return '0x004D0000'

        
    else:
        return '0x00200000'


def get_backgroup_addr_in_flash(chip_id):
    # 壁纸文件
    if chip_id == CHIP_ID_S2:
        return '0x180000'
    elif chip_id == CHIP_ID_S3:
        return '0x480000'
    else:
        return '0x180000'


def getVerValue(ver):
    """
    获取版本的值
    """
    if "UNKNOWN" in ver:
        return 100 * 100 * 100  # 返回最大值 int默认不能太大
    value1_list = ver[1:].split(".")
    sum = 0
    for val in value1_list:
        # 第三位小版本使是S3引入的，故需要特殊处理第三位版本只有2位数的情况
        # curVal = int(val) * 100 if int(val) < 100 else int(val)
        sum = sum * 100 + int(val)
    return sum


def get_version():
    global support
    try:
        # This SH_TOOL version supports [ SH_SW v2.1.1~SH_SW UNKNOWN ].
        # Latest version [ SH_TOOL v2.7.8 ].
        response = requests.get(get_tool_new_ver_url + "/" + common.TOOL_VERSION, timeout=3)  # , verify=False
        new_version_info = re.findall(r'SH_TOOL v\d{1,2}\.\d{1,2}\.\d{1,2}', response.text.split("</p><p>")[1])
        new_version = new_version_info[0].split(" ")[1].strip()

        support_version_info = response.text.split("[")[1][:-3].split("]")[0].strip()
        start = support_version_info.split("~")[0].strip().split(" ")[1]
        end = support_version_info.split("~")[1].strip().split(" ")[1]
        support = [start, end]
        if common.TOOL_VERSION == new_version:
            return "[已是最新版本]"
        else:
            return "[推荐升级最新版本 " + new_version + "]"
    except Exception as err:
        print(str(traceback.format_exc()))
        return "[无法获取到最新版本]"


def get_flash_size(select_com):
    """
    :param select_com:串口号
    :return:size, size_text
    """
    flash_size = 0
    flash_size_text = ""
    try:
        printed_data = ""

        # 创建一个字符串缓冲区
        output_buffer = io.StringIO()

        # 将sys.stdout重定向到缓冲区
        original_stdout = sys.stdout
        sys.stdout = output_buffer

        # 调用函数
        cmd = ['--port', select_com,
               #    '--after', 'no_reset',
               'flash_id']  # hard_reset
        esptool.main(cmd)

        # 恢复sys.stdout
        sys.stdout = original_stdout

        # 获取打印的数据
        printed_data = output_buffer.getvalue()

        line_data = printed_data.split("\n")
        print("line_data", line_data, "\n\n")
        for line in line_data:
            if "Detected flash size: " in line:
                flash_size_text = line.split(": ")[1]
        print('flash_size_text = ', flash_size_text)
        flash_size = int(flash_size_text[:-2]) * 1024 * 1024

    except Exception as err:
        print(str(traceback.format_exc()))
        return 0, "0MB"

    return flash_size, flash_size_text


def get_chip_id(select_com):
    """
    :param select_com:串口号
    :return:size, size_text
    """
    chip_id = CHIP_ID_KNOWN
    try:
        printed_data = ""

        # 创建一个字符串缓冲区
        output_buffer = io.StringIO()

        # 将sys.stdout重定向到缓冲区
        original_stdout = sys.stdout
        sys.stdout = output_buffer

        # 调用函数
        cmd = ['--port', select_com,
               'chip_id']
        esptool.main(cmd)

        # 恢复sys.stdout
        sys.stdout = original_stdout

        # 获取打印的数据
        printed_data = output_buffer.getvalue()

        line_data = printed_data.split("\n")
        print("line_data", line_data, "\n\n")
        for line in line_data:
            if "Detecting chip type... ESP" in line:
                chip_id_text = line.split("... ")[1]
                if "ESP32-S2" in chip_id_text:
                    chip_id = CHIP_ID_S2
                elif "ESP32-S3" in chip_id_text:
                    chip_id = CHIP_ID_S3

    except Exception as err:
        print(str(traceback.format_exc()))
        return CHIP_ID_KNOWN

    return chip_id


class FirmwareDownloader(QThread):
    print_signal = pyqtSignal(str)
    ret_finish = pyqtSignal(bool)
    get_machine_software_ver = pyqtSignal(bool)

    def __init__(self, mode, select_com, firmware_path):
        super().__init__()
        self.mode = mode
        self.select_com = select_com
        self.firmware_path = firmware_path

    def run(self):
        # 在这里编写需要在子线程中执行的代码

        """
        下载操作主体
        :param mode:下载模式
        :param select_com:串口号
        :param firmware_path:固件文件路径
        :return:None
        """
        global g_curr_chip_id
        global default_wallpaper
        try:
            # self.get_machine_software_ver.emit(True)

            if self.mode == DOWN_CLEAR_FLAG_CLEAR:
                self.print_log("正在清空主机数据...")
                # esptool.py erase_region 0x00000 0x400000
                # esptool.py erase_flash
                # cmd = ['--port', select_com, 'erase_flash']
                cmd = ['--port', self.select_com, 'erase_region',
                       '0x00000', '0x400000']
                try:
                    esptool.main(cmd)
                    self.print_log("完成清空！")
                except Exception as e:
                    self.print_log(COLOR_RED % ERR_UART_TEXT)

            g_curr_chip_id = get_chip_id(self.select_com)
            print("chip_id = ", g_curr_chip_id)

            flash_size = 0
            flash_size_text = "0MB"
            if g_curr_chip_id == CHIP_ID_S2:
                flash_size, flash_size_text = get_flash_size(self.select_com)
            elif g_curr_chip_id == CHIP_ID_S3:
                # flash_size = 32 * 1024 * 1024
                # flash_size_text = "32MB"
                # self.print_log("正在获取存空间大小...")
                flash_size, flash_size_text = get_flash_size(self.select_com)
                if flash_size < 8 * 1024 * 1024:
                    self.print_log("识别到不支持的Flash大小：" + (COLOR_RED % flash_size_text))
                    return None

            # flash_size = 8 * 1024 * 1024
            # flash_size_text = "8MB"
            self.print_log("已获取到Flash空间大小为：" + (COLOR_RED % flash_size_text))

            if flash_size == 0:
                self.print_log(COLOR_RED % "错误：储存空间为0，刷机终止。")
                self.print_log(COLOR_RED % ERR_UART_TEXT)
                self.ret_finish.emit(False)
                self.quit()
                return None
            exMediaParam = []
            if self.mode == DOWN_CLEAR_FLAG_CLEAR:
                exMediaParam = [
                    get_backgroup_addr_in_flash(g_curr_chip_id), default_backgroud,
                    get_wallpaper_addr_in_flash(g_curr_chip_id), default_wallpaper]
            cmd = []
            curSWVersion = re.findall(r'SH_SW_v\d{1,2}\.\d{1,2}\.\d{1,2}', self.firmware_path)[0][6:].strip()
            print("curSWVersion", curSWVersion)
            if getVerValue(curSWVersion) > getVerValue("v2.1.17"):
                #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 S2_bootloader_dio_40m.bin 0x00008000 S2_partitions.bin 0x0000e000 S2_boot_app0.bin 0x00010000
                if g_curr_chip_id == CHIP_ID_S2:
                    flash_size_text = flash_size_text if flash_size_text in ["4MB", "8MB", "16MB", "32MB",
                                                                             "64MB"] else "4MB"
                    cmd = ['SnailHeater_WinTool.py', '--port', self.select_com,
                           '--baud', baud_rate,
                           '--after', 'hard_reset',
                           'write_flash',
                           '--flash_size', flash_size_text,
                           '0x00001000', "./base_data/%s_bootloader_%s.bin" % (g_curr_chip_id, flash_size_text),
                           '0x00008000', "./base_data/%s_partitions_%s.bin" % (g_curr_chip_id, flash_size_text),
                           '0x0000e000', "./base_data/%s_boot_app0.bin" % (g_curr_chip_id),
                           '0x00010000', self.firmware_path
                           ] + exMediaParam
                elif g_curr_chip_id == CHIP_ID_S3:
                    flash_size_text = flash_size_text if flash_size_text in ["4MB", "8MB", "16MB", "32MB"] else "32MB"
                    cmd = ['SnailHeater_WinTool.py', '--port', self.select_com,
                           '--baud', baud_rate,
                           '--after', 'hard_reset',
                           'write_flash',
                           '--flash_size', flash_size_text,
                           '0x00000000', "./base_data/%s_bootloader_%s.bin" % (g_curr_chip_id, flash_size_text),
                           '0x00008000', "./base_data/%s_partitions_%s.bin" % (g_curr_chip_id, flash_size_text),
                           # '0x0000e000', "./base_data/%s_boot_app0.bin"% (g_curr_chip_id) ,
                           '0x00010000', self.firmware_path
                           ] + exMediaParam
            elif getVerValue(curSWVersion) > getVerValue("v1.9.8"):
                # S2版本支持的最大Flash容量为16M

                #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 S2_bootloader_dio_40m.bin 0x00008000 S2_partitions.bin 0x0000e000 S2_boot_app0.bin 0x00010000
                flash_size_text = flash_size_text if flash_size_text in ["4MB", "8MB", "16MB"] else "16MB"
                cmd = ['SnailHeater_WinTool.py', '--port', self.select_com,
                       '--baud', baud_rate,
                       '--after', 'hard_reset',
                       'write_flash',
                       '--flash_size', flash_size_text,
                       '0x00001000', "./old_base_data_2117/%s_bootloader_%s.bin" % (g_curr_chip_id, flash_size_text),
                       '0x00008000', "./old_base_data_2117/%s_partitions_%s.bin" % (g_curr_chip_id, flash_size_text),
                       '0x0000e000', "./old_base_data_2117/%s_boot_app0.bin" % (g_curr_chip_id),
                       '0x00010000', self.firmware_path,
                       get_wallpaper_addr_in_flash(g_curr_chip_id),
                       default_wallpaper.replace("base_data", "old_base_data_2117")
                       ]
            print(cmd)

            self.print_log("开始刷写固件...")
            try:
                esptool.main(cmd[1:])
            except Exception as e:
                print(str(traceback.format_exc()))
                self.print_log(COLOR_RED % ERR_UART_TEXT)
                self.ret_finish.emit(False)
                self.quit()
                return None

            self.print_log(COLOR_RED % "刷机结束！")
            self.print_log("刷机流程完毕，请保持typec通电等待焊台屏幕将会亮起后才能断电。")
            self.print_log((COLOR_RED % "注：") + "更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待10s左右。")
            self.print_log("如25s后始终未能自动亮屏，请手动按一次复位键或者拔插一次typec接口再次等待10s。\n")

        except Exception as err:
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            print(str(traceback.format_exc()))
            self.ret_finish.emit(False)
            self.quit()
            return None

        self.ret_finish.emit(True)
        self.quit()

    def print_log(self, massage):
        self.print_signal.emit(massage)

    def __del__(self):
        self.quit()
        self.wait()


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
                                                tool_name + common.TOOL_VERSION + " " + get_version()))

        # 设置文本可复制
        self.form.LinkInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.UpdateLogLinkInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.QQInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.QQInfolabel_2.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.form.sourceInfolabel.setTextInteractionFlags(Qt.TextSelectableByMouse)

        self.form.ComComboBox.clicked.connect(self.scan_com)  # 信号与槽函数的绑定
        self.form.FirmwareComboBox.clicked.connect(self.scan_firmware)
        self.form.QueryPushButton.clicked.connect(self.query_button_click)
        self.form.chooseWPButton.clicked.connect(self.chooseWpFile)
        self.form.chooseFileButton.clicked.connect(self.chooseBgFile)
        self.form.ActivatePushButton.clicked.connect(self.act_button_click)
        # self.form.QueryPushButton_2.clicked.connect(self.read_coredump)

        # self.form.UpdatePushButton.clicked.connect(self.update_button_click)
        self.form.UpdatePushButton.clicked.connect(self.UpdatePushButton_show_message)
        # self.form.WriteWallpaperButton.clicked.connect(self.writeWallpaper)
        self.form.WriteWallpaperButton.clicked.connect(self.WriteWallpaperButton_show_message)
        self.form.reflushWallpaperButton.clicked.connect(self.reflushWallpaperButton_show_message)
        self.form.WriteWallpaperButton_2.clicked.connect(self.WriteBgButton_show_message)
        self.form.CanclePushButton.clicked.connect(self.cancle_button_click)
        self.form.uiReadColorBnt.clicked.connect(self.query_color_button_click)
        self.form.uiWriteColorBnt.clicked.connect(self.write_color_button_click)

        # 设置提示信息
        self.form.Infolabel.setText(_translate("SanilHeaterTool", "使用教程："))
        self.form.QueryPushButton.setToolTip("获取机器码(SN)")
        self.form.QueryPushButton_2.setToolTip("暂未开通")
        self.form.ActivatePushButton.setToolTip("填入SN，点此激活")
        self.form.UpdateModeMethodRadioButton.setToolTip("保留用户的设置信息，只在固件上做更新")
        self.form.ClearModeMethodRadioButton.setToolTip("将会清空芯片内所有可清空的信息，想要完全纯净的刷固件可选此项")
        # self.form.autoScaleBox.setToolTip("自适应长宽。若未勾选则以指定长宽比截取中心区域")
        self.form.chooseWPButton.setToolTip("选择素材文件的路径（可选多项）")
        self.form.chooseFileButton.setToolTip("选择图片文件的路径")
        self.form.WriteWallpaperButton.setToolTip("将选择好的素材转换并刷写到焊台上")
        self.form.WriteWallpaperButton_2.setToolTip("将选择好的素材转换并刷写到焊台上")
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
        self.form.uiForwardROColorlabel.setToolTip("RGB颜色格式为(0x)7AFE89")
        self.form.uiForwardClickedColorlabel.setToolTip("RGB颜色格式为(0x)7AFE89")
        self.form.uiForwardROColorLineEdit.setToolTip("RGB颜色格式为(0x)7AFE89")
        self.form.uiForwardClickedColorLineEdit.setToolTip("RGB颜色格式为(0x)7AFE89")

        self.form.QQInfolabel.setText(_translate("SanilHeaterTool", qq_info[0]))
        self.form.QQInfolabel_2.setText(_translate("SanilHeaterTool", qq_info[1]))
        self.form.LinkInfolabel.setText(_translate("SanilHeaterTool", info_url_0))
        self.form.UpdateLogLinkInfolabel.setText(_translate("SanilHeaterTool", info_url_1))
        self.form.resolutionComboBox.addItems(["280x240 (一、二车)", "320x240 (三车)"])
        self.form.qualityComboBox.addItems([str(num) for num in range(1, 20)])
        self.form.qualityComboBox.setCurrentText("5");
        self.form.fpsEdit.setText("20")
        self.form.startTimeEdit.setText("0")
        self.form.endTimeEdit.setText("0")
        self.form.VerInfolabel.setStyleSheet('color: red')
        self.form.sourceInfolabel.setStyleSheet('color: red')
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
        list_file = os.listdir(firmware_dir)
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
        global g_curr_chip_id
        if self.ser != None:  # 串口打开标志
            return None

        select_com = self.form.ComComboBox.currentText().split(" -> ")[0].strip()

        com_list = [com_obj[0] for com_obj in list(serial.tools.list_ports.comports())]

        if CHIP_ID_S3 == g_curr_chip_id:
            select_com = com_list[0]
            return select_com

        if select_com not in com_list:
            self.print_log((COLOR_RED % "错误提示：") +
                           "无法检测到指定串口设备，先确认 CH340 驱动是否正常或尝试 typec 调换方向。\n")
            return None
        return select_com

    def read_coredump(self):
        global count
        select_com = self.getSafeCom()
        if select_com == None:
            return None
        self.ser = serial.Serial(select_com, baud_rate, timeout=10)

        count = (count + 1) % 4
        if count == 0:
            self.ser.setDTR(False)  # IO0=HIGH
            self.ser.setRTS(True)  # EN=LOW, chip in reset
        elif count == 1:
            self.ser.setDTR(True)  # IO0=LOW, chip out of reset
            self.ser.setRTS(True)  # EN=LOW, chip in reset
        elif count == 2:
            self.ser.setDTR(True)  # IO0=LOW, chip out of reset
            self.ser.setRTS(False)  # EN=HIGH, chip out of reset
        else:
            self.ser.setDTR(False)  # IO0=HIGH
            self.ser.setRTS(False)  # EN=HIGH, chip out of reset

        time.sleep(0.5)
        serial_data = self.ser.read(self.ser.in_waiting)
        print("Count = ", count, "\tState: ", serial_data)

        self.release_serial()
        return None

        """
        读取coredump的数据
        """
        self.print_log("正在获取异常信息...")

        self.hard_reset()
        time.sleep(1)
        machine_code = self.get_machine_code()
        if None == machine_code:
            self.print_log(COLOR_RED % "无法获取异常信息")
            return None
        try:
            # idf.py coredump-info -C D:\Workspace\OpenWorkspace\PersionSnailHeater\Software\firmware_idf -c D:\Workspace\OpenWorkspace\SnailHeater\Tool\QT_Tool\Coredump\coredump.img
            # idf.py coredump-info -s ./coredump.elf
            # idf.py coredump-info -c ./coredump.elf
            try:
                os.makedirs(coredump_dir)
            except Exception as e:
                pass

            select_com = self.getSafeCom()
            if select_com == None:
                self.print_log(COLOR_RED % "激活操作异常，激活中止...")
                return None

            nowTime = datetime.datetime.now().strftime('%Y%m%d_%H_%M_%S')
            coredumpFile = os.path.join(coredump_dir, "SH_%s_%s.coredump" % (nowTime, machine_code))
            print(coredumpFile)

            cmd = ['SnailHeater_WinTool.py', '--port', select_com,
                   '--baud', baud_rate,
                   'read_flash', '0x1D0000',
                   '0x10000', coredumpFile
                   ]

            esptool.main(cmd[1:])

            self.print_log(COLOR_RED % ("异常报告已生成：" + coredumpFile))
        except Exception as e:
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            pass

    def act_button_click(self):
        self.print_log("正在激活设备...")

        select_com = self.getSafeCom()
        if select_com == None:
            self.print_log(COLOR_RED % "激活操作异常，激活中止...")
            return None

        self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)

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
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")
                    
                    match_info = re.findall(r"Success", serial_data)
                    if match_info != []:
                        act_ret = True
                except Exception as err:
                    print(str(traceback.format_exc()))

            if act_ret == True:
                self.print_log("激活成功")
            else:
                self.print_log("激活失败")

        self.release_serial()

    def write_color_button_click(self):
        self.print_log("正在写入UI前台颜色...")

        if len(self.form.uiForwardROColorLineEdit.text().strip()) != 6 or \
                len(self.form.uiForwardClickedColorLineEdit.text().strip()) != 6:
            self.print_log(COLOR_RED % "RGB格式错误")
            return None

        select_com = self.getSafeCom()
        if select_com == None:
            self.print_log(COLOR_RED % "写入UI前台颜色异常...")
            return None

        self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)

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
            send_data.type = mh.VT.VALUE_TYPE_FORWARD_COLOR.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)

            value = self.form.uiForwardROColorLineEdit.text().strip().upper() + " " + \
                    self.form.uiForwardClickedColorLineEdit.text().strip().upper()
            print(value)
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")
                    
                    match_info = re.findall(r"Color Success", serial_data)
                    if match_info != []:
                        act_ret = True
                except Exception as err:
                    print(str(traceback.format_exc()))

            if act_ret == True:
                self.print_log("写入UI前台颜色成功")
            else:
                self.print_log(COLOR_RED % "写入UI前台颜色失败")

        self.release_serial()

    def auto_active(self):
        """
        自动激活
        """
        self.print_log((COLOR_RED % "执行自动激活程序"))
        self.hard_reset()  # 复位芯片
        time.sleep(2)  # 等待重启结束查询

        if self.query_button_click() == False:
            return False
        try:
            self.act_button_click()
        except Exception as err:
            print(str(traceback.format_exc()))

    def query_button_click(self):
        """
        获取用户识别码 显示在用户识别码的信息框里
        获取激活码 显示在激活码的信息框里
        :return: None
        """
        self.print_log("获取机器码（用户识别码）...")
        machine_code = self.get_machine_code()
        self.form.UICLineEdit.setText(machine_code)

        if machine_code == None:
            self.print_log(COLOR_RED % "获取机器码异常")
            return False

        self.print_log("\n获取本地激活码（SN）...")
        sn = ""
        # sn = self.get_sn()    # 从本机查询SN号（本接口可用）
        registrant = "未知"
        # 尝试联网查询
        if sn == "":
            try:
                if activate_sn_url != None and activate_sn_url != "":
                    self.print_log("联网查询激活码（管理员模式）...")
                    response = requests.get(activate_sn_url + machine_code, timeout=3)  # , verify=False
                else:
                    self.print_log("联网查询激活码...")
                    response = requests.get(search_sn_registrant_url + machine_code, timeout=3)  # , verify=False
                    print(search_sn_registrant_url + machine_code)
                    print(response)
                    # 注册者信息
                    registrant = response.text.strip().split("\t")[1]
                # sn = re.findall(r'\d+', response.text)
                sn = response.text.strip().split("\t")[0]
                self.print_log("sn " + str(sn))
            except Exception as err:
                print(str(traceback.format_exc()))
                self.print_log("联网异常")
                return False

        self.form.SNLineEdit.setText(sn)
        self.form.sourceInfolabel.setText("本机器来源 " + registrant)

        try:
            if sn != "":
                sn_record = open(temp_sn_recode_path, 'a', encoding="utf-8")
                sn_record.write(machine_code + "\t" + sn + "\n")
                sn_record.close()
            else:
                return False
        except Exception as err:
            print(str(traceback.format_exc()))
            self.print_log("获取异常异常")

        return True

    def query_color_button_click(self):
        """
        获取颜色信息
        :return: None
        """
        self.print_log("获取UI前台颜色...")
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        color = ""
        try:
            self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)
        except Exception as err:
            self.print_log((COLOR_RED % "串口打开失败"))
            return color

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
            send_data.type = mh.VT.VALUE_TYPE_FORWARD_COLOR.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            value = ""
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")
                    
                    color = re.findall(r"VALUE_TYPE_FORWARD_COLOR = \S* \S*", serial_data)[0] \
                                .split(" ")[-2:]
                except Exception as err:
                    print(str(traceback.format_exc()))
                    color = ""
                print("收到的数据：", color)

            if color == "":
                self.print_log((COLOR_RED % "获取UI前台颜色失败"))
            else:
                self.print_log("获取UI前台颜色成功")
                uiForwardROColorText = hex(int(color[0]))[2:].upper()
                uiForwardROColorText = '0' * (6 - len(uiForwardROColorText)) + uiForwardROColorText
                self.form.uiForwardROColorLineEdit.setText(uiForwardROColorText)

                uiForwardClickedColorText = hex(int(color[1]))[2:].upper()
                uiForwardClickedColorText = '0' * (6 - len(uiForwardClickedColorText)) + uiForwardClickedColorText
                self.form.uiForwardClickedColorLineEdit.setText(uiForwardClickedColorText)

        self.release_serial()
        return color

    def reset_ui_button(self):

        self.progress_bar_time_cnt = 0  # 复位进度条

        self.form.UpdatePushButton.setEnabled(True)
        self.form.UpdateModeMethodRadioButton.setEnabled(True)
        self.form.ClearModeMethodRadioButton.setEnabled(True)

    def update_button_click(self):
        """
        按下 刷机 按键后触发的检查、刷机操作
        :return: None
        """
        global default_wallpaper
        global default_backgroud
        global g_autoActivate
        global g_DownloadClearFlag

        try:
            self.print_log("准备更新固件...")
            self.form.UpdateModeMethodRadioButton.setEnabled(False)
            self.form.ClearModeMethodRadioButton.setEnabled(False)
            self.form.UpdatePushButton.setEnabled(False)

            firmware_path = self.form.FirmwareComboBox.currentText().strip()
            g_DownloadClearFlag = DOWN_CLEAR_FLAG_UPDATE if self.form.UpdateModeMethodRadioButton.isChecked() else DOWN_CLEAR_FLAG_CLEAR

            select_com = self.getSafeCom()
            if select_com == None or firmware_path == "":
                if firmware_path == "":
                    self.print_log((COLOR_RED % "错误提示：") + "未查询到固件文件！")
                self.reset_ui_button()
                return False

            if support != None:
                curSWVersion = re.findall(r'SH_SW_v\d{1,2}\.\d{1,2}\.\d{1,2}', firmware_path)[0][6:].strip()
                if getVerValue(support[0]) > getVerValue(curSWVersion) or getVerValue(support[1]) < getVerValue(
                        curSWVersion):
                    self.print_log((COLOR_RED % "错误提示：") + "当前版本管理工具不支持该固件")
                    self.form.UpdateModeMethodRadioButton.setEnabled(True)
                    self.form.ClearModeMethodRadioButton.setEnabled(True)
                    self.form.UpdatePushButton.setEnabled(True)
                    return False

            self.print_log("串口号：" + (COLOR_RED % select_com))
            self.print_log("固件文件：" + (COLOR_RED % firmware_path))
            self.print_log("刷机模式：" + (COLOR_RED % g_DownloadClearFlag))

            if "③" in firmware_path:
                default_wallpaper = default_wallpaper_320
                default_backgroud = default_backgroud_320
            elif "①" in firmware_path or "②" in firmware_path:
                default_wallpaper = default_wallpaper_280
                default_backgroud = default_backgroud_280
            else:
                default_wallpaper = default_wallpaper_clean

            if not os.path.exists(default_wallpaper):
                default_wallpaper = default_wallpaper_clean

            all_time = 6  # 粗略认为连接并复位芯片需要0.5s钟 自动激活需要6s
            if g_DownloadClearFlag == DOWN_CLEAR_FLAG_CLEAR:
                all_time += 24
            else:
                all_time += 5
            # 此文件列表的 boot_app0 bootloader partitions 文件均不是特指，
            # 但与实际要写入的文件大小无异,只为了方便计算文件大小
            file_list = ["./base_data/S2_boot_app0.bin",
                         "./base_data/S2_bootloader_4MB.bin",
                         "./base_data/S2_partitions_4MB.bin",
                         #  "./base_data/S2_tinyuf2.bin",
                         os.path.join(firmware_dir, firmware_path)]
            if g_DownloadClearFlag == DOWN_CLEAR_FLAG_CLEAR:
                file_list = file_list + [default_backgroud, default_wallpaper]
            
            for filepath in file_list:
                all_time = all_time + os.path.getsize(filepath) * 10 / int(baud_rate)

            self.print_log("刷机预计需要：" + (COLOR_RED % (str(all_time)[0:5] + "s")))

            # 进度条进程要在下载进程之前启动（为了在下载失败时可以立即查杀进度条进程）
            self.progress_bar_timer.start(int(all_time / 100 * 1000))

            # 标记是否执行自动激活
            if "Pro" in firmware_path:
                g_autoActivate = False
            else:
                g_autoActivate = True

            if self.download_thread != None:
                del self.download_thread

            self.download_thread = FirmwareDownloader(g_DownloadClearFlag, select_com, os.path.join(firmware_dir, firmware_path))
            self.download_thread.print_signal.connect(self.print_log)
            self.download_thread.ret_finish.connect(self.down_action_finish)
            self.download_thread.get_machine_software_ver.connect(self.get_machine_software_ver)
            self.download_thread.start()
            self.progress_bar_time_cnt = 1  # 间接启动进度条更新                


        except Exception as err:
            print(str(traceback.format_exc()))

    def down_action_finish(self, isOk):
        global g_curr_chip_id
        global g_autoActivate
        global g_DownloadClearFlag
        if isOk == True and g_DownloadClearFlag == DOWN_CLEAR_FLAG_CLEAR:
            time.sleep(4)  # 等待文件系统初始化完成
            if g_autoActivate == True:
                self.auto_active()
            else:
                self.print_log(COLOR_RED % "请通电30秒后手动，查询激活码并激活。")

        self.reset_ui_button()
        g_curr_chip_id = CHIP_ID_KNOWN

    def cancle_button_click(self):
        """
        取消下载固件
        :return: None
        """

        self.print_log("手动停止更新固件...")

        if self.download_thread != None:
            # 杀线程
            try:
                # common._async_raise(self.download_thread)
                # self.download_thread = None

                self.download_thread.terminate()
                self.download_thread.wait()

                self.release_serial()
            except Exception as err:
                print(str(traceback.format_exc()))
                print(err)

        # self.scan_com()

        self.reset_ui_button()
        self.form.progressBar.setValue(0)

    def release_serial(self):
        """
        关闭串口
        """
        if self.ser != None:
            self.ser.close()  # 关闭串口
            del self.ser
            self.ser = None

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
            if 'SnailHeater_v' in response.text.strip() or 'SH_SW v' in response.text.strip():
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
        machine_code = None

        select_com = self.getSafeCom()
        if select_com == None:
            return machine_code

        try:
            self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)
        except Exception as err:
            print(str(traceback.format_exc()))
            self.print_log((COLOR_RED % "串口打开失败"))
            return machine_code

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
            print("Sent: ", send_data.encode('!'))

            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                print("Recv OK")
                try:
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")
                    
                    machine_code = re.findall(r"VALUE_TYPE[_MC]* = \d*", serial_data)[0] \
                        .split(" ")[-1]
                except Exception as err:
                    print("try Fail")
                    print(str(traceback.format_exc()))
                    pass
                print("machine_code = ", machine_code)

            if machine_code == None:
                self.print_log((COLOR_RED % "机器码查询失败"))
            else:
                self.print_log("机器码查询成功")

        self.release_serial()
        return machine_code

    def get_sn(self):
        '''
        查询SN
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        sn = ""
        try:
            self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)
        except Exception as err:
            self.print_log((COLOR_RED % "串口打开失败"))
            return sn

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
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")
                    
                    sn = re.findall(r"VALUE_TYPE[_SN]* = \S*", serial_data)[0] \
                        .split(" ")[-1]
                except Exception as err:
                    print(str(traceback.format_exc()))
                    sn = ""
                print(sn)

            if sn == "":
                self.print_log((COLOR_RED % "本地激活码查询失败"))
            else:
                self.print_log("本地激活码查询成功")

        self.release_serial()
        return sn

    def get_machine_software_ver(self, param):
        '''
        查询固件的版本
        '''
        select_com = self.getSafeCom()
        if select_com == None:
            return None

        sw_ver = ""
        try:
            self.ser = serial.Serial(select_com, info_baud_rate, timeout=10)
        except Exception as err:
            self.print_log((COLOR_RED % "串口打开失败"))
            return sw_ver

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
            send_data.type = mh.VT.VALUE_TYPE_SH_SOFTWARE_VER.to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            value = ""
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode('!'))
            self.ser.write(send_data.encode('!'))

            time.sleep(1)
            if self.ser.in_waiting:
                try:
                    serial_data = self.ser.read(self.ser.in_waiting)
                    print("\nserial_data -> ", serial_data)
                    serial_data = serial_data.replace(b"\x55\xaa", b"##").decode("utf8")

                    sw_ver = re.findall(r"VALUE_TYPE_SH_SOFTWARE_VER = \S*", serial_data)[0] \
                        .split(" ")[-1]
                except Exception as err:
                    print(str(traceback.format_exc()))
                    sw_ver = ""
                print(sw_ver)

            if sw_ver == "":
                self.print_log((COLOR_RED % "查询固件版本失败"))
            else:
                self.print_log("目前焊台软件版本为：" + sw_ver)

        self.release_serial()
        return sw_ver

    def chooseWpFile(self):
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

    def chooseBgFile(self):
        '''
        打开资源管理器 选择文件
        '''
        fileNames, fileType = QFileDialog.getOpenFileNames(None, '可选择一个素材文件', os.getcwd(),
                                                           '图片文件(*.jpg *.png *.jpeg *.bin);;所有文件(*)')
        self.print_log((COLOR_RED % "已选择以下素材：\n") + str(fileNames))

        path_text = ""
        for fileName in fileNames:
            path_text = path_text + fileName + ";"
        self.form.choosePathEdit_2.setText(path_text)

    def writeBackgroud(self):
        """
        写入背景图片
        """
        try:
            os.makedirs(backgroud_base)
        except Exception as e:
            pass

        try:
            os.makedirs(backgroud_cache_dir)
        except Exception as e:
            pass

        param = self.get_output_param(self.form.choosePathEdit_2.text().strip())
        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            return False

        std_img_path = None
        param_v = dict()
        for ind in range(len(param["src_path"])):
            if param["format"][ind] in IMAGE_FORMAT or param["format"][ind] in ["bin", "BIN"]:
                param_v["format"] = param["format"][ind]
                param_v["src_path"] = param["src_path"][ind]
                param_v["width"] = param["width"]
                param_v["height"] = param["height"]
                name = os.path.basename(param["src_path"][ind]).split(".")[0]
                std_img_path = os.path.join(backgroud_cache_dir, name + ".jpg")

                break

        if "src_path" not in param_v.keys():
            self.print_log(COLOR_RED % "参数出错：只有选中的第一张照片才会生效")
            return False

        if param_v["format"] in IMAGE_FORMAT:
            # 缩放图片
            src_im: Image.Image = Image.open(param_v["src_path"])

            mode = "保持比例裁剪" if self.form.PictureModeRadioButton_0.isChecked() else "全尺寸缩放"
            if mode == "保持比例裁剪":
                rect = None
                new_width = None
                new_height = None
                # 裁剪
                if src_im.size[1] / int(param["height"]) > src_im.size[0] / int(param["width"]):
                    new_width = src_im.size[0]
                    new_height = new_width * (int(param["height"]) / int(param["width"]))
                    rect = (0, (src_im.size[1] - new_height) / 2, new_width, new_height)
                else:
                    new_height = src_im.size[1]
                    new_width = new_height * (int(param["width"]) / int(param["height"]))
                    rect = ((src_im.size[0] - new_width) / 2, 0, new_width, new_height)
                self.print_log((COLOR_RED % "宽x高 -> ") + str(src_im.size[0]) + "x" + str(src_im.size[1]))
                self.print_log((COLOR_RED % "新的 宽x高 -> ") + str(int(new_width)) + "x" + str(int(new_height)))
                src_im = src_im.crop(rect)
            suffix = os.path.basename(param_v["src_path"]).split(".")[1]
            if suffix == "png" or suffix == "PNG":
                # 由于PNG是RGBA四个通道 而jpg只有RGB三个通道
                src_im = src_im.convert('RGB')
            # 缩放
            new_im = src_im.resize((int(param_v["width"]), int(param_v["height"])), Image.BICUBIC)
            new_im.save(std_img_path)  # , format='JPEG', quality=95

            # 转化LVGL图片
            output_dir = os.path.join(gen_path, "Backgroud")
            cmd = ['lv_img_conv.py', std_img_path,
                   '-f', 'true_color',
                   #   '-f', 'true_color_alpha',
                   #   '-f', 'true_color_chroma',
                   '-cf', 'RGB565SWAP',
                   '-ff', 'BIN',
                   '-o', output_dir
                   ]
            conv = image_conv.lv_img_conv.Main(image_conv.lv_img_conv.parse_args(cmd[1:]))
            conv.convert()
            lvgl_filepath = os.path.join(output_dir, os.path.basename(std_img_path).split(".")[0] + ".bin")
        else:
            self.print_log(COLOR_RED % "此文件为图片Bin文件，无需图片转化，可直接写入")
            lvgl_filepath = param_v["src_path"]
        # 生成 SPIFFS 文件系统镜像
        # lvgl_filepath = os.path.join(gen_path, "backgroud.bin")
        # image_size = 0x50000
        # try:
        #     os.makedirs(backgroud_base)
        # except Exception as e:
        #     self.form.WriteWallpaperButton.setEnabled(True)
        #     pass

        # if not os.path.exists(backgroud_base):
        #     raise RuntimeError('given base directory %s does not exist' % backgroud_base)

        # with open(lvgl_filepath, 'wb') as image_file:
        #     # image_size = int(image_size, 0)
        #     page_size = 256
        #     block_size = 4096
        #     meta_len = 4
        #     obj_name_len = 48
        #     big_endian = False
        #     use_magic = True
        #     use_magic_len = True
        #     follow_symlinks = False

        #     # Based on typedefs under spiffs_config.h
        #     SPIFFS_OBJ_ID_LEN = 2  # spiffs_obj_id
        #     SPIFFS_SPAN_IX_LEN = 2  # spiffs_span_ix
        #     SPIFFS_PAGE_IX_LEN = 2  # spiffs_page_ix
        #     SPIFFS_BLOCK_IX_LEN = 2  # spiffs_block_ix

        #     spiffs_build_default = spiffsgen.SpiffsBuildConfig(page_size, SPIFFS_PAGE_IX_LEN,
        #                                             block_size, SPIFFS_BLOCK_IX_LEN, meta_len,
        #                                             obj_name_len, SPIFFS_OBJ_ID_LEN, SPIFFS_SPAN_IX_LEN,
        #                                             True, True, 'big' if big_endian else 'little',
        #                                             use_magic, use_magic_len)

        #     spiffs = spiffsgen.SpiffsFS(image_size, spiffs_build_default)

        #     for root, dirs, files in os.walk(backgroud_base, followlinks=follow_symlinks):
        #         for f in files:
        #             full_path = os.path.join(root, f)
        #             spiffs.create_file('/' + os.path.relpath(full_path, backgroud_base).replace('\\', '/'), full_path)

        #     image = spiffs.to_binary()
        #     image_file.write(image)

        # lvgl_filepath = os.path.join(gen_path, "Backgroud", "bridge_320x240.bin")

        print("lvgl_filepath = ", lvgl_filepath)

        # 50为预留值
        bg_all_size = 320 * 1024
        rate = int(os.path.getsize(lvgl_filepath) / bg_all_size * 100)
        self.print_log((COLOR_RED % "背景图可用的全容量为 ") + str(int(bg_all_size / 1024)) + " KB")
        self.print_log((COLOR_RED % "本次背景图占用全容量的 ") + str(rate) + "%")
        if os.path.getsize(lvgl_filepath) > bg_all_size:
            self.print_log(COLOR_RED % "异常终止：数据过大，请更换图片。")
            self.form.WriteWallpaperButton_2.setEnabled(True)
            return False

        # 烧录背景图
        select_com = self.getSafeCom()
        if select_com == None:
            return False

        chip_id = get_chip_id(select_com)

        try:
            cmd = ['SnailHeater_WinTool.py', '--port', select_com,
                   '--baud', baud_rate,
                   '--after', 'hard_reset',
                   'write_flash',
                   get_backgroup_addr_in_flash(chip_id), lvgl_filepath
                   ]
            time = int(os.path.getsize(lvgl_filepath)) * 10 / int(baud_rate) + 2
            self.print_log("正在烧入背景数据到主机，请等待（%ds）......" % time)
            esptool.main(cmd[1:])
            # self.hard_reset()  # 复位芯片
            self.print_log("成功烧入背景数据到主机")
        except Exception as err:
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            self.form.WriteWallpaperButton_2.setEnabled(True)
            print(err)

        self.form.WriteWallpaperButton_2.setEnabled(True)

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
        except Exception as err:
            self.form.WriteWallpaperButton.setEnabled(True)
            print(str(traceback.format_exc()))

        try:
            os.makedirs(wallpaper_path)
        except Exception as err:
            print(str(traceback.format_exc()))

        chip_id = get_chip_id(select_com)

        # self.print_log("正在获取存空间大小...")
        flash_size_max = 0
        flash_size_real = 0
        if chip_id == CHIP_ID_S2:
            flash_size_max = 1024 * 1024 * 16  # S2版本支持的最大Flash容量为16M
            flash_size_real, _ = get_flash_size(select_com)
        elif chip_id == CHIP_ID_S3:
            flash_size_max = 1024 * 1024 * 32
            # flash_size_real = 32 * 1024 * 1024
            flash_size_real, _ = get_flash_size(select_com)
        # self.print_log("已获取到存空间大小.")

        # 处理大容量的情况
        # 50为预留值
        flash_size_use = flash_size_real if flash_size_real <= flash_size_max else flash_size_max
        wallpaper_all_size = flash_size_use - (int(get_wallpaper_addr_in_flash(chip_id), 16) + 50)

        try:
            param = self.get_output_param(self.form.choosePathEdit.text().strip())
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

            rate = int(os.path.getsize(wallpaper_name) / wallpaper_all_size * 100)
            self.print_log((COLOR_RED % "本机储存容量为 ") + str(int(flash_size_real / 1024 / 1024)) + " MB")
            self.print_log((COLOR_RED % "壁纸可用的全容量为 ") + str(int(wallpaper_all_size / 1024)) + " KB")
            self.print_log((COLOR_RED % "本次壁纸占用全容量的 ") + str(rate) + "%")
            if os.path.getsize(wallpaper_name) > wallpaper_all_size:
                self.print_log(COLOR_RED % "异常终止：壁纸数据过大，请适当降低帧率或截取更短的时间。")
                self.form.WriteWallpaperButton.setEnabled(True)
                return False

        except Exception as err:
            print(str(traceback.format_exc()))
            return False

        try:
            cmd = ['SnailHeater_WinTool.py',
                   # '--no-stub',
                   #  '--compress',
                   '--port', select_com,
                   '--baud', baud_rate,
                   # '--before', 'no_reset',
                   '--after', 'hard_reset',
                   'write_flash',
                   get_wallpaper_addr_in_flash(chip_id), wallpaper_name,
                   #   '0x00FFF000', wallpaper_name
                   ]
            time_start = time.time()
            # 8984757 B 花费142.33s
            use_time = 2 + int(os.path.getsize(wallpaper_name)) / (1024 * 1024 * 2) * 32.75
            self.print_log("正在烧入壁纸数据到主机，请等待（%ds）......" % use_time)
            esptool.main(cmd[1:])
            # self.hard_reset()  # 复位芯片
            print("写入共花费了 ", (time.time() - time_start) * 1000, int(os.path.getsize(wallpaper_name)))
            self.print_log("成功烧入壁纸数据到主机")
        except Exception as e:
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            print(str(traceback.format_exc()))

        self.form.WriteWallpaperButton.setEnabled(True)

        return True

    def generateWallpaperBin(self):
        """
        生成壁纸的bin文件
        """
        self.print_log("正在生成壁纸文件......")
        param = self.get_output_param(self.form.choosePathEdit.text().strip())
        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            return None

        # path = os.path.dirname(param["dst_path"][0])
        # param["dst_path"][0] = f"{path}/rtttl.rtttl"
        wallpapers = param["dst_path"]
        verMark = 0x11  # 版本号
        total = 0  # 壁纸总数（1字节）
        fps = int(param["fps"])  # 帧率
        # fps = 22

        type = []  # 壁纸类型（1字节）jpg图片0 mjpeg视频1
        startAddr = []  # 4字节
        dataLen = []  # 4字节
        isLegal = []  # 处理过程中的标志位
        dataAddrOffset = 580  # 标记当前的数据可存地址
        for ind in range(len(wallpapers)):
            suffix = os.path.basename(wallpapers[ind]).split(".")[-1]
            if suffix == "mjpeg":
                type.append(TYPE_MJPEG)
            elif suffix == "rtttl":
                type.append(TYPE_RTTTL)
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
            os.remove(wallpaper_name)
        except Exception as err:
            print(str(traceback.format_exc()))

        with open(wallpaper_name, "wb") as fbin:
            binaryData = b'' + struct.pack('=' + "1H1B1B", *[verMark, total, fps])
            for ind in range(len(wallpapers)):
                if isLegal[ind] == False:
                    continue
                format = "1B1I1I"
                byteOrder = '='
                params = [type[ind], startAddr[ind], dataLen[ind]]
                binaryData = binaryData + struct.pack(byteOrder + format, *params)

            binaryData = binaryData + b'\x00' * (580 - len(binaryData))
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
        param = self.get_output_param(self.form.choosePathEdit.text().strip())

        if param == False:
            self.print_log((COLOR_RED % "请检查参数设置"))
            return False
        
        #  -c:v libx264 -crf 18
        #  -c copy
        cmd_time = 'ffmpeg -ss %s -to %s -i "%s" -c:v copy "%s"'  # 时间片截取 -i一定要放在时间参数后

        # （这个是为了跟cmd_to_mjpeg统一格式才加的参数）
        # fps 帧率过滤器
        # scale 缩放过滤器
        # crop 裁剪过滤器
        cmd_to_mjpeg = 'ffmpeg -y -i "%s" -vf "fps=%s,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -q:v %s "%s"'
        cmd_to_mjpeg_t = 'ffmpeg -y -i "%s" -vf "fps=%s,scale=%s:-1:flags=lanczos,crop=in_w:%s:0:(in_h-%s)/2" -q:v %s "%s"'

        rtttlConverter = rtttl.MP4ToRTTTLConverter()

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
                print(str(traceback.format_exc()))

            try:
                os.remove(param["dst_path"][ind])
            except Exception as err:
                print(str(traceback.format_exc()))

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
                if os.path.getsize(param["dst_path"][ind]) == 0:
                    self.print_log((COLOR_RED % "注：") + "原视频的宽高比大于屏幕的宽高比，正在尝试最大化裁剪")
                    # 最终输出的文件
                    trans_cmd = cmd_to_mjpeg_t
                    # 最后的转换命令
                    out_cmd = trans_cmd % (wallpaper_cache_path, param["fps"], param["width"],
                                           param["height"], param["height"], param["quality"][ind],
                                           param["dst_path"][ind])
                    print(out_cmd)
                    os.system(out_cmd)
            elif param["format"][ind] == "rtttl":
                try:
                    # 执行转换
                    rtttlConverter.convert(
                        mp4_path=param["src_path"][ind],
                        output_rtttl=param["dst_path"][ind],
                        rtttl_title="RTitle_"+str(ind)
                    )
                except Exception as e:
                    print(str(traceback.format_exc()))
                    print(f"转换失败：{str(e)}")
            elif param["format"][ind] in IMAGE_FORMAT:
                wallpaper_cache_path = param["src_path"][ind]
                src_im: Image.Image = Image.open(wallpaper_cache_path)

                mode = "保持比例裁剪" if self.form.PictureModeRadioButton_0.isChecked() else "全尺寸缩放"
                self.print_log(COLOR_RED % mode)
                if mode == "保持比例裁剪":
                    rect = None
                    new_width = None
                    new_height = None
                    # 裁剪
                    if src_im.size[1] / int(param["height"]) > src_im.size[0] / int(param["width"]):
                        new_width = src_im.size[0]
                        new_height = new_width * (int(param["height"]) / int(param["width"]))
                        rect = (0, (src_im.size[1] - new_height) / 2, new_width, new_height)
                    else:
                        new_height = src_im.size[1]
                        new_width = new_height * (int(param["width"]) / int(param["height"]))
                        rect = ((src_im.size[0] - new_width) / 2, 0, new_width, new_height)
                    self.print_log((COLOR_RED % "宽x高 -> ") + str(src_im.size[0]) + "x" + str(src_im.size[1]))
                    self.print_log((COLOR_RED % "新的 宽x高 -> ") + str(new_width) + "x" + str(new_height))
                    src_im = src_im.crop(rect)
                if suffix == "png" or suffix == "PNG":
                    # 由于PNG是RGBA四个通道 而jpg只有RGB三个通道
                    src_im = src_im.convert('RGB')
                # 缩放
                new_im = src_im.resize((int(param["width"]), int(param["height"])), Image.BICUBIC)
                new_im.save(param["dst_path"][ind])  # , format='JPEG', quality=95
                if os.path.getsize(param["dst_path"][ind]) >= waLLPAPER_JPG_MAX_SIZE:
                    # 超过固件的jpg 数据buffer的长度，再次压缩换得更小得图片
                    for qua in range(95, 5, -5):
                        new_im.save(param["dst_path"][ind], quality=qua)  # , format='JPEG', quality=95
                        if os.path.getsize(param["dst_path"][ind]) < waLLPAPER_JPG_MAX_SIZE:
                            break;

            try:
                if os.path.getsize(param["dst_path"][ind]) == 0:
                    self.print_log((COLOR_RED % "生成文件失败：") + param["src_path"][ind])
                    return False
            except Exception as err:
                print(str(traceback.format_exc()))
                return False

        self.print_log("转换完成")

        return True

    def get_output_param(self, fileNameText_t):
        """
        得到输出参数
        """
        resolutionW, resolutionH = self.form.resolutionComboBox.currentText().split(" ")[0].split("x")

        fileNameListText = fileNameText_t
        if fileNameListText == "":
            self.print_log(COLOR_RED % "未选择素材文件")
            return False
        oldFileNames = fileNameListText.split(";")[0:-1]
        fileNames = []
        outFileNames = []
        qualitys = []
        formats = []
        # 文件转化的创建输出目录
        for ind in range(len(oldFileNames)):
            fileName = oldFileNames[ind]
            fileNames.append(fileName)
            name_suffix = os.path.basename(fileName).split(".")
            if name_suffix[1] in MOVIE_FORMAT:
                # 音频数据文件
                fileNames.append(fileName) # 音频文件多出一个
                outFileNames.append(
                    os.path.join(wallpaper_cache_dir,
                                 name_suffix[0] + "_" + resolutionW + "x" + resolutionH + ".rtttl"))
                formats.append("rtttl")
                qualitys.append(self.form.qualityComboBox.currentText().strip())
                # 视频数据文件
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
            elif name_suffix[1] == "bin":
                outFileNames.append(
                    os.path.join(wallpaper_cache_dir, wallpaper_name))
                formats.append("bin")
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

        chip_id = get_chip_id(select_com)

        self.print_log("正在清空壁纸...")
        # esptool.py erase_region 0x20000 0x4000
        # esptool.py erase_flash
        # cmd = ['SnailHeater_WinTool.py', '--port', select_com,
        #        'erase_region', get_wallpaper_addr_in_flash(chip_id), '0x200000']
        # try:
        #     esptool.main(cmd[1:])
        # except Exception as e:
        #     self.print_log(COLOR_RED % ERR_UART_TEXT)
        #     pass

        cmd = ['SnailHeater_WinTool.py', '--port', select_com,
               '--baud', baud_rate,
               '--after', 'hard_reset',
               'write_flash',
               get_wallpaper_addr_in_flash(chip_id), default_wallpaper_clean
               ]
        try:
            esptool.main(cmd[1:])
            # self.hard_reset()  # 复位芯片
            self.print_log("成功清空壁纸.")
        except Exception as e:
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            pass

    def print_log(self, info):
        self.form.LogInfoTextBrowser.append(info + '\n')
        QApplication.processEvents()
        # self.form.LogInfoTextBrowser.

    def hard_reset(self):
        """
        重启芯片(控制USB-TLL的rst dst引脚)
        DST RST设置接口都是 true为低电平
        参考文档 https://blog.csdn.net/qq_37388044/article/details/111035944
        可以把以下代码修改到 esptool/targets/esp32s2.py 的 hard_reset()中
        :return:
        """

        select_com = self.getSafeCom()
        if select_com == None:
            return None
        self.ser = serial.Serial(select_com, baud_rate, timeout=10)

        self.ser.setDTR(False)  # IO0=HIGH
        self.ser.setRTS(True)  # EN=LOW, chip in reset
        time.sleep(0.05)
        self.ser.setRTS(False)  # EN=HIGH, chip out of reset
        # 0.5 needed for ESP32 rev0 and rev1
        time.sleep(0.05)  # 0.5 / 0.05

        self.release_serial()

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
        #                                COLOR_RED % "开始前一定要拔掉220V电源线！",
        #                                QMessageBox.Yes | QMessageBox.Cancel,
        #                                QMessageBox.Cancel)

        # 创建自定义消息框
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示",
                                COLOR_RED % "开始前一定要拔掉220V电源线！")
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
                                COLOR_RED % "开始前一定要拔掉220V电源线！")
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
                                COLOR_RED % "开始前一定要拔掉220V电源线！")
        # 添加自定义按钮
        do = self.mbox.addButton('确定', QMessageBox.YesRole)
        cancle = self.mbox.addButton('取消', QMessageBox.NoRole)
        # 设置消息框中内容前面的图标
        self.mbox.setIcon(2)
        do.clicked.connect(self.cleanWallpaper)
        self.mbox.show()

    def WriteBgButton_show_message(self):
        """
        警告拔掉AC220V消息框
        :return: None
        """

        # 创建自定义消息框
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示",
                                COLOR_RED % "开始前一定要拔掉220V电源线！")
        # 添加自定义按钮
        do = self.mbox.addButton('确定', QMessageBox.YesRole)
        cancle = self.mbox.addButton('取消', QMessageBox.NoRole)
        # 设置消息框中内容前面的图标
        self.mbox.setIcon(2)
        do.clicked.connect(self.writeBackgroud)
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
