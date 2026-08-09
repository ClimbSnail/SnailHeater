# -*- coding: utf-8 -*-
"""兼容旧版 MP4→RTTTL 处理流程的音频转换服务。"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path
from typing import Callable, Iterable, Optional, Sequence, Tuple


class MP4ToRTTTLConverter:
    """将 MP4 音轨近似转换为 RTTTL 文本。

    算法先用 FFmpeg 提取 44.1 kHz 单声道 PCM，
    再使用 librosa 的 pYIN 提取基频、自动估算 BPM、按十六分音符量化，最后
    生成 ``标题:d=4,o=4,b=BPM:音符...`` 格式。复杂伴奏无法保证准确识别，
    该工具适合蜂鸣器旋律的近似转换。
    """

    def __init__(
        self,
        ffmpeg_executable: Optional[str] = None,
        log: Optional[Callable[[str], None]] = None,
    ) -> None:
        self.ffmpeg_executable = ffmpeg_executable
        self.log = log or (lambda _message: None)

    @staticmethod
    def _audio_dependencies():
        """延迟加载可选的音频分析依赖，避免缺包时整个工具无法启动。"""
        try:
            import librosa
            import numpy as np
        except ImportError as error:
            raise RuntimeError(
                "缺少 RTTTL 音频转换依赖，请安装 requirements 中的 librosa 和 numpy"
            ) from error
        return librosa, np

    def _ffmpeg(self) -> str:
        """优先使用工具随附的 FFmpeg，其次使用系统 PATH 中的 FFmpeg。"""
        if self.ffmpeg_executable:
            return self.ffmpeg_executable
        executable = shutil.which("ffmpeg")
        if executable:
            return executable
        raise FileNotFoundError("未找到 ffmpeg，无法从 MP4 提取音轨")

    def extract_audio_from_mp4(self, mp4_path: str, output_wav: str = "temp_audio.wav") -> str:
        """按旧版参数提取适合蜂鸣器音高识别的单声道 WAV 文件。"""
        source = Path(mp4_path)
        target = Path(output_wav)
        if source.suffix.lower() != ".mp4":
            raise ValueError("RTTTL 转换只支持 MP4 视频文件")
        if not source.is_file():
            raise FileNotFoundError(f"MP4 文件不存在：{source}")
        target.parent.mkdir(parents=True, exist_ok=True)

        # 旧逻辑使用 44.1 kHz、16 位单声道 PCM，并保留 80–4000 Hz 的蜂鸣器频段。
        command = [
            self._ffmpeg(),
            "-y",
            "-i",
            str(source),
            "-vn",
            "-ac",
            "1",
            "-ar",
            "44100",
            "-acodec",
            "pcm_s16le",
            "-af",
            "highpass=f=80,lowpass=f=4000",
            str(target),
        ]
        self.log("正在从 MP4 提取单声道音轨……")
        try:
            subprocess.run(command, check=True, capture_output=True, text=True, encoding="utf-8")
        except subprocess.CalledProcessError as error:
            detail = (error.stderr or error.stdout or "").strip()
            raise RuntimeError(f"MP4 音频提取失败：{detail or source}") from error
        if not target.is_file() or target.stat().st_size == 0:
            raise RuntimeError(f"音频提取失败，未生成 WAV 文件：{target}")
        self.log(f"音频提取完成：{target}")
        return str(target)

    def wav_to_rtttl(
        self,
        wav_path: str,
        output_rtttl_path: str,
        rtttl_title: str,
        bpm: int = 120,
        default_duration: int = 4,
        default_octave: int = 4,
    ) -> str:
        """将 WAV 转为 RTTTL；音符量化和默认参数"""
        librosa, np = self._audio_dependencies()
        y, sample_rate = librosa.load(wav_path, sr=None, mono=True)
        f0, _voiced_flag, _voiced_probs = librosa.pyin(
            y,
            fmin=librosa.note_to_hz("C2"),
            fmax=librosa.note_to_hz("C7"),
            sr=sample_rate,
        )
        tempo, _beat_frames = librosa.beat.beat_track(y=y, sr=sample_rate)
        # 新版 librosa 可能返回单元素 ndarray；旧版和新版都统一为一个正整数 BPM。
        bpm = max(1, int(float(np.asarray(tempo).reshape(-1)[0])))
        self.log(f"检测到 BPM：{bpm}")

        notes = []
        times = librosa.times_like(f0, sr=sample_rate)
        for frequency in f0:
            notes.append("p" if np.isnan(frequency) else self.frequency_to_note(float(frequency), np))

        quantized_notes = self.quantize_notes(times, notes, bpm)
        rtttl_text = self.generate_rtttl(
            rtttl_title, quantized_notes, bpm, default_duration, default_octave
        )
        output = Path(output_rtttl_path)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(rtttl_text, encoding="utf-8")
        return rtttl_text

    @staticmethod
    def frequency_to_note(frequency: float, np_module) -> str:
        """将频率映射到十二平均律音符，沿用旧版 A4=440 Hz 的计算方式。"""
        if frequency <= 0:
            return "p"
        a4 = 440
        c0 = a4 * (2 ** (-4.75))
        half_steps = round(12 * np_module.log2(frequency / c0))
        note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
        octave = max(0, min(8, half_steps // 12))
        return f"{note_names[half_steps % 12]}{octave}"

    @staticmethod
    def quantize_notes(times: Iterable[float], notes: Sequence[str], bpm: int) -> list[Tuple[str, int]]:
        """将连续的基频帧合并，并以十六分音符为最小时间单位量化。"""
        beat_duration = 60 / bpm
        quantized: list[Tuple[str, int]] = []
        current_note: Optional[str] = None
        start_time = 0.0
        values = list(zip(times, notes))
        for index, (time_value, note) in enumerate(values):
            if note != current_note or index == len(values) - 1:
                if current_note is not None:
                    duration_beats = (float(time_value) - start_time) / beat_duration
                    duration_16th = max(1, min(16, round(duration_beats * 4)))
                    quantized.append((current_note, duration_16th))
                current_note = note
                start_time = float(time_value)
        return quantized

    @staticmethod
    def generate_rtttl(
        rtttl_title: str,
        notes: Iterable[Tuple[str, int]],
        bpm: int,
        default_duration: int,
        default_octave: int,
    ) -> str:
        """按旧版时值映射拼接标准 RTTTL 文本。"""
        duration_map = {
            1: 16,  # 16 分音符
            2: 8,   # 8 分音符
            3: 8,   # 附点 8 分音符（近似）
            4: 4,   # 4 分音符
            6: 4,   # 附点 4 分音符（近似）
            8: 2,   # 2 分音符
            12: 2,  # 附点 2 分音符（近似）
            16: 1,  # 全音符
        }
        rendered_notes = []
        for note, duration_16th in notes:
            if note == "p":
                rendered_notes.append(f"{duration_16th}p")
                continue
            octave = int(note[-1])
            note_name = note[:-1]
            duration = duration_map.get(duration_16th, 4)
            rendered_notes.append(
                f"{duration}{note_name}{octave}" if octave != default_octave else f"{duration}{note_name}"
            )
        settings = f"d={default_duration},o={default_octave},b={bpm}"
        return f"{rtttl_title}:{settings}:{','.join(rendered_notes)}"

    def convert(
        self,
        mp4_path: str,
        output_rtttl: str,
        rtttl_title: str = "TestTune",
        max_notes: int = 40000,
    ) -> str:
        """保留旧版公开签名；中间 WAV 与 RTTTL 文件保存在同一目录。"""
        del max_notes  # 旧实现未实际限制音符数量，保留参数仅为调用兼容。
        output = Path(output_rtttl)
        output_wav = output.with_suffix(".wav")
        self.extract_audio_from_mp4(mp4_path, str(output_wav))
        rtttl_text = self.wav_to_rtttl(str(output_wav), str(output), rtttl_title)
        self.log(f"RTTTL 转换完成：{output}")
        return rtttl_text
