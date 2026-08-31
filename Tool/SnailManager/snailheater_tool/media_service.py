# -*- coding: utf-8 -*-
import os
import shutil
import struct
import subprocess
import tempfile
import traceback
import wave
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, List, Union
from PIL import Image
import lvgl_image_converter as image_conv
from .paths import RuntimePaths
from .profiles import ProductProfile

IMAGE_FORMATS = {"jpg", "jpeg", "png"}
MOVIE_FORMATS = {"mp4", "avi", "mov"}
WALLPAPER_MASK = 17
WALLPAPER_JPG_MAX_SIZE = 17500
MEDIA_TYPES = {"jpeg": 0, "mjpeg": 1, "pwm_song": 123, "pcm_u8_1": 124, "rtttl": 125}
PWM_SONG_TICK_MS = 5
PWM_SONG_SAMPLE_RATE = 22050
PWM_SONG_MIN_FREQ = 80.0
PWM_SONG_MAX_FREQ = 4000.0
PWM_SONG_MAX_DURATION_MS = (0xFFFF // PWM_SONG_TICK_MS) * PWM_SONG_TICK_MS


@dataclass(frozen=True)
class MediaOptions:
    resolution: str
    start_time: int
    end_time: int
    fps: str
    quality: str


@dataclass
class MediaParams:
    """Shared parameters for media conversion and wallpaper packaging."""

    __slots__ = (
        "src_path",
        "dst_path",
        "width",
        "height",
        "start_time",
        "end_time",
        "fps",
        "quality",
        "format",
    )

    src_path: List[str]
    dst_path: List[str]
    width: int
    height: int
    start_time: str
    end_time: str
    fps: str
    quality: List[str]
    format: List[str]

    def __post_init__(self) -> None:
        lengths = {
            len(self.src_path),
            len(self.dst_path),
            len(self.quality),
            len(self.format),
        }
        if len(lengths) != 1:
            raise ValueError(
                "媒体参数长度不一致："
                f"src={len(self.src_path)}, dst={len(self.dst_path)}, "
                f"quality={len(self.quality)}, format={len(self.format)}"
            )


class MediaService:
    """Media conversion, capacity validation, and wallpaper packaging service."""

    def __init__(
        self, paths: RuntimePaths, profile: ProductProfile, background_lvgl_version: int = 8
    ):
        if background_lvgl_version not in {8, 9}:
            raise ValueError(f"Unsupported background LVGL version: {background_lvgl_version}")
        self.paths, self.profile = (paths, profile)
        self.background_lvgl_version = background_lvgl_version

    def ensure_directories(self) -> None:
        """Create the wallpaper, background, and conversion cache directories."""
        self.paths.ensure_output_dirs()

    def convert_mp4_to_rtttl(self, source: Union[str, Path], log: Callable[[str], None]) -> Path:
        """Convert the audio track of an MP4 file to an RTTTL asset."""
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

        # Keep the legacy RTTTL title format for compatibility.
        converter = MP4ToRTTTLConverter(self.ffmpeg_executable(), log)
        converter.convert(str(source_path), str(destination), rtttl_title="RTitle_0")
        if not destination.is_file() or destination.stat().st_size == 0:
            raise RuntimeError(f"RTTTL 文件生成失败：{destination}")
        return destination

    def ffmpeg_executable(self) -> str:
        """Return the available FFmpeg executable path."""
        if self.paths.ffmpeg.is_file():
            return str(self.paths.ffmpeg)
        executable = shutil.which("ffmpeg")
        if executable:
            return executable
        raise FileNotFoundError(f"缺少 ffmpeg：{self.paths.ffmpeg}，系统 PATH 中也未找到 ffmpeg")

    def build_output_params(self, file_text: str, options: MediaOptions) -> MediaParams:
        """Build normalized parameters for media conversion and packaging."""
        if not file_text:
            raise ValueError("未选择素材文件")
        if options.start_time > options.end_time:
            raise ValueError("不允许开始时间大于结束时间")
        width, height = (int(value) for value in options.resolution.split(" ")[0].split("x"))
        input_sources = [path for path in file_text.split(";") if path]
        sources: List[str] = []
        destinations: List[str] = []
        formats: List[str] = []
        qualities: List[str] = []
        for source in input_sources:
            stem, suffix = os.path.splitext(os.path.basename(source))
            ext = suffix.lstrip(".").lower()
            if ext in MOVIE_FORMATS:
                # Keep source, destination, format, and quality entries aligned.
                sources.extend([source, source])
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.pwm_song"))
                formats.append("pwm_song")
                qualities.append("10")

                destinations.append(
                    str(self.paths.wallpaper_cache_dir / f"{stem}_{width}x{height}.mjpeg")
                )
                formats.append("mjpeg")
                qualities.append(options.quality)
            elif ext in IMAGE_FORMATS:
                sources.append(source)
                destinations.append(
                    str(self.paths.wallpaper_cache_dir / f"{stem}_{width}x{height}.jpeg")
                )
                formats.append("jpeg")
                qualities.append("10")
            elif ext in {"lsw", "bin"}:
                sources.append(source)
                destinations.append(str(self.paths.wallpaper_file))
                formats.append(ext)
                qualities.append("10")
            elif ext == "rtttl":
                if not self.profile.supports_rtttl:
                    raise ValueError("EL 工具不支持 RTTTL 素材")
                sources.append(source)
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.rtttl"))
                formats.append(ext)
                qualities.append("10")
            elif ext == "pcm_u8_1":
                sources.append(source)
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.pcm_u8_1"))
                formats.append(ext)
                qualities.append("10")
            elif ext == "pwm_song":
                sources.append(source)
                destinations.append(str(self.paths.wallpaper_cache_dir / f"{stem}.pwm_song"))
                formats.append(ext)
                qualities.append("10")
            else:
                raise ValueError(f"不支持的素材格式：{suffix or source}")
        return MediaParams(
            src_path=sources,
            dst_path=destinations,
            width=width,
            height=height,
            start_time=str(options.start_time),
            end_time=str(options.end_time),
            fps=options.fps,
            quality=qualities,
            format=formats,
        )

    def convert(
        self,
        params: MediaParams,
        crop_to_fill: bool,
        log: Callable[[str], None],
    ) -> None:
        """Convert all input media according to the requested output formats."""
        self.ensure_directories()
        for index, source in enumerate(params.src_path):
            destination = params.dst_path[index]
            media_format = params.format[index]
            source_path = Path(source)
            destination_path = Path(destination)
            # Pre-encoded device assets must not be deleted before copying.
            reuse_existing = (
                media_format in {"rtttl", "pcm_u8_1", "pwm_song"}
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
                    params.width,
                    params.height,
                    crop_to_fill,
                    log,
                )
            elif media_format == "pwm_song" and source_path.suffix.lower().lstrip(".") in MOVIE_FORMATS:
                self._convert_pwmsong(source, destination, params, log)
            elif media_format in {"rtttl", "pcm_u8_1", "pwm_song"}:
                # Pre-encoded buzzer assets are copied directly into the cache.
                shutil.copy(source, destination)
            elif media_format in {"lsw", "bin"}:
                continue
            else:
                raise ValueError(f"不支持的转换格式：{media_format}")
            if not Path(destination).is_file() or Path(destination).stat().st_size == 0:
                raise RuntimeError(f"生成文件失败：{source}")

    def _convert_pwmsong(
        self,
        source: Union[str, Path],
        destination: Union[str, Path],
        params: MediaParams,
        log: Callable[[str], None],
    ) -> None:
        """Convert a video audio track to PWM_SONG_TICK_MS ms aligned PWM Song binary data."""
        source_path = Path(source).expanduser()
        destination_path = Path(destination)
        if not source_path.is_file():
            raise FileNotFoundError(f"视频文件不存在：{source_path}")
        if source_path.suffix.lower().lstrip(".") not in MOVIE_FORMATS:
            raise ValueError(f"PWM Song 转换只支持视频文件：{source_path}")

        try:
            import numpy as np
        except ModuleNotFoundError as error:
            raise RuntimeError("PWM Song 转换需要 numpy 依赖") from error

        self.ensure_directories()
        with tempfile.TemporaryDirectory(prefix="snailheater_pwmsong_") as temp_dir:
            wav_path = Path(temp_dir) / "audio.wav"
            try:
                ffmpeg_args = [self.ffmpeg_executable(), "-y"]
                if params.end_time != "0":
                    ffmpeg_args.extend(["-ss", params.start_time, "-to", params.end_time])
                ffmpeg_args.extend(
                    [
                        "-i",
                        str(source_path),
                        "-vn",
                        "-ac",
                        "1",
                        "-ar",
                        str(PWM_SONG_SAMPLE_RATE),
                        "-c:a",
                        "pcm_s16le",
                        str(wav_path),
                    ]
                )
                subprocess.run(
                    ffmpeg_args,
                    check=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                )
            except subprocess.CalledProcessError as error:
                detail = error.stderr.decode(errors="replace").strip() if error.stderr else ""
                raise RuntimeError(f"提取视频音频失败：{detail or source_path}") from error

            sample_rate, samples = self._read_wav_mono(wav_path, np)
            notes = self._analyze_pwmsong(samples, sample_rate, np)

        destination_path.parent.mkdir(parents=True, exist_ok=True)
        destination_path.write_bytes(self._encode_pwmsong(notes))
        log(f"PWM Song 生成完成：{destination_path}（{len(notes)} 个音符/休止符）")

    @staticmethod
    def _read_wav_mono(path: Path, np):
        """Read PCM WAV data and convert it to mono float samples in [-1, 1]."""
        with wave.open(str(path), "rb") as wav_file:
            channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            sample_rate = wav_file.getframerate()
            raw = wav_file.readframes(wav_file.getnframes())

        if channels <= 0 or sample_rate <= 0:
            raise ValueError(f"WAV 音频参数无效：{path}")
        if sample_width == 1:
            samples = (np.frombuffer(raw, dtype=np.uint8).astype(np.float32) - 128.0) / 128.0
        elif sample_width == 2:
            samples = np.frombuffer(raw, dtype="<i2").astype(np.float32) / 32768.0
        elif sample_width == 3:
            packed = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3)
            samples = (
                packed[:, 0].astype(np.int32)
                | (packed[:, 1].astype(np.int32) << 8)
                | (packed[:, 2].astype(np.int32) << 16)
            )
            negative = (samples & 0x800000) != 0
            samples[negative] -= 1 << 24
            samples = samples.astype(np.float32) / 8388608.0
        elif sample_width == 4:
            samples = np.frombuffer(raw, dtype="<i4").astype(np.float32) / 2147483648.0
        else:
            raise ValueError(f"不支持的 WAV 采样位宽：{sample_width * 8} bit")

        if len(samples) % channels:
            samples = samples[: len(samples) - (len(samples) % channels)]
        if channels > 1:
            samples = samples.reshape(-1, channels).mean(axis=1)
        return sample_rate, samples

    @staticmethod
    def _estimate_pwmsong_frequency(frame, sample_rate: int, np):
        """Estimate the fundamental frequency of one frame using normalized autocorrelation."""
        centered = frame - np.mean(frame)
        window = np.hanning(len(centered))
        centered = centered * window
        energy = float(np.dot(centered, centered))
        if energy <= 1e-8:
            return None, 0.0

        minimum_lag = max(2, int(sample_rate / PWM_SONG_MAX_FREQ))
        maximum_lag = min(len(centered) - 2, int(sample_rate / PWM_SONG_MIN_FREQ))
        if maximum_lag <= minimum_lag:
            return None, 0.0

        # Use FFT autocorrelation to avoid quadratic work for every PWM_SONG_TICK_MS ms frame.
        fft_size = 1 << (2 * len(centered) - 1).bit_length()
        spectrum = np.fft.rfft(centered, n=fft_size)
        correlation = np.fft.irfft(spectrum * np.conjugate(spectrum), n=fft_size)[: len(centered)]
        search = correlation[minimum_lag : maximum_lag + 1]
        peak_lag = minimum_lag + int(np.argmax(search))
        confidence = float(correlation[peak_lag] / energy)
        if confidence < 0.30:
            return None, confidence

        # Use parabolic interpolation around the autocorrelation peak.
        if 1 <= peak_lag < len(correlation) - 1:
            left, peak, right = correlation[peak_lag - 1 : peak_lag + 2]
            denominator = float(left - 2.0 * peak + right)
            if abs(denominator) > 1e-12:
                peak_lag += float(0.5 * (left - right) / denominator)
        if peak_lag <= 0:
            return None, confidence
        return float(sample_rate / peak_lag), confidence

    @classmethod
    def _analyze_pwmsong(cls, samples, sample_rate: int, np):
        """Detect silence, estimate pitch, and quantize frames to twelve-tone equal temperament."""
        if len(samples) == 0:
            raise ValueError("视频中没有可用音频")

        frame_size = max(1, int(round(sample_rate * PWM_SONG_TICK_MS / 1000.0)))
        peak_level = float(np.max(np.abs(samples)))
        silence_threshold = max(0.01, peak_level * 0.06)
        frame_notes = []

        for start in range(0, len(samples), frame_size):
            actual_size = min(frame_size, len(samples) - start)
            frame = samples[start : start + actual_size]
            rms = float(np.sqrt(np.mean(frame * frame))) if actual_size else 0.0
            if rms <= silence_threshold:
                frame_notes.append(0)
                continue

            padded = np.zeros(frame_size, dtype=np.float32)
            padded[:actual_size] = frame
            frequency, confidence = cls._estimate_pwmsong_frequency(padded, sample_rate, np)
            if frequency is None or confidence < 0.30:
                frame_notes.append(0)
                continue

            midi = int(round(69.0 + 12.0 * np.log2(frequency / 440.0)))
            quantized = 440.0 * (2.0 ** ((midi - 69) / 12.0))
            frame_notes.append(
                int(round(quantized))
                if PWM_SONG_MIN_FREQ <= quantized <= PWM_SONG_MAX_FREQ
                else 0
            )

        notes = []
        for frequency in frame_notes:
            if notes and notes[-1][0] == frequency:
                notes[-1][1] += PWM_SONG_TICK_MS
            else:
                notes.append([frequency, PWM_SONG_TICK_MS])
        return [(int(frequency), int(duration)) for frequency, duration in notes]

    @staticmethod
    def _encode_pwmsong(notes) -> bytes:
        """Encode records as frequency(uint16 little-endian) plus duration(uint16 little-endian)."""
        payload = bytearray()
        for frequency, duration in notes:
            frequency = int(frequency)
            duration = int(duration)
            if not 0 <= frequency <= 0xFFFF:
                raise ValueError(f"PWM Song ???? uint16 ???{frequency}")
            if duration <= 0:
                raise ValueError(f"PWM Song ????????{duration}")

            remaining = duration
            while remaining:
                chunk = min(remaining, PWM_SONG_MAX_DURATION_MS)
                payload.extend(struct.pack("<HH", frequency, chunk))
                remaining -= chunk
        return bytes(payload)

    def _convert_video(
        self, source: str, destination: str, params: MediaParams, index: int, log
    ) -> None:
        """Convert one video to MJPEG."""
        ffmpeg = self.ffmpeg_executable()
        input_path = source
        if params.end_time != "0":
            cache = self.paths.wallpaper_cache_dir / (
                Path(source).stem + "_cache" + Path(source).suffix
            )
            cache.unlink(missing_ok=True)
            subprocess.run(
                [
                    ffmpeg,
                    "-y",
                    "-ss",
                    params.start_time,
                    "-to",
                    params.end_time,
                    "-i",
                    source,
                    "-c:v",
                    "copy",
                    str(cache),
                ],
                check=True,
            )
            input_path = str(cache)
        width, height = (params.width, params.height)
        filters = [
            f"fps={params.fps},scale=-1:{height}:flags=lanczos,crop={width}:in_h:(in_w-{width})/2:0",
            f"fps={params.fps},scale={width}:-1:flags=lanczos,crop=in_w:{height}:0:(in_h-{height})/2",
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
                    params.quality[index],
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
        """Convert one image to the requested resolution and format."""
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
        self, params: MediaParams, crop_to_fill: bool, log: Callable[[str], None]
    ) -> Path:
        """Convert the selected image or BIN asset to a flashable RGB565 background file."""
        self.ensure_directories()
        selected_index = next(
            (index for (index, fmt) in enumerate(params.format) if fmt in {"jpeg", "bin"}), None
        )
        if selected_index is None:
            raise ValueError("参数出错：只有选中的第一张图片或 BIN 文件会生效")
        source = Path(params.src_path[selected_index])
        width, height = (params.width, params.height)
        if params.format[selected_index] == "bin":
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
        return output

    @staticmethod
    def _convert_background_lvgl9(source: Path, output: Path) -> None:
        """Convert a PNG to an uncompressed LVGL 9 RGB565_SWAPPED BIN file."""
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
        """Validate an LVGL 9 background BIN against the firmware requirements."""
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

    def pack_wallpaper(self, params: MediaParams, log: Callable[[str], None]) -> Path:
        """Pack converted wallpaper assets into the device wallpaper format."""
        wallpapers = params.dst_path
        fps = int(params.fps)
        # The first 580 bytes are the wallpaper directory area.
        entries = []
        offset = 580
        for index, wallpaper in enumerate(wallpapers):
            suffix = Path(wallpaper).suffix.lstrip(".").lower()
            size = Path(wallpaper).stat().st_size
            if suffix == "jpeg" and size >= 20000:
                log(f"此图片文件过大（已忽略）：{params.src_path[index]}")
                continue
            entries.append((MEDIA_TYPES.get(suffix, 126), offset, size, wallpaper))
            offset += size
        # RTTTL uses type 125 for compatibility with the legacy Qt format.
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
        """Validate file size against the target capacity and return usage percentage."""
        size = os.path.getsize(str(path))
        if size > capacity:
            raise ValueError("壁纸数据过大，请适当降低帧率或截取更短的时间")
        return int(size / capacity * 100) if capacity else 0
