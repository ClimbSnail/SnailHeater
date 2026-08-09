# -*- coding: utf-8 -*-
"""现代 WebUI 的独立启动入口，不依赖经典 Qt 界面。"""

from __future__ import annotations

import argparse
import sys
from typing import Literal, Sequence

ProductKind = Literal["snailheater", "el"]


def run(product: ProductKind) -> int:
    """启动指定产品的现代 WebUI 桌面窗口。"""
    from .webapp.desktop import run_web_desktop

    try:
        return run_web_desktop(product)
    except RuntimeError as error:
        print(f"现代界面启动失败：{error}", file=sys.stderr)
        return 2


def main(argv: Sequence[str] | None = None, default_product: ProductKind = "snailheater") -> int:
    """解析产品参数并启动现代 WebUI。"""
    parser = argparse.ArgumentParser(description="SnailHeater 现代 WebUI 启动器")
    parser.add_argument("--product", choices=("snailheater", "el"), default=default_product)
    args = parser.parse_args(argv)
    return run(args.product)
