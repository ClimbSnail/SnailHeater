# -*- coding: utf-8 -*-
import re
import time
from contextlib import contextmanager
from typing import List, Optional, Tuple
import serial
import serial.tools.list_ports
import massagehead as mh


class DeviceService:
    """与 Qt 无关的串口设备服务。"""

    def __init__(self, info_baud_rate: str, flash_baud_rate: str):
        self.info_baud_rate = int(info_baud_rate)
        self.flash_baud_rate = int(flash_baud_rate)

    @staticmethod
    def list_ports() -> List[Tuple[str, str]]:
        return [(p[0], p[1].split("(")[0].strip()) for p in serial.tools.list_ports.comports()]

    @classmethod
    def display_ports(cls) -> List[str]:
        return [f"{name} -> {description}" for (name, description) in cls.list_ports()] or [
            "未识别到"
        ]

    @staticmethod
    def parse_selected_port(text: str) -> str:
        return (text or "").split(" -> ")[0].strip()

    def validate_port(self, selected_text: str) -> Optional[str]:
        selected = self.parse_selected_port(selected_text)
        return selected if selected in {name for (name, _) in self.list_ports()} else None

    @contextmanager
    def open_info_serial(self, port: str, timeout: float = 10):
        """
        打开用于读取设备信息的串口。

        Args:
            port: 串口名称或串口对象。
            timeout: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        device = serial.Serial(port, self.info_baud_rate, timeout=timeout)
        try:
            yield device
        finally:
            if device and device.is_open:
                device.close()

    def hard_reset(self, port: str):
        """
        通过串口执行设备硬复位。

        Args:
            port: 串口名称或串口对象。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        with serial.Serial(port, self.flash_baud_rate, timeout=10) as device:
            device.setDTR(False)
            device.setRTS(True)
            time.sleep(0.05)
            device.setRTS(False)
            time.sleep(0.05)

    @staticmethod
    def _setting_message(action: int, value_type: int, value: str = "") -> bytes:
        """
        构造设备设置消息。

        Args:
            action: 对应的输入参数。
            value_type: 对应的输入参数。
            value: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        message = mh.SettingMsg()
        message.action_type = action
        message.key = b""
        message.type = value_type.to_bytes(1, byteorder="little", signed=True)
        message.value = value.encode("utf-8")
        return message.encode("!")

    def set_value(self, port: str, value_type: int, value: str) -> bool:
        """
        向设备写入指定的配置值。

        Args:
            port: 串口名称或串口对象。
            value_type: 对应的输入参数。
            value: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        with self.open_info_serial(port) as device:
            device.write(self._setting_message(mh.AT.AT_SETTING_SET, value_type, value))
            time.sleep(1)
            data = device.read(device.in_waiting) if device.in_waiting else b""
        return b"Success" in data

    def query_value(self, port: str, value_type: int, pattern: str) -> str:
        """
        从设备读取指定的配置值。

        Args:
            port: 串口名称或串口对象。
            value_type: 对应的输入参数。
            pattern: 对应的输入参数。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        with self.open_info_serial(port) as device:
            device.write(self._setting_message(mh.AT.AT_SETTING_GET, value_type))
            time.sleep(1)
            data = device.read(device.in_waiting) if device.in_waiting else b""
        text = data.replace(b"U\xaa", b"##").decode("utf-8", errors="ignore")
        match = re.search(pattern, text)
        return match.group(1).strip() if match else ""
