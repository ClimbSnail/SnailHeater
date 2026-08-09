#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""SnailHeater EL 现代 WebUI 入口。"""

import sys
from snailheater_tool.launcher import main

if __name__ == "__main__":
    sys.exit(main(default_product="el"))
