# coding: utf-8
import os
import subprocess
import librosa
import numpy as np
from typing import List, Tuple, Optional


class MP4ToRTTTLConverter:
    def __init__(self, ffmpeg_path: str = "ffmpeg"):
        self.ffmpeg_path = ffmpeg_path

        # 完整的十二平均律频率表（C0到B8）
        self.note_freqs = {}
        notes = ['c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#', 'a', 'a#', 'b']

        # 生成C0到B8的频率（8.18Hz到7902Hz）
        for octave in range(0, 9):
            for i, note in enumerate(notes):
                freq = 440.0 * (2 ** ((octave - 5) + (i - 9) / 12.0))
                if 50 <= freq <= 4000:  # 蜂鸣器有效范围
                    self.note_freqs[freq] = f"{note}{octave}"

        # 常用参数
        self.rtttl_defaults = {
            "duration": 4,  # 默认四分音符
            "octave": 5,  # 默认八度
            "bpm": 120  # 默认BPM
        }

        # 音符时值映射（秒->RTTTL音长）
        self.duration_map = {
            4.0: 1,  # 全音符
            2.0: 2,  # 二分音符
            1.0: 4,  # 四分音符
            0.5: 8,  # 八分音符
            0.25: 16,  # 十六分音符
            0.125: 32,  # 三十二分音符
        }

    def extract_audio_from_mp4(self, mp4_path: str, output_wav: str = "temp_audio.wav") -> str:
        """改进的音频提取"""
        if not os.path.exists(mp4_path):
            raise FileNotFoundError(f"MP4文件不存在：{mp4_path}")

        # # 提高采样率以获得更好音高检测
        # cmd = [
        #     self.ffmpeg_path, "-i", mp4_path,
        #     "-vn", "-ac", "1", "-ar", "44100",  # 44.1kHz采样
        #     "-acodec", "pcm_s16le",  # 16位PCM
        #     "-af", "highpass=f=80,lowpass=f=4000",  # 带通滤波（蜂鸣器范围）
        #     "-y", output_wav
        # ]
        cmd = 'ffmpeg -i "%s" -vn -ac 1 -ar 44100 -acodec pcm_s16le -af highpass=f=80,lowpass=f=4000 -y "%s"'

        try:
            # subprocess.run(cmd, check=True, capture_output=True, text=True)
            print(cmd % (mp4_path, output_wav))
            os.system(cmd % (mp4_path, output_wav))
            print(f"音频提取完成：{output_wav}")
            return output_wav
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"FFmpeg提取音频失败：{e.stderr}")

    def detect_notes_with_fft(self, y: np.ndarray, sr: int, hop_length: int = 512) -> List[Tuple[float, float, float]]:
        """
        使用FFT频谱分析检测音符
        返回: [(起始时间, 持续时间, 频率), ...]
        """
        # 短时傅里叶变换
        n_fft = 2048
        S = np.abs(librosa.stft(y, n_fft=n_fft, hop_length=hop_length))

        # 转换为频率和dB
        freqs = librosa.fft_frequencies(sr=sr, n_fft=n_fft)
        times = librosa.frames_to_time(np.arange(S.shape[1]), sr=sr, hop_length=hop_length)

        # 找到显著峰值（音符）
        notes = []
        threshold = np.percentile(S, 90)  # 前10%作为有效音符

        for i in range(S.shape[1]):
            frame = S[:, i]

            # 找到该帧中最强的频率
            if np.max(frame) > threshold:
                # 使用质心法计算主频率
                centroid = np.sum(freqs * frame) / np.sum(frame)

                # 过滤到有效范围
                if 100 <= centroid <= 2000:
                    # 匹配到最接近的音符频率
                    closest_freq = self._find_closest_note_freq(centroid)
                    notes.append((times[i], hop_length / sr, closest_freq))

        # 合并连续相同频率的音符
        merged_notes = []
        if notes:
            current_start, current_dur, current_freq = notes[0]

            for start, dur, freq in notes[1:]:
                if freq == current_freq and start - (current_start + current_dur) < 0.05:
                    # 合并连续相同频率
                    current_dur += dur
                else:
                    # 保存前一个音符
                    if current_dur > 0.05:  # 至少50ms
                        merged_notes.append((current_start, current_dur, current_freq))
                    # 开始新音符
                    current_start, current_dur, current_freq = start, dur, freq

            # 保存最后一个音符
            if current_dur > 0.05:
                merged_notes.append((current_start, current_dur, current_freq))

        return merged_notes

    def _find_closest_note_freq(self, freq: float) -> float:
        """找到最接近的十二平均律音符频率"""
        # 计算最接近的半音数
        semitone = 12 * np.log2(freq / 440.0) + 69  # MIDI编号
        semitone_rounded = round(semitone)

        # 转换回频率
        closest_freq = 440.0 * (2 ** ((semitone_rounded - 69) / 12.0))
        return closest_freq

    def quantize_duration(self, duration_sec: float, bpm: int) -> int:
        """将秒数量化为RTTTL音长"""
        # 计算一拍的长度（秒）
        beat_duration = 60.0 / bpm

        # 计算相当于多少拍
        beats = duration_sec / beat_duration

        # 映射到最近的音符时值
        if beats >= 3.5:
            return 1  # 全音符
        elif beats >= 1.75:
            return 2  # 二分音符
        elif beats >= 0.875:
            return 4  # 四分音符
        elif beats >= 0.4375:
            return 8  # 八分音符
        elif beats >= 0.21875:
            return 16  # 十六分音符
        else:
            return 32  # 三十二分音符

    def extract_notes_improved(self, wav_path: str) -> List[Tuple[str, int]]:
        """
        改进的音符提取算法
        返回: [(音符, 音长), ...]
        """
        # 加载音频
        # y, sr = librosa.load(wav_path, sr=44100, mono=True)
        y, sr = librosa.load(wav_path, sr=None, mono=True)

        print("y len = ", len(y))
        print("sr len = ", sr)

        # 检测BPM
        tempo, beat_frames = librosa.beat.beat_track(y=y, sr=sr)
        self.rtttl_defaults["bpm"] = int(tempo)
        print(f"检测到BPM: {self.rtttl_defaults['bpm']}")

        # 使用改进的FFT方法检测音符
        raw_notes = self.detect_notes_with_fft(y, sr)

        print("len = ", len(raw_notes))
        print(raw_notes)

        # 转换为RTTTL格式
        rtttl_notes = []
        beat_duration = 60.0 / self.rtttl_defaults["bpm"]

        for start, duration, freq in raw_notes:
            # 1. 匹配音符名称
            if freq < 100:
                note_name = "p"  # 休止符
            else:
                # 找到最接近的音符
                closest_freq = min(self.note_freqs.keys(),
                                   key=lambda x: abs(x - freq))
                note_name = self.note_freqs[closest_freq]

            # 2. 量化时值
            rtttl_duration = self.quantize_duration(duration, self.rtttl_defaults["bpm"])

            rtttl_notes.append((note_name, rtttl_duration))

        return rtttl_notes

    def simplify_melody(self, note_list: List[Tuple[str, int]],
                        max_notes: int = 50) -> List[Tuple[str, int]]:
        """
        简化旋律，减少音符数量，适合蜂鸣器播放
        """
        if len(note_list) <= max_notes:
            return note_list

        # 1. 合并连续相同音符
        simplified = []
        current_note, current_duration = note_list[0]

        for note, duration in note_list[1:]:
            if note == current_note:
                # 合并时值（取平均值）
                current_duration = max(current_duration, duration)
            else:
                simplified.append((current_note, current_duration))
                current_note, current_duration = note, duration

        simplified.append((current_note, current_duration))

        # 2. 如果还是太多，降低采样率
        if len(simplified) > max_notes:
            step = len(simplified) // max_notes + 1
            simplified = simplified[::step]

        return simplified

    def generate_rtttl(self, note_list: List[Tuple[str, int]],
                       rtttl_title: str = "Converted") -> str:
        """
        生成RTTTL字符串
        """
        # 名称处理
        rtttl_title = rtttl_title[:15].replace(":", "_")  # 避免冒号

        # 计算最常用的时值作为默认
        durations = [d for _, d in note_list if d != 0]
        if durations:
            self.rtttl_defaults["duration"] = max(set(durations), key=durations.count)

        # 计算最常用的八度
        octaves = []
        for note, _ in note_list:
            if note != "p" and note[-1].isdigit():
                octaves.append(int(note[-1]))
        if octaves:
            self.rtttl_defaults["octave"] = max(set(octaves), key=octaves.count)

        # 构建RTTTL
        defaults = f"d={self.rtttl_defaults['duration']}," \
                   f"o={self.rtttl_defaults['octave']}," \
                   f"b={self.rtttl_defaults['bpm']}"

        rtttl_notes = []
        for note, duration in note_list:
            if note == "p":
                rtttl_notes.append(f"{duration}p")
            else:
                # 如果八度与默认相同，省略八度
                if note[-1].isdigit() and int(note[-1]) == self.rtttl_defaults["octave"]:
                    note_without_octave = note[:-1]
                    rtttl_notes.append(f"{duration}{note_without_octave}")
                else:
                    rtttl_notes.append(f"{duration}{note}")

        rtttl_str = f"{rtttl_title}:{defaults}:{','.join(rtttl_notes)}"
        return rtttl_str

    def convert(self, mp4_path: str, output_rtttl: str,
                rtttl_title: str = "Converted", max_notes: int = 50):
        """
        完整转换流程
        """
        print(f"开始转换: {mp4_path}")

        # 1. 提取音频
        filename = os.path.basename(output_rtttl).split(".")[0]
        filePath = os.path.dirname(output_rtttl)
        wav_path = f"{filePath}/{filename}.wav"  # 输出RTTTL文件
        self.extract_audio_from_mp4(mp4_path, wav_path)

        # 2. 提取音符
        print("分析音频...")
        note_list = self.extract_notes_improved(wav_path)

        print(f"原始检测到 {len(note_list)} 个音符")

        # 3. 简化旋律
        note_list = self.simplify_melody(note_list, max_notes)
        print(f"简化后 {len(note_list)} 个音符")

        # 4. 生成RTTTL
        rtttl_str = self.generate_rtttl(note_list, rtttl_title)

        # 5. 保存
        with open(output_rtttl, "w", encoding="utf-8") as f:
            f.write(rtttl_str)

        # 清理
        if os.path.exists(wav_path):
            pass
            # os.remove(wav_path)

        print(f"转换完成！保存到: {output_rtttl}")
        print("=" * 50)
        print(rtttl_str[:200] + "..." if len(rtttl_str) > 200 else rtttl_str)
        print("=" * 50)

        return rtttl_str


# 使用示例
if __name__ == "__main__":
    converter = MP4ToRTTTLConverter()

    # 测试转换
    # input_mp4 = "D:\Workspace\OpenWorkspace\SnailHeater\Tool\视频素材\养眼美女/看她摇好爽啊！【雨宝摇~】-480P 清晰-AVC.mp4"  # 你的MP4文件
    input_mp4 = "./RTTTL/3000万年前的迪迦童年珍贵片段.mp4"  # 你的MP4文件
    filename = os.path.basename(input_mp4).split(".")[0]
    filePath = os.path.dirname(input_mp4)
    output_rtttl = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件

    if os.path.exists(input_mp4):
        try:
            result = converter.convert(
                mp4_path=input_mp4,
                output_rtttl=output_rtttl,
                rtttl_title="TestTune",
                max_notes=40000  # 限制音符数量
            )
        except Exception as e:
            print(f"转换失败: {e}")
    else:
        print(f"文件不存在: {input_mp4}")