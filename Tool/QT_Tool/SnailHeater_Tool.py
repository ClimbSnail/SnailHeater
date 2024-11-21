# encoding: utf-8

# 打包脚本
# pyinstaller --icon ./images/SnailHeater_256.ico -F SnailHeater_Tool.py

# pip install pyserial -i https://mirrors.aliyun.com/pypi/simple/
import sys
import os
import time
import serial  # pip install pyserial
import serial.tools.list_ports
# from esptoolpy import espefuse
import esptool
# 需要修改esptool源码loader.py中得一个文件路径
# STUBS_DIR = os.path.join(os.path.dirname(__file__), "targets", "stub_flasher")
# 修改为如下
# STUBS_DIR = os.path.join(os.getcwd(), "stub_flasher")

import requests
import yaml  # pip install pyyaml
import io
import re
import traceback
import massagehead as mh

TOOL_VERSION = "v2.8.1 Lite"

cur_dir = os.getcwd()  # 当前目录
# 生成的文件目录
gen_path = os.path.join(cur_dir, "Generate")
# 背景图片
backgroud_base = os.path.join(gen_path, "Backgroud")
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

CHIP_ID_KNOWN = ""
CHIP_ID_S2 = "S2"
CHIP_ID_S3 = "S3"

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


def get_wallpaper_addr_in_flash(chip_id):
    # 背景图
    if chip_id == CHIP_ID_S2:
        return '0x00200000'
    elif chip_id == CHIP_ID_S3:
        return '0x004D0000'


def get_backgroup_addr_in_flash(chip_id):
    # 壁纸文件
    if chip_id == CHIP_ID_S2:
        return '0x180000'
    elif chip_id == CHIP_ID_S3:
        return '0x480000'


def act_button_click(com):
    global sn
    print("正在激活设备...")

    select_com = com
    if select_com == None:
        print("激活操作异常，激活中止...")
        return None

    ser = serial.Serial(select_com, info_baud_rate, timeout=10)

    act_ret = False

    # 判断是否打开成功
    if ser.is_open:

        # 循环接收数据，此为死循环，可用线程实现
        send_data = mh.SettingMsg()
        send_data.action_type = mh.AT.AT_SETTING_SET
        # send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
        key = ""
        send_data.key = bytes(key, encoding='utf8')
        send_data.type = mh.VT.VALUE_TYPE_SN.to_bytes(1, byteorder='little', signed=True)
        print(send_data.type)
        value = sn
        print(value)
        send_data.value = bytes(value, encoding='utf8')
        print(send_data.encode('!'))
        ser.write(send_data.encode('!'))

        time.sleep(1)
        if ser.in_waiting:
            try:
                STRGLO = ser.read(ser.in_waiting)
                print("\nSTRGLO = ", STRGLO)
                match_info = re.findall(r"Success", STRGLO.decode("utf8"))
                if match_info != []:
                    act_ret = True
            except Exception as err:
                print(str(traceback.format_exc()))

        if act_ret == True:
            print("激活成功")
        else:
            print("激活失败")

    ser.close()  # 关闭串口
    del ser


def query_button_click(com):
    """
    获取用户识别码 显示在用户识别码的信息框里
    获取激活码 显示在激活码的信息框里
    :return: None
    """
    global sn
    print("获取机器码（用户识别码）...")
    machine_code = get_machine_code(com)

    if machine_code == None:
        print("获取机器码异常，获取中止...")
        return False

    print("\n获取本地激活码（SN）...")
    sn = ""
    # sn = self.get_sn()    # 从本机查询SN号（本接口可用）
    registrant = "未知"
    # 尝试联网查询
    if sn == "":
        try:
            if activate_sn_url != None and activate_sn_url != "":
                print("联网查询激活码（管理员模式）...")
                response = requests.get(activate_sn_url + machine_code, timeout=3)  # , verify=False
            else:
                print("联网查询激活码...")
                response = requests.get(search_sn_registrant_url + machine_code, timeout=3)  # , verify=False
                print(search_sn_registrant_url + machine_code)
                print(response)
                # 注册者信息
                registrant = response.text.strip().split("\t")[1]
            # sn = re.findall(r'\d+', response.text)
            sn = response.text.strip().split("\t")[0]
            print("sn " + str(sn))
        except Exception as err:
            print(str(traceback.format_exc()))
            print("联网异常")
            return False

    try:
        if sn != "":
            sn_record = open(temp_sn_recode_path, 'a', encoding="utf-8")
            sn_record.write(machine_code + "\t" + sn + "\n")
            sn_record.close()
        else:
            return False
    except Exception as err:
        print(str(traceback.format_exc()))
        print("获取异常异常")

    return True


def get_machine_code(com):
    '''
    查询机器码
    '''
    machine_code = None

    select_com = com
    if select_com == None:
        return machine_code

    try:
        ser = serial.Serial(select_com, info_baud_rate, timeout=10)
    except Exception as err:
        print("串口打开失败")
        return machine_code

    # 判断是否打开成功
    if ser.is_open:

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
        ser.write(send_data.encode('!'))

        time.sleep(1)
        if ser.in_waiting:
            try:
                STRGLO = ser.read(ser.in_waiting).decode("utf8")
                print(STRGLO)
                machine_code = re.findall(r"VALUE_TYPE[_MC]* = \d*", STRGLO)[0] \
                    .split(" ")[-1]
            except Exception as err:
                pass
            print(machine_code)

        if machine_code == None:
            print("机器码查询失败")
        else:
            print("机器码查询成功")

    ser.close()  # 关闭串口
    del ser
    return machine_code


def hard_reset(com):
    """
    重启芯片(控制USB-TLL的rst dst引脚)
    DST RST设置接口都是 true为低电平
    参考文档 https://blog.csdn.net/qq_37388044/article/details/111035944
    可以把以下代码修改到 esptool/targets/esp32s2.py 的 hard_reset()中
    :return:
    """

    select_com = com
    if select_com == None:
        return None
    ser = serial.Serial(select_com, baud_rate, timeout=10)

    ser.setDTR(False)  # IO0=HIGH
    ser.setRTS(True)  # EN=LOW, chip in reset
    time.sleep(0.05)
    ser.setRTS(False)  # EN=HIGH, chip out of reset
    # 0.5 needed for ESP32 rev0 and rev1
    time.sleep(0.05)  # 0.5 / 0.05

    ser.close()  # 关闭串口
    del ser


def auto_active(com):
    """
    自动激活
    """
    print("执行自动激活程序")
    hard_reset(com)  # 复位芯片
    time.sleep(2)  # 等待重启结束查询

    if query_button_click(select_com) == False:
        return False
    try:
        act_button_click(select_com)
    except Exception as err:
        print(str(traceback.format_exc()))


def get_flash_size(select_com):
    """
    :param select_com:串口号
    :return:size, size_text
    """
    global default_wallpaper
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
        cmd = ['--port', select_com, 'flash_id']
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
        cmd = ['--port', select_com, 'chip_id']
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


if __name__ == '__main__':

    # cmd = ['espefuse.py', '-p', 'COM4', 'adc_info']
    # espefuse.main(cmd)
    try:
        print("\nSnailHeater焊台命令行刷机工具 %s\n" % TOOL_VERSION)
        input("\n刷机注意事项：\n1. 确保安装好 CH340 驱动.\n2. 一定要断开 AC220V 供电后，再连接 typec 接口.\n\n\n准备好后敲下回车键继续。")
        print("")
        com_obj_list = list(serial.tools.list_ports.comports())
        # serial.utilities.
        select_com = None
        # 获取可用COM口名字
        com_list = [com_obj[0] + " -> " + com_obj[1].split("(")[0].strip() for com_obj in com_obj_list]
        print("您本机的串口设备有：", com_list, end='\n')
        if len(com_list) == 1:
            select_com = com_list[0].split(" ")[0].strip().upper()
        else:
            select_com = input("输入 COM口（例如 COM7）: ").strip().upper()

        # 列出文件夹下所有的目录与文件
        list_file = os.listdir(firmware_dir)
        firmware_path = ''
        firmware_path_list = []
        for file_name in list_file:
            if 'SnailHeater_v' in file_name or 'SH_SW_v' in file_name:
                firmware_path_list.append(file_name.strip())

        # 固件选择
        if len(firmware_path_list) == 1:
            firmware_path = firmware_path_list[0]
        elif len(firmware_path_list) > 1:
            print("检测到目录下含有多个固件文件：")
            for index in range(len(firmware_path_list)):
                print("    ", index + 1, " --> ", firmware_path_list[index])
            while True:
                number = int(input("请选择固件序号，按回车。（如输入 %d）" % (len(firmware_path_list))).strip())
                if number > len(firmware_path_list) or number < 1:
                    print("\n序号有误!\n")
                else:
                    firmware_path = firmware_path_list[number - 1]
                    break
        else:
            print("未找到固件文件")

        print("已选择刷机固件为：%s\n" % firmware_path)

        # 刷机方式选择
        print("刷机类型:")
        print("    更新式刷机: 保留用户的设置信息，只在固件上做更新。")
        print("    清空式刷机: 将会清空芯片内所有可清空的信息，想要完全纯净的刷固件可选此项")
        print("")
        while True:
            method = int(input("请选择固件刷写方式。（输入1 更新式刷机， 输入2 清空式刷机）").strip())
            if method == 2:
                print("\n清空芯片中.....")
                cmd = ['--port', select_com, 'erase_region',
                       '0x00000', '0x400000']
                esptool.main(cmd)
                print("清空芯片成功！")
                break
            elif method == 1:
                break
            else:
                print("\n选择有误，请重新选择\n")

        # 动态壁纸和背景
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

        chip_id = get_chip_id(select_com)
        print("chip_id = ", chip_id)

        print("正在获取存空间大小...")
        flash_size = 0
        flash_size_text = "0MB"
        if chip_id == CHIP_ID_S2:
            flash_size, flash_size_text = get_flash_size(select_com)
        elif chip_id == CHIP_ID_S3:
            flash_size, flash_size_text = get_flash_size(select_com)
            if flash_size < 8 * 1024 * 1024:
                print("识别到不支持的Flash大小：" + flash_size_text)

        exMediaParam = []
        if method == 2:
            # 清空时，需要重新刷入壁纸
            exMediaParam = [
                get_backgroup_addr_in_flash(chip_id), default_backgroud,
                get_wallpaper_addr_in_flash(chip_id), default_wallpaper]
        #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_40m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000 
        cmd = []
        if chip_id == CHIP_ID_S2:
            flash_size_text = flash_size_text if flash_size_text in ["4MB", "8MB", "16MB", "32MB", "64MB"] else "4MB"
            cmd = ['SnailHeater_WinTool.py', '--port', select_com,
                   '--baud', baud_rate,
                   '--after', 'hard_reset',
                   'write_flash',
                   '--flash_size', flash_size_text,
                   '0x00001000', "./base_data/%s_bootloader_%s.bin" % (chip_id, flash_size_text),
                   '0x00008000', "./base_data/%s_partitions_%s.bin" % (chip_id, flash_size_text),
                   '0x0000e000', "./base_data/%s_boot_app0.bin" % (chip_id),
                   '0x00010000', os.path.join(firmware_dir, firmware_path)
                   ] + exMediaParam
        elif chip_id == CHIP_ID_S3:
            flash_size_text = flash_size_text if flash_size_text in ["4MB", "8MB", "16MB", "32MB"] else "32MB"
            cmd = ['SnailHeater_WinTool.py', '--port', select_com,
                   '--baud', baud_rate,
                   '--after', 'hard_reset',
                   'write_flash',
                   '--flash_size', flash_size_text,
                   '0x00000000', "./base_data/%s_bootloader_%s.bin" % (chip_id, flash_size_text),
                   '0x00008000', "./base_data/%s_partitions_%s.bin" % (chip_id, flash_size_text),
                   # '0x0000e000', "./base_data/%s_boot_app0.bin"% (chip_id) ,
                   '0x00010000', os.path.join(firmware_dir, firmware_path)
                   ] + exMediaParam

        # sys.argv = cmd
        esptool.main(cmd[1:])

        time.sleep(4)  # 等待文件系统初始化完成
        auto_active(select_com)  # 自动激活

    except Exception as err:
        print(err)

    print("\n\n刷机流程完毕，请保持typec通电等待焊台屏幕将会亮起后才能断电。\n注：更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待20s左右。\n")

    print("\n若无法刷机成功，先确认 CH340 驱动是否正常或尝试 typec 调换方向后，重新打开软件执行刷机。\n")

    input("按回车以关闭本软件。")
