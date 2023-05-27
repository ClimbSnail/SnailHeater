# encoding: utf-8

# pip install pyserial -i https://mirrors.aliyun.com/pypi/simple/
import sys
import os
import serial  # pip install pyserial
import serial.tools.list_ports
# from esptoolpy import espefuse
from esptoolpy import esptool

VERSION = "Ver1.0"

# 打包脚本
# pyinstaller --icon ./snail_heater_256.ico -w -F SnailHeater_Tool.py

if __name__ == '__main__':

    # cmd = ['espefuse.py', '-p', 'COM4', 'adc_info']
    # espefuse.main(cmd)
    try:
        print("\nSnailHeater焊台专用刷机工具 v1.0\n")
        input("\n刷机注意事项：\n1. 确保安装好 CH340 驱动.\n2. 一定要断开 AC220V 供电后，再连接 typec 接口.\n\n\n准备好后敲下回车键继续。")
        print("")
        com_obj_list = list(serial.tools.list_ports.comports())
        # serial.utilities.
        select_com = None
        # 获取可用COM口名字
        com_list = [com_obj[0] for com_obj in com_obj_list]
        print("您本机的串口设备有：", com_list, end='\n')
        if len(com_list) == 1:
            select_com = com_list[0]
        else:
            select_com = input("输入 COM口（例如 COM7）: ").strip()

        # 列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        firmware_path = ''
        firmware_path_list = []
        for file_name in list_file:
            if 'SnailHeater_v' in file_name:
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
                cmd = [ '--port', select_com, 'erase_flash']
                esptool.main(cmd)
                print("清空芯片成功！")
                break
            elif method == 1:
                break
            else:
                print("\n选择有误，请重新选择\n")

        #  --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_40m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000 
        cmd = ['HeatPlatform_tool.py', '--port', select_com,
               '--baud', '921600',
               'write_flash',
               '0x00001000', "bootloader.bin",
               '0x00008000', "partitions.bin",
               '0x0000e000', "boot_app0.bin",
               '0x002d0000', "tinyuf2.bin",
               '0x00010000', firmware_path
               ]

        # sys.argv = cmd
        esptool.main(cmd[1:])
    except Exception as err:
        print(err)

    print("\n\n刷机流程完毕，请保持typec通电等待焊台屏幕将会亮起后才能断电。\n注：更新式刷机一般刷机完成后2s就能亮屏，清空式刷机则需等待20s左右。\n")

    print("\n若无法刷机成功，先确认 CH340 驱动是否正常或尝试 typec 调换方向后，重新打开软件执行刷机。\n")

    input("按回车以关闭本软件。")
