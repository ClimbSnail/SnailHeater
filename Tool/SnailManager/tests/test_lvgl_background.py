# -*- coding: utf-8 -*-
import tempfile
import unittest
import struct
from pathlib import Path
from unittest.mock import patch

from PIL import Image

from snailheater_tool.media_service import MediaOptions, MediaParams, MediaService
from snailheater_tool.paths import RuntimePaths
from snailheater_tool.profiles import EL_PROFILE, SNAILHEATER_PROFILE
from snailheater_tool.webapp.session import WebToolSession


class LvglBackgroundTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.root = Path(self.temp_dir.name)
        self.external_root_patcher = patch(
            "snailheater_tool.paths.external_data_root", return_value=self.root
        )
        self.external_root_patcher.start()
        self.paths = RuntimePaths(self.root)
        self.source = self.root / "source.png"
        Image.new("RGB", (2, 1), (255, 0, 0)).save(self.source)
        self.params = MediaParams(
            src_path=[str(self.source)],
            dst_path=[str(self.root / "background.jpeg")],
            width=2,
            height=1,
            start_time="0",
            end_time="0",
            fps="20",
            quality=["10"],
            format=["jpeg"],
        )

    def tearDown(self) -> None:
        self.external_root_patcher.stop()
        self.temp_dir.cleanup()

    def test_lvgl9_output_has_expected_header_and_swapped_pixels(self) -> None:
        service = MediaService(self.paths, EL_PROFILE, background_lvgl_version=9)
        output = service.prepare_background(self.params, True, lambda _: None)

        data = output.read_bytes()
        self.assertEqual(data[:12], bytes([0x19, 0x1B, 0, 0, 2, 0, 1, 0, 4, 0, 0, 0]))
        self.assertEqual(len(data), 12 + 2 * 1 * 2)
        self.assertEqual(data[12:14], b"\xF8\x00")

    def test_lvgl8_output_keeps_legacy_four_byte_header(self) -> None:
        service = MediaService(self.paths, SNAILHEATER_PROFILE)
        output = service.prepare_background(self.params, True, lambda _: None)

        self.assertEqual(output.stat().st_size, 4 + 2 * 1 * 2)
        self.assertNotEqual(output.read_bytes()[0], 0x19)

    def test_lvgl9_validation_rejects_invalid_bin_headers(self) -> None:
        invalid = self.root / "invalid.bin"
        valid_header = bytearray([0x19, 0x1B, 0, 0, 2, 0, 1, 0, 4, 0, 0, 0])
        valid_data = b"\x00" * 4

        invalid.write_bytes(b"\x00" * 12)
        with self.assertRaisesRegex(ValueError, "magic"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

        invalid.write_bytes(bytes([0x19, 0x12]) + bytes(valid_header[2:]) + valid_data)
        with self.assertRaisesRegex(ValueError, "RGB565_SWAPPED"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

        compressed = bytearray(valid_header)
        compressed[2] = 0x08
        invalid.write_bytes(compressed + valid_data)
        with self.assertRaisesRegex(ValueError, "uncompressed"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

        wrong_resolution = bytearray(valid_header)
        wrong_resolution[4] = 3
        invalid.write_bytes(wrong_resolution + valid_data)
        with self.assertRaisesRegex(ValueError, "resolution"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

        wrong_stride = bytearray(valid_header)
        wrong_stride[8] = 2
        invalid.write_bytes(wrong_stride + valid_data)
        with self.assertRaisesRegex(ValueError, "stride"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

        invalid.write_bytes(valid_header)
        with self.assertRaisesRegex(ValueError, "size"):
            MediaService._validate_lvgl9_background(invalid, 2, 1)

    def test_web_sessions_select_the_expected_background_format(self) -> None:
        self.assertEqual(WebToolSession("el").media_service.background_lvgl_version, 9)
        self.assertEqual(WebToolSession("snailheater").media_service.background_lvgl_version, 8)

    def test_mp4_builds_mjpeg_and_pwm_song_outputs(self) -> None:
        service = MediaService(self.paths, SNAILHEATER_PROFILE)
        params = service.build_output_params(
            "movie.mp4;",
            MediaOptions("320x240", 0, 0, "20", "5"),
        )

        self.assertEqual(params.src_path, ["movie.mp4", "movie.mp4"])
        self.assertEqual(params.format, ["pwm_song", "mjpeg"])
        self.assertEqual(params.quality, ["10", "5"])
        self.assertEqual(
            [Path(path).suffix for path in params.dst_path],
            [".pwm_song", ".mjpeg"],
        )

    def test_pwm_song_uses_little_endian_uint16_pairs(self) -> None:
        encoded = MediaService._encode_pwmsong([(440, 500), (0, 100)])

        self.assertEqual(
            encoded,
            struct.pack("<HHHH", 440, 500, 0, 100),
        )
        self.assertEqual(
            MediaService._encode_pwmsong([(440, 65550)]),
            struct.pack("<HHHH", 440, 65500, 440, 50),
        )

    def test_generate_dir_uses_executable_directory_when_frozen(self) -> None:
        executable = self.root / "app" / "SnailHeater_Modern_EL.exe"
        self.external_root_patcher.stop()
        with patch("snailheater_tool.paths.sys.frozen", True, create=True), patch(
                "snailheater_tool.paths.sys.executable", str(executable)
        ):
            self.assertEqual(self.paths.generate_dir, executable.parent / "Generate")


if __name__ == "__main__":
    # python tools\play_pwmsong.py "D:\Workspace\OpenWorkspace\SnailHeater\Tool\SnailManager\Generate\Cache\Wallpaper\3000万年前的迪迦童年珍贵片段_5.pwm_song"
    # python tools\play_pwmsong.py "D:\Workspace\OpenWorkspace\SnailHeater\Tool\SnailManager\Generate\Cache\Wallpaper\3000万年前的迪迦童年珍贵片段_50.pwm_song"
    unittest.main()
