# -*- coding: utf-8 -*-
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Optional
import requests
from .config import ToolConfig
from .paths import RuntimePaths


@dataclass(frozen=True)
class ActivationInfo:
    sn: str
    registrant: str = "未知"


class ActivationService:
    """激活信息的网络查询与本地缓存。"""

    def __init__(self, config: ToolConfig, paths: RuntimePaths):
        self.config, self.paths = (config, paths)

    def query(self, machine_code: str) -> ActivationInfo:
        """
        查询设备或服务数据。

        Args:
            machine_code: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if self.config.activate_sn_url:
            response = requests.get(self.config.activate_sn_url + machine_code, timeout=2)
            response.raise_for_status()
            return ActivationInfo(response.text.strip().split("\t")[0], "管理员模式")
        if not self.config.search_sn_registrant_url:
            raise RuntimeError("未配置激活码查询地址")
        response = requests.get(self.config.search_sn_registrant_url + machine_code, timeout=2)
        response.raise_for_status()
        result = json.loads(response.text.strip())
        data = result.get("data", {})
        return ActivationInfo(str(data.get("sn", "")), str(data.get("msg", "未知")))

    def append_cache(self, machine_code: str, sn: str):
        """
        将查询结果追加写入本地缓存。

        Args:
            machine_code: 对应的输入参数。
            sn: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if not sn or not self.config.temp_sn_recode_path:
            return
        path = self.paths.resolve(self.config.temp_sn_recode_path)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a", encoding="utf-8") as stream:
            stream.write(machine_code + "\t" + sn + "\n")
