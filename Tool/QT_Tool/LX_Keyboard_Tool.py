# encoding: utf-8

# 打包脚本
# pyinstaller --icon ./images/open_source.ico -F LX_Keyboard_Tool.py

# pip install pyserial -i https://mirrors.aliyun.com/pypi/simple/
import sys
import os
import time
import serial  # pip install pyserial
import serial.tools.list_ports
# from esptoolpy import espefuse
import esptool
import requests
import yaml  # pip install pyyaml
import io
import re
import traceback
import massagehead as mh

TOOL_VERSION = "v1.0.0_beta Lite"

cur_dir = os.getcwd()  # 当前目录
# 生成的文件目录
gen_path = os.path.join(cur_dir, "Generate")

baud_rate = "921600" # 


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

if __name__ == '__main__':

    # cmd = ['espefuse.py', '-p', 'COM4', 'adc_info']
    # espefuse.main(cmd)
    try:
        print("\n蓝星键盘刷机工具 %s By ClimbSnail\n" % TOOL_VERSION)
        print("\n注：工具如有问题可联系QQ： ClimbSnail\n")
        input("\n刷机注意事项：\n1. 确保安装好 USB-TTL 驱动.\n2. 插接好接口.\n3. 设置好拨码开关.\n\n\n准备好后敲下回车键继续。")
        input("\n若给旋钮刷机，请将旋钮通过排线连接至键盘板。或外接独立的USB-TLL（注意连接RXD、TXD、DTR、RTS）。")
        print("")

        # 选择刷机对象
        refresh_obj = int(input("请选择刷机对象。（输入1 键盘主体， 输入2 旋钮）").strip())
        if refresh_obj == 1:
            refresh_obj_str = "keyboard"
        else:
            refresh_obj_str = "knob"
        print("")
        
        com_obj_list = list(serial.tools.list_ports.comports())
        # serial.utilities.
        select_com = None
        # 获取可用COM口名字
        com_list = [com_obj[0]+ " -> " +com_obj[1].split("(")[0].strip() for com_obj in com_obj_list]
        print("您本机的串口设备有：", com_list, end='\n')
        if len(com_list) == 1:
            select_com = com_list[0].split(" ")[0].strip().upper()
        else:
            select_com = input("输入 COM口（例如 COM7）: ").strip().upper()

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
        cmd = ['LX_Keyboard_Tool.py', '--port', select_com,
               '--baud', '921600',
                '--after', 'hard_reset',
                'write_flash',
                '--flash_size', flash_size_text,
                '0x00000000', "./lx_bin/lx_%s_bootloader.bin" % (refresh_obj_str),
                '0x00008000', "./lx_bin/lx_%s_partitions.bin" % (refresh_obj_str),
                '0x0000e000', "./lx_bin/lx_%s_boot_app0.bin" % (refresh_obj_str),
                '0x00010000', "./lx_bin/lx_%s_firmware.bin" % (refresh_obj_str)
               ]

        # sys.argv = cmd
        print("刷机参数 --> ", cmd)
        
        esptool.main(cmd[1:])

    except Exception as err:
        print(err)

    print("\n\n刷机流程完毕，请保持通电等待屏幕将会亮起后才能断电。\n注：更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待10s左右。\n")

    print("\n若无法刷机成功，先确认 USB-TTL 驱动以及连接线是否正常，重新打开软件执行刷机。\n")

    input("按回车以关闭本软件。")
