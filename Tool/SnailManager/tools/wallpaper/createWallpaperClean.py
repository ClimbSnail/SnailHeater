#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成不包含任何素材的 WallpaperClean.lsw。"""

import argparse
import struct
from pathlib import Path


def create_wallpaper_clean(output: Path, fps: int = 20) -> Path:
    """
    创建清理后的默认壁纸资源。

    Args:
        output: 输出文件路径。
        fps: 对应的输入参数。

    Returns:
        函数处理结果；具体类型由调用场景决定。
    """
    version_mark = 0x11
    total = 0
    binary_data = struct.pack("=1H1B1B", version_mark, total, fps)
    binary_data += b"\x00" * (580 - len(binary_data))
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(binary_data)
    return output


def main() -> None:
    """
    执行程序入口逻辑。

    Returns:
        函数处理结果；具体类型由调用场景决定。
    """
    parser = argparse.ArgumentParser(description="生成空白壁纸文件")
    parser.add_argument(
        "output",
        nargs="?",
        default="WallpaperClean.lsw",
        help="输出文件路径，默认写入当前目录",
    )
    parser.add_argument("--fps", type=int, default=20, help="壁纸帧率字段，默认 20")
    args = parser.parse_args()
    output = create_wallpaper_clean(Path(args.output), args.fps)
    print(f"壁纸文件生成成功：{output.resolve()}")


if __name__ == "__main__":
    main()
