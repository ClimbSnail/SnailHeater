import sys
import os
import serial
from esptoolpy import esptool

VERSION = "Ver1.0"

# 打包脚本
# pyinstaller --icon ./snail_heater_256.ico -w -F SnailHeater_Tool.py

if __name__ == '__main__':
    try:
        input("\n刷机注意事项：\n1. 确保安装好 CH340 驱动.\n2. 一定要断开 AC220V 供电后，再连接 typec 接口.\n\n\n准备好后敲下回车键继续。\n")
        com_obj_list = list(serial.tools.list_ports.comports())

        select_com = None
        # 获取可用COM口名字
        com_list = [com_obj[0] for com_obj in com_obj_list]
        print("您本机的串口设备有：", end='')
        print(com_list)
        if len(com_list) == 1:
            select_com = com_list[0]
        else:
            select_com = input("输入 COM口（例如 COM7）: ")

        print("\n若无法刷机成功，先确认 CH340 驱动是否正常或尝试 typec 调换方向后，重新打开软件执行刷机。\n")

        
        #列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        firmware_path = 'SnailHeater_v1.0.bin'
        for file_name in list_file:
            if 'SnailHeater_v' in file_name:
                firmware_path = file_name.strip()
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
    
    input("刷机完毕，请保持typec通电等待10s左右焊台屏幕将会亮起。\n按回车以关闭本软件。")