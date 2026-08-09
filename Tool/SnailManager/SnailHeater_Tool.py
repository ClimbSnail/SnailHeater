#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""SnailHeater 命令行工具兼容入口。"""

from tools.cli.SnailHeater_Tool import (
    TOOL_VERSION,
    act_button_click,
    auto_active,
    get_backgroup_addr_in_flash,
    get_chip_id,
    get_flash_size,
    get_machine_code,
    get_wallpaper_addr_in_flash,
    hard_reset,
    main,
    query_button_click,
)

__all__ = [
    "TOOL_VERSION",
    "act_button_click",
    "auto_active",
    "get_backgroup_addr_in_flash",
    "get_chip_id",
    "get_flash_size",
    "get_machine_code",
    "get_wallpaper_addr_in_flash",
    "hard_reset",
    "main",
    "query_button_click",
]


if __name__ == "__main__":
    main()
