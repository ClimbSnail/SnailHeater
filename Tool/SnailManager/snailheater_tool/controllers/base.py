# -*- coding: utf-8 -*-
"""两个产品控制器共用的界面控制基础实现。"""

import datetime
import os
import re
import shutil
import time
import traceback
from choose_ver import QtCore, QtWidgets

QWidget = QtWidgets.QWidget
QApplication = QtWidgets.QApplication
QMessageBox = QtWidgets.QMessageBox
QFileDialog = QtWidgets.QFileDialog
Qt = QtCore.Qt
import common
import massagehead as mh
from ..activation_service import ActivationService
from ..device_service import DeviceService
from ..firmware_service import FirmwareService
from ..media_service import MediaOptions, MediaService
from ..models import DownloadMode, FlashEntry
from ..paths import RuntimePaths
from ..profiles import CHIP_ID_KNOWN, CHIP_ID_S2, CHIP_ID_S3
from ..worker import FirmwareWorker

COLOR_RED = '<span style=" color: #ff0000;">%s</span>'
ERR_UART_TEXT = "错误：通讯异常。检查设备，或拔插TypeC数据线重试！"


class BaseToolController(object):

    def __init__(self, profile, ui_class):
        """
        初始化对象状态和运行所需的依赖。

        Args:
            profile: 产品配置对象。
            ui_class: 界面类。

        """
        self.profile = profile
        self.ui_class = ui_class
        self.paths = RuntimePaths.discover()
        self.config = profile.load_config(self.paths)
        self.state = profile.initial_state(self.paths)
        self.firmware_service = FirmwareService(self.config, profile, self.paths)
        self.device_service = DeviceService(self.config.info_baud_rate, self.config.baud_rate)
        self.activation_service = ActivationService(self.config, self.paths)
        self.media_service = MediaService(self.paths, profile)
        self.media_service.ensure_directories()
        self.default_wallpaper_clean = profile.wallpaper_clean_path(self.paths)
        self.progress_bar_time_cnt = 0
        self.progress_bar_timer = QtCore.QTimer()
        self.progress_bar_timer.timeout.connect(self.schedule_display_time)
        self.download_thread = None

    def run(self):
        """创建并显示当前产品的主窗口。"""
        self.win_main = QWidget()
        self.form = self.ui_class()
        self.form.setupUi(self.win_main)
        translate = QtCore.QCoreApplication.translate
        version_text = self.firmware_service.get_tool_version_text(self.state)
        self.win_main.setWindowTitle(
            translate(
                "SanilHeaterTool", self.config.tool_name + common.TOOL_VERSION + " " + version_text
            )
        )
        selectable = getattr(Qt, "TextSelectableByMouse", None)
        if selectable is None and hasattr(Qt, "TextInteractionFlag"):
            selectable = Qt.TextInteractionFlag.TextSelectableByMouse
        for widget in (
            self.form.LinkInfolabel,
            self.form.UpdateLogLinkInfolabel,
            self.form.QQInfolabel,
            self.form.QQInfolabel_2,
            self.form.sourceInfolabel,
        ):
            widget.setTextInteractionFlags(selectable)
        self._bind_signals()
        # Qt 版不再提供独立的 MP4 转 RTTTL 按钮，保留对象仅用于兼容旧 UI 定义。
        self.form.convertRtttlButton.setVisible(False)
        self._configure_common_ui(translate)
        self.configure_product_ui()
        self._center_window()
        self.win_main.show()
        return self.win_main

    def _center_window(self):
        """将 Qt 主窗口放到主显示器可用区域的中心，兼容 PyQt5 与 PySide2。"""
        try:
            primary_screen = getattr(QApplication, "primaryScreen", None)
            screen = primary_screen() if primary_screen else None
            available_geometry = screen.availableGeometry() if screen is not None else None
            if available_geometry is None:
                desktop = QApplication.desktop()
                available_geometry = desktop.availableGeometry(self.win_main)
            window_geometry = self.win_main.frameGeometry()
            window_geometry.moveCenter(available_geometry.center())
            self.win_main.move(window_geometry.topLeft())
        except Exception:
            # 屏幕信息不可用时交给系统决定初始位置，不能影响工具启动。
            pass

    def _bind_signals(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        bindings = (
            (self.form.ComComboBox.clicked, self.scan_com),
            (self.form.FirmwareComboBox.clicked, self.scan_firmware),
            (self.form.QueryPushButton.clicked, self.query_button_click),
            (self.form.chooseWPButton.clicked, self.chooseWpFile),
            (self.form.convertRtttlButton.clicked, self.convert_mp4_to_rtttl),
            (self.form.chooseFileButton.clicked, self.chooseBgFile),
            (self.form.ActivatePushButton.clicked, self.act_button_click),
            (self.form.UpdatePushButton.clicked, self.UpdatePushButton_show_message),
            (self.form.WriteWallpaperButton.clicked, self.WriteWallpaperButton_show_message),
            (self.form.reflushWallpaperButton.clicked, self.reflushWallpaperButton_show_message),
            (self.form.WriteWallpaperButton_2.clicked, self.WriteBgButton_show_message),
            (self.form.CanclePushButton.clicked, self.cancle_button_click),
            (self.form.uiReadColorBnt.clicked, self.query_color_button_click),
            (self.form.uiWriteColorBnt.clicked, self.write_color_button_click),
        )
        for signal, callback in bindings:
            signal.connect(callback)

    def _configure_common_ui(self, translate):
        """
        执行该函数的主要处理逻辑。

        Args:
            translate: 界面翻译函数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        tips = {
            self.form.QueryPushButton: "获取机器码(SN)",
            self.form.QueryPushButton_2: "暂未开通",
            self.form.ActivatePushButton: "填入SN，点此激活",
            self.form.UpdateModeMethodRadioButton: "保留用户的设置信息，只在固件上做更新",
            self.form.ClearModeMethodRadioButton: "清空芯片内可清空的信息后重新刷写",
            self.form.chooseWPButton: "选择素材文件的路径（可选多项）",
            self.form.chooseFileButton: "选择图片文件的路径",
            self.form.WriteWallpaperButton: "转换素材并刷写到设备",
            self.form.WriteWallpaperButton_2: "转换素材并刷写到设备",
            self.form.PictureModeRadioButton_0: "保持分辨率比例，在中心区域最大面积裁剪",
            self.form.PictureModeRadioButton_1: "全图范围内按最佳比例缩放",
            self.form.reflushWallpaperButton: "清除设备上的壁纸信息",
            self.form.timeLabel_0: "只允许填写正整数，全量截取请设置 0 0",
            self.form.qualitylabel: "数字越小，质量越高",
            self.form.qualityComboBox: "数字越小，质量越高",
            self.form.fpslabel: "性能一定时，帧率越大越卡顿",
            self.form.fpsEdit: "性能一定时，帧率越大越卡顿",
            self.form.startTimeEdit: "需要截取时间范围时设置",
            self.form.endTimeEdit: "需要截取时间范围时设置",
        }
        self.form.Infolabel.setText(translate("SanilHeaterTool", self.config.info_url_0.name + "："))
        self.form.updateLoglabel.setText(translate("SanilHeaterTool", self.config.info_url_1.name + "："))
        label_alignment = Qt.AlignRight | Qt.AlignVCenter
        self.form.Infolabel.setAlignment(label_alignment)
        self.form.updateLoglabel.setAlignment(label_alignment)
        for widget, tip in tips.items():
            widget.setToolTip(tip)
        for widget in (
            self.form.uiForwardROColorlabel,
            self.form.uiForwardClickedColorlabel,
            self.form.uiForwardROColorLineEdit,
            self.form.uiForwardClickedColorLineEdit,
        ):
            widget.setToolTip("RGB颜色格式为(0x)7AFE89")
        qq_pair = self.config.qq_pair
        self.form.QQInfolabel.setText(translate("SanilHeaterTool", qq_pair[0]))
        self.form.QQInfolabel_2.setText(translate("SanilHeaterTool", qq_pair[1]))
        # 配置只保存原始文本或 URL；富文本链接样式由各 Qt UI 类统一负责。
        self.form.set_info_urls(self.config.info_url_0, self.config.info_url_1)
        # 两行名称标签与右侧内容保持同一右边界，并按配置文本动态向左扩展，避免长名称被截断。
        for label, value_label in (
            (self.form.Infolabel, self.form.LinkInfolabel),
            (self.form.updateLoglabel, self.form.UpdateLogLinkInfolabel),
        ):
            metrics = label.fontMetrics()
            measure = getattr(metrics, "horizontalAdvance", None) or getattr(metrics, "width")
            text_width = int(measure(label.text())) + 6
            right_edge = value_label.geometry().x() - 4
            left_limit = 180
            width = min(max(label.width(), text_width), max(1, right_edge - left_limit))
            label.setGeometry(right_edge - width, label.geometry().y(), width, label.height())
        self.form.resolutionComboBox.clear()
        self.form.resolutionComboBox.addItems(list(self.profile.resolutions))
        self.form.qualityComboBox.clear()
        self.form.qualityComboBox.addItems([str(number) for number in range(1, 20)])
        self.form.qualityComboBox.setCurrentText("5")
        self.form.fpsEdit.setText("20")
        self.form.startTimeEdit.setText("0")
        self.form.endTimeEdit.setText("0")
        self.form.VerInfolabel.setStyleSheet("color: red")
        self.form.sourceInfolabel.setStyleSheet("color: red")
        self.form.UICLineEdit.setReadOnly(True)

    def configure_product_ui(self):
        """由两个产品控制器设置差异化界面。"""

    def scan_com(self):
        self.print_log("搜索串口号...")
        self.form.ComComboBox.clear()
        self.form.ComComboBox.addItems(self.device_service.display_ports())

    def scan_firmware(self):
        """
        扫描可用的固件文件。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.get_firmware_version()
        self.print_log("搜索固件目录下的可用固件...")
        self.form.FirmwareComboBox.clear()
        try:
            names = self.firmware_service.scan_firmware()
        except (OSError, re.error) as error:
            self.print_log(COLOR_RED % f"固件扫描失败：{error}")
            names = []
        self.form.FirmwareComboBox.addItems(names or ["未找到固件"])

    def getSafeCom(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        selected = self.device_service.validate_port(self.form.ComComboBox.currentText())
        if selected:
            return selected
        ports = self.device_service.list_ports()
        if self.state.current_chip_id == CHIP_ID_S3 and len(ports) == 1:
            return ports[0][0]
        self.print_log(
            COLOR_RED % "错误提示："
            + "无法检测到指定串口设备，先确认 CH340 驱动是否正常或尝试 typec 调换方向。\n"
        )
        return None

    def read_coredump(self):
        """读取设备 coredump；当前按钮默认未绑定。"""
        port = self.getSafeCom()
        if not port:
            return None
        try:
            machine_code = self.get_machine_code() or "UNKNOWN"
            now = datetime.datetime.now().strftime("%Y%m%d_%H_%M_%S")
            output = str(self.paths.coredump_dir / f"SH_{now}_{machine_code}.coredump")
            self.print_log("正在获取异常信息...")
            self.firmware_service.read_flash(
                port, "0x1D0000", "0x10000", output, self.print_log
            )
            self.print_log(COLOR_RED % ("异常报告已生成：" + output))
            return output
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"无法获取异常信息：{error}")
            return None

    def act_button_click(self):
        """
        处理设备激活操作。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.print_log("正在激活设备...")
        port = self.getSafeCom()
        if not port:
            self.print_log(COLOR_RED % "激活操作异常，激活中止...")
            return None
        try:
            success = self.device_service.set_value(
                port, mh.VT.VALUE_TYPE_SN, self.form.SNLineEdit.text().strip()
            )
            self.print_log("激活成功" if success else COLOR_RED % "激活失败")
            return success
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            return False

    def write_color_button_click(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.print_log("正在写入UI前台颜色...")
        normal = self.form.uiForwardROColorLineEdit.text().strip().upper()
        clicked = self.form.uiForwardClickedColorLineEdit.text().strip().upper()
        if len(normal) != 6 or len(clicked) != 6:
            self.print_log(COLOR_RED % "RGB格式错误")
            return None
        port = self.getSafeCom()
        if not port:
            self.print_log(COLOR_RED % "写入UI前台颜色异常...")
            return None
        try:
            success = self.device_service.set_value(
                port, mh.VT.VALUE_TYPE_FORWARD_COLOR, normal + " " + clicked
            )
            self.print_log("写入UI前台颜色成功" if success else COLOR_RED % "写入UI前台颜色失败")
            return success
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            return False

    def auto_active(self):
        """
        自动激活
        """
        self.print_log(COLOR_RED % "执行自动激活程序")
        self.hard_reset()
        time.sleep(2)
        if self.query_button_click() == False:
            return False
        try:
            self.act_button_click()
        except Exception as err:
            print(str(traceback.format_exc()))

    def query_button_click(self):
        """查询机器码，并通过激活服务查询对应 SN。"""
        self.print_log("获取机器码（用户识别码）...")
        machine_code = self.get_machine_code()
        self.form.UICLineEdit.setText(machine_code or "")
        if not machine_code:
            self.print_log(COLOR_RED % "获取机器码异常")
            return False
        self.print_log("\n获取本地激活码（SN）...")
        try:
            self.print_log("联网查询激活码...")
            info = self.activation_service.query(machine_code)
            if not info.sn:
                self.print_log(COLOR_RED % "未查询到激活码")
                return False
            self.form.SNLineEdit.setText(info.sn)
            self.form.sourceInfolabel.setText("本机器来源 " + info.registrant)
            self.activation_service.append_cache(machine_code, info.sn)
            return True
        except Exception:
            print(traceback.format_exc())
            self.print_log("联网异常")
            return False

    def query_color_button_click(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.print_log("获取UI前台颜色...")
        port = self.getSafeCom()
        if not port:
            return None
        try:
            colors = self.device_service.query_value(
                port, mh.VT.VALUE_TYPE_FORWARD_COLOR, "VALUE_TYPE_FORWARD_COLOR = (\\S+ \\S+)"
            ).split()
            if len(colors) != 2:
                raise ValueError("设备未返回有效颜色")
            self.form.uiForwardROColorLineEdit.setText(format(int(colors[0], 0), "06X"))
            self.form.uiForwardClickedColorLineEdit.setText(format(int(colors[1], 0), "06X"))
            self.print_log("获取UI前台颜色成功")
            return colors
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % "获取UI前台颜色失败")
            return None

    def reset_ui_button(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.progress_bar_time_cnt = 0
        self.form.UpdatePushButton.setEnabled(True)
        self.form.UpdateModeMethodRadioButton.setEnabled(True)
        self.form.ClearModeMethodRadioButton.setEnabled(True)
        self.form.CanclePushButton.setEnabled(True)

    def update_button_click(self):
        """校验界面输入并启动公共固件 Worker。"""
        self.form.UpdateModeMethodRadioButton.setEnabled(False)
        self.form.ClearModeMethodRadioButton.setEnabled(False)
        self.form.UpdatePushButton.setEnabled(False)
        try:
            port = self.getSafeCom()
            if not port:
                self.reset_ui_button()
                return False
            firmware_name = self.form.FirmwareComboBox.currentText().strip()
            if not firmware_name or firmware_name == "未找到固件":
                self.print_log(COLOR_RED % "未选择有效固件")
                self.reset_ui_button()
                return False
            firmware_path = self.firmware_service.firmware_path(firmware_name)
            if not os.path.isfile(firmware_path):
                self.print_log(COLOR_RED % f"固件文件不存在：{firmware_path}")
                self.reset_ui_button()
                return False
            mode = (
                DownloadMode.UPDATE
                if self.form.UpdateModeMethodRadioButton.isChecked()
                else DownloadMode.CLEAR
            )
            self.state.download_mode = mode
            wallpaper, background = self.profile.choose_default_media(firmware_name, self.paths)
            self.state.default_wallpaper, self.state.default_background = (wallpaper, background)
            if not os.path.isfile(self.state.default_wallpaper):
                self.state.default_wallpaper = self.default_wallpaper_clean
            if not self.profile.is_el and self.state.support_versions:
                current = self.profile.firmware_version(firmware_name)
                start, end = self.state.support_versions
                if common.getVerValue(start) > common.getVerValue(current) or common.getVerValue(
                    end
                ) < common.getVerValue(current):
                    self.print_log(COLOR_RED % "当前版本管理工具不支持该固件")
                    self.reset_ui_button()
                    return False
            self.state.auto_activate = self.profile.auto_activation and "Pro" not in firmware_name
            self.print_log("串口号：" + COLOR_RED % port)
            self.print_log("固件文件：" + COLOR_RED % firmware_name)
            self.print_log("刷机模式：" + COLOR_RED % mode.value)
            self.progress_bar_timer.start(1000)
            self.progress_bar_time_cnt = 1
            self.download_thread = FirmwareWorker(
                self.firmware_service,
                port=port,
                firmware_path=firmware_path,
                mode=mode,
                state=self.state,
            )
            self.download_thread.print_signal.connect(self.print_log)
            self.download_thread.ret_finish.connect(self.down_action_finish)
            self.download_thread.start()
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"启动刷机失败：{error}")
            self.reset_ui_button()
            return False

    def down_action_finish(self, is_ok):
        """
        执行该函数的主要处理逻辑。

        Args:
            is_ok: 操作是否成功。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.progress_bar_timer.stop()
        if is_ok:
            self.form.progressBar.setValue(100)
        should_activate = (
            is_ok
            and self.state.download_mode == DownloadMode.CLEAR
            and self.state.auto_activate
            and self.profile.auto_activation
        )
        self.reset_ui_button()
        if should_activate:
            QtCore.QTimer.singleShot(4000, self._run_auto_activation)
        else:
            self.state.current_chip_id = CHIP_ID_KNOWN
            if (
                is_ok
                and self.state.download_mode == DownloadMode.CLEAR
                and self.profile.auto_activation
            ):
                self.print_log(COLOR_RED % "请通电30秒后手动查询激活码并激活。")

    def _run_auto_activation(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            self.auto_active()
        finally:
            self.state.current_chip_id = CHIP_ID_KNOWN

    def cancle_button_click(self):
        """按 QT_Tool 的经典行为立即终止正在执行的刷机 QThread。"""
        self.print_log("手动停止更新固件...")
        if self.download_thread is not None:
            try:
                if self.download_thread.isRunning():
                    self.download_thread.terminate()
                    self.download_thread.wait()
            except Exception:
                print(traceback.format_exc())
        self.reset_ui_button()
        self.form.progressBar.setValue(0)

    def get_firmware_version(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            self.print_log("联网查询最新固件版本...")
            value = self.firmware_service.get_latest_firmware_text()
            if not value:
                raise ValueError("未识别到固件版本信息")
            self.form.VerInfolabel.setText("最新固件版本 " + str(value))
            self.print_log("最新固件版本 " + COLOR_RED % str(value))
            return value
        except Exception:
            self.form.VerInfolabel.setText("无法获取最新固件版本")
            self.print_log(COLOR_RED % "无法获取最新固件版本")
            return None

    def get_machine_code(self):
        """
        从设备读取机器码。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        port = self.getSafeCom()
        if not port:
            return None
        try:
            value = self.device_service.query_value(
                port, mh.VT.VALUE_TYPE_MC, "VALUE_TYPE[_MC]* = (\\d*)"
            )
            if not value:
                raise ValueError("机器码为空")
            self.print_log("机器码查询成功")
            return value
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % "机器码查询失败")
            return None

    def get_sn(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        port = self.getSafeCom()
        if not port:
            return ""
        try:
            value = self.device_service.query_value(
                port, mh.VT.VALUE_TYPE_SN, "VALUE_TYPE[_SN]* = (\\S*)"
            )
            self.print_log("本地激活码查询成功" if value else COLOR_RED % "本地激活码查询失败")
            return value
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % "本地激活码查询失败")
            return ""

    def get_machine_software_ver(self, param=None):
        """
        执行该函数的主要处理逻辑。

        Args:
            param: 查询参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        port = self.getSafeCom()
        if not port:
            return ""
        try:
            value = self.device_service.query_value(
                port, mh.VT.VALUE_TYPE_SH_SOFTWARE_VER, "VALUE_TYPE_SH_SOFTWARE_VER = (\\S*)"
            )
            self.print_log(
                "目前焊台软件版本为：" + value if value else COLOR_RED % "查询固件版本失败"
            )
            return value
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % "查询固件版本失败")
            return ""

    def picture_directory(self):
        """Return the platform's user-visible Pictures folder for media pickers."""
        pictures = QtCore.QStandardPaths.writableLocation(QtCore.QStandardPaths.PicturesLocation)
        return pictures or os.path.expanduser("~")

    def media_dialog_directory(self):
        """Return the most recently used media directory for this session."""
        return getattr(self, "_last_media_directory", "") or self.picture_directory()

    def remember_media_directory(self, filename):
        """Keep the selected media file's directory for the next picker invocation."""
        directory = os.path.dirname(filename)
        if directory:
            self._last_media_directory = directory

    def chooseWpFile(self):
        """选择一个或多个壁纸素材；视频、图片、RTTTL 与已打包文件共用此入口。"""
        file_names, _ = QFileDialog.getOpenFileNames(
            None, "可选择多个素材文件", self.media_dialog_directory(), "素材文件 (*.*)"
        )
        if file_names:
            self.remember_media_directory(file_names[0])
            text = ";".join(file_names) + ";"
            self.form.choosePathEdit.setText(text)
            self.print_log("已选择素材：" + text)

    def convert_mp4_to_rtttl(self):
        """从 Qt 界面选择一个 MP4，并按旧版算法生成独立的 RTTTL 素材。"""
        if not self.profile.supports_rtttl:
            self.print_log(COLOR_RED % "当前产品不支持 RTTTL 素材")
            return False
        source, _ = QFileDialog.getOpenFileName(
            None, "选择要转换的 MP4 视频", str(self.paths.root), "MP4 视频 (*.mp4)"
        )
        if not source:
            return False
        self.form.convertRtttlButton.setEnabled(False)
        try:
            output = self.media_service.convert_mp4_to_rtttl(source, self.print_log)
            self.print_log("RTTTL 素材已生成：" + str(output))
            self.print_log("请将该 RTTTL 文件作为壁纸素材添加后，再生成或写入壁纸包。")
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"RTTTL 转换失败：{error}")
            return False
        finally:
            self.form.convertRtttlButton.setEnabled(True)

    def chooseBgFile(self):
        """选择单个背景素材；背景分区只接受图片或已生成的 BIN 文件。"""
        file_names, _ = QFileDialog.getOpenFileNames(
            None, "可选择一个素材文件", self.media_dialog_directory(), "图片文件 (*.jpg *.jpeg *.png *.bin)"
        )
        if file_names:
            self.remember_media_directory(file_names[0])
            text = file_names[0] + ";"
            self.form.choosePathEdit_2.setText(text)
            self.print_log("已选择背景素材：" + text)

    def writeBackgroud(self):
        """生成背景文件并通过固件服务写入。"""
        self.form.WriteWallpaperButton_2.setEnabled(False)
        try:
            params = self.get_output_param(self.form.choosePathEdit_2.text().strip())
            if not params:
                return False
            crop_to_fill = self.form.PictureModeRadioButton_0.isChecked()
            background = self.media_service.prepare_background(params, crop_to_fill, self.print_log)
            capacity = 320 * 1024
            rate = self.media_service.validate_capacity(background, capacity)
            self.print_log(COLOR_RED % f"背景图可用容量为 {capacity // 1024} KB")
            self.print_log(COLOR_RED % f"本次背景图占用 {rate}%")
            port = self.getSafeCom()
            if not port:
                return False
            chip_id = self.firmware_service.get_chip_id(port, self.print_log)
            self.print_log("正在烧入背景数据到主机，请等待......")
            self.firmware_service.write_entries(
                port,
                [FlashEntry(self.profile.background_address(chip_id), str(background))],
                log=self.print_log,
            )
            self.print_log("成功烧入背景数据到主机")
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"背景写入失败：{error}")
            return False
        finally:
            self.form.WriteWallpaperButton_2.setEnabled(True)

    def writeWallpaper(self):
        """转换、打包并写入壁纸。"""
        port = self.getSafeCom()
        if not port:
            return False
        self.form.WriteWallpaperButton.setEnabled(False)
        try:
            chip_id = self.firmware_service.get_chip_id(port, self.print_log)
            flash_size_real, _ = self.firmware_service.get_flash_size(
                port, self.print_log
            )
            flash_size_max = 16 * 1024 * 1024 if chip_id == CHIP_ID_S2 else 32 * 1024 * 1024
            if not flash_size_real or chip_id not in {CHIP_ID_S2, CHIP_ID_S3}:
                raise RuntimeError("无法识别芯片或 Flash 容量")
            capacity = min(flash_size_real, flash_size_max) - (
                int(self.profile.wallpaper_address(chip_id), 16) + 50
            )
            params = self.get_output_param(self.form.choosePathEdit.text().strip())
            if not params:
                return False
            if params["format"][0] == "lsw":
                self.print_log(COLOR_RED % "正在使用已打包好的壁纸文件")
                shutil.copy(params["src_path"][0], self.paths.wallpaper_file)
            else:
                self.media_service.convert(
                    params, self.form.PictureModeRadioButton_0.isChecked(), self.print_log
                )
                self.media_service.pack_wallpaper(params, self.print_log)
            rate = self.media_service.validate_capacity(self.paths.wallpaper_file, capacity)
            self.print_log(COLOR_RED % f"本机储存容量为 {flash_size_real // 1024 // 1024} MB")
            self.print_log(COLOR_RED % f"壁纸可用容量为 {capacity // 1024} KB")
            self.print_log(COLOR_RED % f"本次壁纸占用 {rate}%")
            self.print_log("正在烧入壁纸数据到主机，请等待......")
            self.firmware_service.write_entries(
                port,
                [
                    FlashEntry(
                        self.profile.wallpaper_address(chip_id),
                        str(self.paths.wallpaper_file),
                    )
                ],
                log=self.print_log,
            )
            self.print_log("成功烧入壁纸数据到主机")
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"壁纸写入失败：{error}")
            return False
        finally:
            self.form.WriteWallpaperButton.setEnabled(True)

    def generateWallpaperBin(self):
        """调用素材服务生成统一的 LSW 壁纸包。"""
        self.print_log("正在生成壁纸文件......")
        params = self.get_output_param(self.form.choosePathEdit.text().strip())
        if not params:
            self.print_log(COLOR_RED % "请检查参数设置")
            return None
        try:
            result = self.media_service.pack_wallpaper(params, self.print_log)
            self.print_log("壁纸文件生成成功：" + str(result))
            return result
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"壁纸文件生成失败：{error}")
            return None

    def trans_format(self):
        """调用与 Qt 解耦的素材服务完成格式转换。"""
        self.print_log(COLOR_RED % "正在转换（注：视频较大时处理时间会较长）")
        params = self.get_output_param(self.form.choosePathEdit.text().strip())
        if not params:
            self.print_log(COLOR_RED % "请检查参数设置")
            return False
        try:
            crop_to_fill = self.form.PictureModeRadioButton_0.isChecked()
            self.media_service.convert(params, crop_to_fill, self.print_log)
            self.print_log("转换完成")
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"素材转换失败：{error}")
            return False

    def get_output_param(self, file_name_text):
        """
        执行该函数的主要处理逻辑。

        Args:
            file_name_text: 文件名或路径文本。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        try:
            options = MediaOptions(
                resolution=self.form.resolutionComboBox.currentText(),
                start_time=int(self.form.startTimeEdit.text().strip()),
                end_time=int(self.form.endTimeEdit.text().strip()),
                fps=self.form.fpsEdit.text().strip(),
                quality=self.form.qualityComboBox.currentText().strip(),
            )
            return self.media_service.build_output_params(file_name_text, options)
        except (ValueError, OSError) as error:
            self.print_log(COLOR_RED % str(error))
            return False

    def cleanWallpaper(self):
        """
        执行该函数的主要处理逻辑。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        port = self.getSafeCom()
        if not port:
            return None
        try:
            chip_id = self.firmware_service.get_chip_id(port, self.print_log)
            self.print_log("正在清空壁纸...")
            self.firmware_service.write_entries(
                port,
                [
                    FlashEntry(
                        self.profile.wallpaper_address(chip_id),
                        self.default_wallpaper_clean,
                    )
                ],
                log=self.print_log,
            )
            self.print_log("成功清空壁纸。")
            return True
        except Exception as error:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % f"清空壁纸失败：{error}")
            return False

    def print_log(self, info):
        self.form.LogInfoTextBrowser.append(str(info))
        QApplication.processEvents()

    def hard_reset(self):
        """
        通过串口执行设备硬复位。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        port = self.getSafeCom()
        if not port:
            return None
        try:
            self.device_service.hard_reset(port)
        except Exception:
            print(traceback.format_exc())
            self.print_log(COLOR_RED % ERR_UART_TEXT)
            return None

    def schedule_display_time(self):
        if self.progress_bar_time_cnt > 0 and self.progress_bar_time_cnt < 99:
            self.progress_bar_time_cnt += 1
        self.form.progressBar.setValue(self.progress_bar_time_cnt)

    def _show_confirmation(self, message, callback):
        """
        执行该函数的主要处理逻辑。

        Args:
            message: 提示消息。
            callback: 结果回调函数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.mbox = QMessageBox(QMessageBox.Warning, "重要提示", COLOR_RED % message)
        confirm = self.mbox.addButton("确定", QMessageBox.YesRole)
        self.mbox.addButton("取消", QMessageBox.NoRole)
        self.mbox.setIcon(QMessageBox.Warning)
        confirm.clicked.connect(callback)
        self.mbox.show()

    def UpdatePushButton_show_message(self):
        self._show_confirmation(self.power_warning_message(), self.update_button_click)

    def power_warning_message(self):
        return "开始前一定要拔掉220V电源线！"

    def WriteWallpaperButton_show_message(self):
        self._show_confirmation(self.power_warning_message(), self.writeWallpaper)

    def reflushWallpaperButton_show_message(self):
        self._show_confirmation(self.power_warning_message(), self.cleanWallpaper)

    def WriteBgButton_show_message(self):
        self._show_confirmation(self.power_warning_message(), self.writeBackgroud)
