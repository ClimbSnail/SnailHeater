
import struct
import os

wallpaper_name = "WallpaperClean.lsw"
verMark = 0x11  # 版本号
total = 0  # 壁纸总数（1字节）
fps = 20  # 帧率

with open(wallpaper_name, "wb") as fbin:
    binaryData = b'' + struct.pack('=' + "1H1B1B", *[verMark, total, fps])
    binaryData = binaryData + b'\x00' * (580 - len(binaryData))
    fbin.write(binaryData)

print("壁纸文件生成成功：" + wallpaper_name)