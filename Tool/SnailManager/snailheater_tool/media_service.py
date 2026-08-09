# -*- coding: utf-8 -*-
import os
import shutil
import struct
import subprocess
import traceback
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Dict, List, Union
from PIL import Image
import lvgl_image_converter as image_conv
from .paths import RuntimePaths
from .profiles import ProductProfile

IMAGE_FORMATS = {"jpg", "jpeg", "png"}
MOVIE_FORMATS = {"mp4", "avi", "mov"}
WALLPAPER_MASK = 17
WALLPAPER_JPG_MAX_SIZE = 17500
MEDIA_TYPES = {"jpeg": 0, "mjpeg": 1, "pcm_u8_1": 124, "rtttl": 125}


@dataclass(frozen=True)
class MediaOptions:
    resolution: str
    start_time: int
    end_time: int
    fps: str
    quality: str


class MediaService:
    """与 Qt 无关的素材参数、转换、容量校验和壁纸打包服务。"""

    def __init__(
        self, paths: RuntimePaths, profile: ProductProfile, background_lvgl_version: int = 8
    ):
        if background_lvgl_version not in {8, 9}:
            raise ValueError(f"Unsupported background LVGL version: {background_lvgl_version}")
        self.paths, self.profile = (paths, profile)
        self.background_lvgl_version = background_lvgl_version

    def ensure_directories(self) -> None:
        """创建壁纸、背景及转换缓存目录；所有媒体输出都集中在这些目录。"""
        self.paths.ensure_output_dirs()

    def convert_mp4_to_rtttl(self, source: Union[str, Path], log: Callable[[str], None]) -> Path:
        """按旧版 MP4→WAV→RTTTL 流程生成可直接打包的 RTTTL 素材。

        这是一项独立转换操作，不改变普通视频默认转为 MJPEG 的行为。输出放在
        ``Generate/WallpaperCache``，随后可作为已有 ``.rtttl`` 素材加入壁纸包。
        """
        if not self.profile.supports_rtttl:
            raise ValueError("当前产品不支持 RTTTL 素材")
        source_path = Path(source).expanduser()
        if source_path.suffix.lower() != ".mp4":
            raise ValueError("RTTTL 转换只支持 MP4 视频文件")
        if not source_path.is_file():
            raise FileNotFoundError(f"MP4 文件不存在：{source_path}")
        self.ensure_directories()
        destination = self.paths.wallpaper_cache_dir / f"{source_path.stem}.rtttl"
        from .rtttl_service import MP4ToRTTTLConverter

        # 旧 Qt 分支对第一个待处理媒体使用 RTitle_0；专用操作继续使用该标题格式。
        converter = MP4ToRTTTLConverter(self.ffmpeg_executable(), log)
        converter.convert(str(source_path), str(destination), rtttl_title="RTitle_0")
        if not destination.is_file() or destination.stat().st_size == 0:
            raise RuntimeError(f"RTTTL 文件生成失败：{destination}")
        return destination

    def ffmpeg_executable(self) -> str:
        """
        返回当前平台可用的 FFmpeg 可执行文件路径。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if self.paths.ffmpeg.is_file():
            return str(self.paths.ffmpeg)
        executable = shutil.which("ffmpeg")
        if executable:
            return executable
        raise FileNotFoundError(f"缺少 ffmpeg：{self.paths.ffmpeg}，系统 PATH 中也未找到 ffmpeg")

    def build_output_params(self, file_text: str, options: MediaOptions) -> Dict[str, object]:
        """
        根据界面选项构建媒体输出参数。

        Args:
            file_text: 输入文件文本。
            options: 媒体输出选项。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        if not file_text:
            raise ValueError("未选择素材文件")
        if options.start_time > options.end_time:
            raise ValueError("不允许开始时间大于结束时间")
        width, height = options.resolution.split(" ")[0].split("x")
        sources = [path for path in file_text.split(";") if path]
        destinations: List[str] = []
        formats: List[str] = []
        qualities: List[str] = []
        for source in sources:
            stem, suffix = os.path.splitext(os.path.basename(source))
            ext = suffix.lstrip(".").lower()
            if ext in MOVIE_FORMATS:
                destinations.append(
                    str(self.paths.wallpaper_cache_dir / f"{stem}_{width}x{height}.mjpeg")
                )
                formats.append("mjpeg")
                qualities.append(options.quality)
            elif ext in IMAGE_FORMATS:
                destinations.append(
                    str(self.paths.wallpaper_cache_dir / f"{stem}_{width}x{height}.jpeg")
                )
                formats.append("jpeg")
                qualities.append("10")
            elif ext in {"lsw", "bin"}:
                destinations.append(str(self.paths.wallpaper_file))
                formats.append(ext)
                qualities.append("10")
            elif ext == "rtttl":
                if not self.profile.supports_rtttl:
                    raise ValueError("EL 工具不支持 RTTTL 素材")
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.rtttl"))
                formats.append(ext)
                qualities.append("10")
            elif ext == "pcm_u8_1":
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.pcm_u8_1"))
                formats.append(ext)
                qualities.append("10")
            else:
                raise ValueError(f"不支持的素材格式：{suffix or source}")
        return {
            "src_path": sources,
            "dst_path": destinations,
            "width": width,
            "height": height,
            "start_time": str(options.start_time),
            "end_time": str(options.end_time),
            "fps": options.fps,
            "quality": qualities,
            "format": formats,
        }

    def convert(
        self, params: Dict[str, object], crop_to_fill: bool, log: Callable[[str], None]
    ) -> None:
        """
        执行输入数据的格式转换。

        Args:
            params: 媒体处理参数。
            crop_to_fill: 是否裁剪填充。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        self.ensure_directories()
        for index, source in enumerate(params["src_path"]):
            destination = params["dst_path"][index]
            media_format = params["format"][index]
            source_path = Path(source)
            destination_path = Path(destination)
            # RTTTL/PCM 本身已是设备素材格式；源文件就在缓存目录时绝不能先删除再复制。
            reuse_existing = (
                media_format in {"rtttl", "pcm_u8_1"}
                and source_path.resolve() == destination_path.resolve()
            )
            if reuse_existing:
                log(f"复用已转换素材：{destination_path}")
                continue
            destination_path.unlink(missing_ok=True)
            if media_format == "mjpeg":
                self._convert_video(source, destination, params, index, log)
            elif media_format == "jpeg":
                self._convert_image(
                    source,
                    destination,
                    int(params["width"]),
                    int(params["height"]),
                    crop_to_fill,
                    log,
                )
            elif media_format in {"rtttl", "pcm_u8_1"}:
                # 已编码的蜂鸣器/RTTTL 素材无需重新压缩，仅复制进统一缓存目录。
                shutil.copy(source, destination)
            elif media_format in {"lsw", "bin"}:
                continue
            else:
                raise ValueError(f"不支持的转换格式：{media_format}")
            if not Path(destination).is_file() or Path(destination).stat().st_size == 0:
                raise RuntimeError(f"生成文件失败：{source}")

    def _convert_video(
        self, source: str, destination: str, params: Dict[str, object], index: int, log
    ) -> None:
        """
        转换单个视频文件并输出处理进度。

        Args:
            source: 源文件路径。
            destination: 目标文件路径。
            params: 媒体处理参数。
            index: 当前资源索引。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        ffmpeg = self.ffmpeg_executable()
        input_path = source
        if params["end_time"] != "0":
            cache = self.paths.wallpaper_cache_dir / (
                Path(source).stem + "_cache" + Path(source).suffix
            )
            cache.unlink(missing_ok=True)
            subprocess.run(
                [
                    ffmpeg,
                    "-y",
                    "-ss",
                    params["start_time"],
                    "-to",
                    params["end_time"],
                    "-i",
                    source,
                    "-c:v",
                    "copy",
                    str(cache),
                ],
                check=True,
            )
            input_path = str(cache)
        width, height = (params["width"], params["height"])
        filters = [
            f"fps={params['fps']},scale=-1:{height}:flags=lanczos,crop={width}:in_h:(in_w-{width})/2:0",
            f"fps={params['fps']},scale={width}:-1:flags=lanczos,crop=in_w:{height}:0:(in_h-{height})/2",
        ]
        for attempt, video_filter in enumerate(filters):
            subprocess.run(
                [
                    ffmpeg,
                    "-y",
                    "-i",
                    input_path,
                    "-vf",
                    video_filter,
                    "-q:v",
                    params["quality"][index],
                    destination,
                ],
                check=False,
            )
            if Path(destination).is_file() and Path(destination).stat().st_size:
                return
            if attempt == 0:
                log("原视频宽高比不匹配，正在尝试另一种最大化裁剪方式")
        raise RuntimeError(f"视频转换失败：{source}")

    @staticmethod
    def _convert_image(
        source: str, destination: str, width: int, height: int, crop_to_fill: bool, log
    ) -> None:
        """
        转换单张图片并输出处理进度。

        Args:
            source: 源文件路径。
            destination: 目标文件路径。
            width: 目标宽度。
            height: 目标高度。
            crop_to_fill: 是否裁剪填充。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        with Image.open(source) as image:
            if crop_to_fill:
                if image.height / height > image.width / width:
                    new_width = image.width
                    new_height = new_width * height / width
                    rect = (
                        0,
                        (image.height - new_height) / 2,
                        new_width,
                        (image.height + new_height) / 2,
                    )
                else:
                    new_height = image.height
                    new_width = new_height * width / height
                    rect = (
                        (image.width - new_width) / 2,
                        0,
                        (image.width + new_width) / 2,
                        new_height,
                    )
                log(
                    f"宽x高 -> {image.width}x{image.height}，裁剪 -> {int(new_width)}x{int(new_height)}"
                )
                image = image.crop(rect)
            if image.mode not in {"RGB", "L"}:
                image = image.convert("RGB")
            result = image.resize((width, height), Image.BICUBIC)
            result.save(destination)
            is_jpeg = Path(destination).suffix.lower() in {".jpg", ".jpeg"}
            if is_jpeg and Path(destination).stat().st_size >= WALLPAPER_JPG_MAX_SIZE:
                for quality in range(95, 5, -5):
                    result.save(destination, quality=quality)
                    if Path(destination).stat().st_size < WALLPAPER_JPG_MAX_SIZE:
                        break

    def prepare_background(
        self, params: Dict[str, object], crop_to_fill: bool, log: Callable[[str], None]
    ) -> Path:
        """将首个图片或 BIN 素材转换为可烧录的 RGB565 背景文件。"""
        self.ensure_directories()
        selected_index = next(
            (index for (index, fmt) in enumerate(params["format"]) if fmt in {"jpeg", "bin"}), None
        )
        if selected_index is None:
            raise ValueError("参数出错：只有选中的第一张图片或 BIN 文件会生效")
        source = Path(params["src_path"][selected_index])
        width, height = (int(params["width"]), int(params["height"]))
        if params["format"][selected_index] == "bin":
            output = source
            if self.background_lvgl_version == 9:
                self._validate_lvgl9_background(output, width, height)
        else:
            image_extension = ".png" if self.background_lvgl_version == 9 else ".jpg"
            standard_image = self.paths.background_cache_dir / (source.stem + image_extension)
            self._convert_image(str(source), str(standard_image), width, height, crop_to_fill, log)
            output = self.paths.background_dir / (standard_image.stem + ".bin")
            if self.background_lvgl_version == 9:
                self._convert_background_lvgl9(standard_image, output)
                self._validate_lvgl9_background(output, width, height)
            else:
                args = [
                    str(standard_image),
                    "-f",
                    "true_color",
                    "-cf",
                    "RGB565SWAP",
                    "-ff",
                    "BIN",
                    "-o",
                    str(self.paths.background_dir),
                ]
                converter = image_conv.lv_img_conv.Main(image_conv.lv_img_conv.parse_args(args))
                converter.convert()
        if not output.is_file():
            raise FileNotFoundError(f"背景文件生成失败：{output}")
        self.validate_capacity(output, 320 * 1024)
        return output

    @staticmethod
    def _convert_background_lvgl9(source: Path, output: Path) -> None:
        """Convert a PNG to the uncompressed LVGL 9 RGB565_SWAPPED BIN format."""
        try:
            from lvgl_image_converter.LVGLImage import ColorFormat, LVGLImage
        except ModuleNotFoundError as error:
            raise RuntimeError(
                "LVGL 9 background conversion requires the pypng and lz4 dependencies"
            ) from error

        image = LVGLImage().from_png(str(source), ColorFormat.RGB565_SWAPPED)
        image.adjust_stride(align=1)
        image.to_bin(str(output))

    @staticmethod
    def _validate_lvgl9_background(path: Path, width: int, height: int) -> None:
        """Reject prebuilt BIN files that cannot be used by the LVGL 9 EL firmware."""
        header = path.read_bytes()[:12]
        if len(header) != 12:
            raise ValueError("LVGL 9 background BIN is missing its 12-byte header")
        if header[0] != 0x19:
            raise ValueError("LVGL 9 background BIN has an invalid magic value")
        if header[1] != 0x1B:
            raise ValueError("LVGL 9 background BIN must use RGB565_SWAPPED")
        if header[2:4] != b"\x00\x00":
            raise ValueError("LVGL 9 background BIN must be uncompressed")

        actual_width = int.from_bytes(header[4:6], "little")
        actual_height = int.from_bytes(header[6:8], "little")
        stride = int.from_bytes(header[8:10], "little")
        if (actual_width, actual_height) != (width, height):
            raise ValueError(
                f"LVGL 9 background BIN resolution is {actual_width}x{actual_height}; expected {width}x{height}"
            )
        expected_stride = width * 2
        if stride != expected_stride:
            raise ValueError(
                f"LVGL 9 background BIN stride is {stride}; expected {expected_stride}"
            )
        expected_size = 12 + stride * height
        actual_size = path.stat().st_size
        if actual_size != expected_size:
            raise ValueError(
                f"LVGL 9 background BIN size is {actual_size}; expected {expected_size}"
            )

    def pack_wallpaper(self, params: Dict[str, object], log: Callable[[str], None]) -> Path:
        """
        将生成的壁纸资源打包为设备格式。

        Args:
            params: 媒体处理参数。
            log: 日志输出回调。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        wallpapers = params["dst_path"]
        fps = int(params["fps"])
        # 固件将前 580 字节视为目录区；每项描述类型、数据偏移和长度，资源随后紧密排列。
        entries = []
        offset = 580
        for index, wallpaper in enumerate(wallpapers):
            suffix = Path(wallpaper).suffix.lstrip(".").lower()
            size = Path(wallpaper).stat().st_size
            if suffix == "jpeg" and size >= 20000:
                log(f"此图片文件过大（已忽略）：{params['src_path'][index]}")
                continue
            entries.append((MEDIA_TYPES.get(suffix, 126), offset, size, wallpaper))
            offset += size
        # RTTTL 的类型值为 125，与原 Qt 工具写入的壁纸包格式保持兼容。
        header = struct.pack("=1H1B1B", WALLPAPER_MASK, len(entries), fps)
        for media_type, address, size, _ in entries:
            header += struct.pack("=1B1I1I", media_type, address, size)
        if len(header) > 580:
            raise ValueError("壁纸目录超过 580 字节上限")
        with self.paths.wallpaper_file.open("wb") as output:
            output.write(header + b"\x00" * (580 - len(header)))
            for _, _, _, wallpaper in entries:
                with open(wallpaper, "rb") as source:
                    shutil.copyfileobj(source, output)
        return self.paths.wallpaper_file

    @staticmethod
    def validate_capacity(path: Union[str, Path], capacity: int) -> int:
        """
        校验文件大小是否超过目标容量。

        Args:
            path: 文件路径。
            capacity: 允许的最大容量。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        size = os.path.getsize(str(path))
        if size > capacity:
            raise ValueError("壁纸数据过大，请适当降低帧率或截取更短的时间")
        return int(size / capacity * 100) if capacity else 0
