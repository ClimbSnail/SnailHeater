# encoding: utf-8

# 打包脚本
# pyinstaller --icon ./images/open_source.ico -F esp_greneral_tool.py

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

TOOL_VERSION = "v1.0.0 Lite"

cur_dir = os.getcwd()  # 当前目录

# 读取配置信息
cfg_fp = open("esp_greneral_tool.yaml", "r", encoding="utf-8")

win_cfg = yaml.load(cfg_fp, Loader=yaml.SafeLoader)["windows_tool"]

tool_name = win_cfg["tool_name"] \
    if "tool_name" in win_cfg.keys() else ""
tool_start_info = win_cfg["tool_start_info"] \
    if "tool_start_info" in win_cfg.keys() else ""
tool_end_info = win_cfg["tool_end_info"] \
    if "tool_end_info" in win_cfg.keys() else ""

qq_info = win_cfg["qq_info"].split(",") \
    if "qq_info" in win_cfg.keys() else ["", ""]
info_url_0 = win_cfg["info_url_0"] \
    if "info_url_0" in win_cfg.keys() else ""
info_url_1 = win_cfg["info_url_1"] \
    if "info_url_1" in win_cfg.keys() else ""
download_baud_rate = win_cfg["download_baud_rate"] \
    if "download_baud_rate" in win_cfg.keys() else ""
info_baud_rate = win_cfg["info_baud_rate"] \
    if "info_baud_rate" in win_cfg.keys() else ""
main_appdir_rules = win_cfg["main_appdir_rules"] \
    if "main_appdir_rules" in win_cfg.keys() else None
main_app_rules = win_cfg["main_app_rules"] \
    if "main_app_rules" in win_cfg.keys() else None
main_app_addr = win_cfg["main_app_addr"] \
    if "main_app_addr" in win_cfg.keys() else None
firmware_info_list = win_cfg["firmware_info_list"] \
    if "firmware_info_list" in win_cfg.keys() else []
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

cfg_fp.close()


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
    ser = serial.Serial(select_com, download_baud_rate, timeout=10)

    ser.setDTR(False)  # IO0=HIGH
    ser.setRTS(True)  # EN=LOW, chip in reset
    time.sleep(0.05)
    ser.setRTS(False)  # EN=HIGH, chip out of reset
    # 0.5 needed for ESP32 rev0 and rev1
    time.sleep(0.05)  # 0.5 / 0.05

    ser.close()  # 关闭串口
    del ser


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


def down():
    # cmd = ['espefuse.py', '-p', 'COM4', 'adc_info']
    # espefuse.main(cmd)
    try:
        print("\n%s %s By ClimbSnail\n" % (tool_name, TOOL_VERSION))
        input(tool_start_info)

        # 选择规则对象

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
        list_file = os.listdir(main_appdir_rules)
        firmware_path_list = []
        # print("main_app_rules" , main_app_rules)
        for filename in list_file:
            match_info = re.findall(main_app_rules, filename)
            if match_info != []:
                firmware_path_list.append(os.path.join(main_appdir_rules, filename))
        firmware_num = 0  # 所选择的固件
        if len(firmware_path_list) != 0:
            print("已找到如下固件：")
            for ind in range(len(firmware_path_list)):
                print("\t%d: %s" % (ind + 1, firmware_path_list[ind]))
            firmware_num = int(input("请选择固件序号：")) - 1
        else:
            print("未找到相关固件，程序终止。请检查目录路径或固件名称。")
            return None

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

        print("正在获取存空间大小...")
        flash_size, flash_size_text = get_flash_size(select_com)

        #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_40m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000 
        cmd = ['--port', select_com,
               '--baud', download_baud_rate,
               '--after', 'hard_reset',
               'write_flash',
               '--flash_size', flash_size_text
               ]
        # 主固件
        cmd.append(main_app_addr)
        cmd.append(firmware_path_list[firmware_num])
        # 辅助bin文件
        if firmware_info_list != None and firmware_info_list != []:
            for bin_obj in firmware_info_list:
                cmd.append(bin_obj["addr"])
                cmd.append(bin_obj["filepath"])

        # sys.argv = cmd
        print("刷机参数 --> ", cmd)

        esptool.main(cmd)

    except Exception as err:
        print(str(traceback.format_exc()))

    print(tool_end_info)
    input("按回车以关闭本软件。")


if __name__ == '__main__':
    down()
