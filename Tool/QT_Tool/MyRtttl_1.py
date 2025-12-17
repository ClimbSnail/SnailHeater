# codeing: utf-8 -*-
# pip install librosa pydub numpy ffmpeg-python -i https://mirrors.aliyun.com/pypi/simple/

import os
import subprocess
import librosa
import numpy as np
from typing import List, Tuple

class MP4ToRTTTLConverter:
    def __init__(self, ffmpeg_path: str = "ffmpeg"):
        """
        初始化转换器
        :param ffmpeg_path: FFmpeg可执行文件路径（默认系统PATH中的ffmpeg）
        """
        # self.ffmpeg_path = ffmpeg_path
        self.note_mapping = {
            # 音高频率（Hz）到RTTTL音符的映射（基于十二平均律，440Hz=A5）
            261.63: "c5", 277.18: "c#5", 293.66: "d5", 311.13: "d#5",
            329.63: "e5", 349.23: "f5", 369.99: "f#5", 392.00: "g5",
            415.30: "g#5", 440.00: "a5", 466.16: "a#5", 493.88: "b5",
            523.25: "c6", 554.37: "c#6", 587.33: "d6", 622.25: "d#6",
            659.25: "e6", 698.46: "f6", 739.99: "f#6", 783.99: "g6",
            830.61: "g#6", 880.00: "a6", 932.33: "a#6", 987.77: "b6",
            246.94: "b4", 233.08: "a#4", 220.00: "a4", 207.65: "g#4",
            196.00: "g4", 185.00: "f#4", 174.61: "f4", 164.81: "e4",
            155.56: "d#4", 146.83: "d4", 138.59: "c#4", 130.81: "c4"
        }
        self.rtttl_defaults = {
            "duration": 8,    # 默认八分音符
            "octave": 5,      # 默认5八度
            "bpm": 120        # 默认每分钟120拍
        }

    def extract_audio_from_mp4(self, mp4_path: str, output_wav: str = "temp_audio.wav") -> str:
        """
        用FFmpeg提取MP4中的音频为WAV格式
        :param mp4_path: 输入MP4文件路径
        :param output_wav: 输出WAV文件路径
        :return: WAV文件路径
        """
        if not os.path.exists(mp4_path):
            raise FileNotFoundError(f"MP4文件不存在：{mp4_path}")
        
        # FFmpeg命令：提取音频、单声道、降采样到22050Hz（简化分析）
        # cmd = [
        #     self.ffmpeg_path, "-i", mp4_path,
        #     "-vn",               # 忽略视频
        #     "-ac", "1",          # 单声道
        #     "-ar", "22050",      # 采样率22050Hz
        #     "-y",                # 覆盖输出文件
        #     output_wav
        # ]

        cmd = 'ffmpeg -i "%s" -vn -ac 1 -ar 22050 -y "%s"'
        
        try:
            # subprocess.run(cmd, check=True, capture_output=True, text=True) # 编码依靠系统
            print(cmd % (mp4_path, output_wav))
            os.system(cmd % (mp4_path, output_wav))
            print(f"音频提取完成：{output_wav}")
            return output_wav
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"FFmpeg提取音频失败：{e.stderr}")

    def extract_notes(self, wav_path: str) -> List[Tuple[str, int]]:
        """
        从WAV音频中提取音符和时长（音拍）
        :param wav_path: WAV文件路径
        :return: 列表，每个元素为(音符, 音拍)，如("a5", 8)
        """
        # 加载音频
        y, sr = librosa.load(wav_path, sr=22050)
        
        # 提取节拍（BPM）
        tempo, beat_frames = librosa.beat.beat_track(y=y, sr=sr)
        self.rtttl_defaults["bpm"] = int(tempo)
        print(f"检测到BPM：{self.rtttl_defaults['bpm']}")
        
        # 提取基频（音高）- 使用pyin算法
        f0, voiced_flag, voiced_probs = librosa.pyin(
            y, fmin=librosa.note_to_hz('C4'), fmax=librosa.note_to_hz('B6')
        )
        
        # 按节拍分割音频，计算每个节拍的主音高
        beat_times = librosa.frames_to_time(beat_frames, sr=sr)
        note_list = []
        
        # 遍历每个节拍区间
        for i in range(len(beat_times) - 1):
            start_time = beat_times[i]
            end_time = beat_times[i + 1]
            
            # 计算该区间的帧范围
            start_frame = librosa.time_to_frames(start_time, sr=sr)
            end_frame = librosa.time_to_frames(end_time, sr=sr)
            
            # 提取该区间的有效基频
            f0_segment = f0[start_frame:end_frame]
            valid_f0 = f0_segment[np.isfinite(f0_segment)]
            
            if len(valid_f0) == 0:
                # 无有效音高，视为停顿（p）
                note = "p"
            else:
                # 取平均基频，匹配最接近的音符
                avg_f0 = np.mean(valid_f0)
                closest_freq = min(self.note_mapping.keys(), key=lambda x: abs(x - avg_f0))
                note = self.note_mapping[closest_freq]
            
            # 计算音拍（默认八分音符，可根据节拍长度调整）
            duration = self.rtttl_defaults["duration"]
            note_list.append((note, duration))
        
        return note_list

    def generate_rtttl(self, note_list: List[Tuple[str, int]], ringtone_name: str = "Converted") -> str:
        """
        将音符列表转换为RTTTL格式字符串
        :param note_list: 音符列表，元素为(音符, 音拍)
        :param ringtone_name: 铃声名称（<10字符）
        :return: RTTTL格式字符串
        """
        # 处理名称（限制长度）
        ringtone_name = ringtone_name[:10]
        
        # 构建默认参数部分
        defaults = (
            f"d={self.rtttl_defaults['duration']},"
            f"o={self.rtttl_defaults['octave']},"
            f"b={self.rtttl_defaults['bpm']}"
        )
        
        # 构建音符序列
        rtttl_notes = []
        for note, duration in note_list:
            # RTTTL格式：[音拍][音符][音阶]，如8a5 → 可简化为8a（默认音阶）
            if note == "p":
                # 停顿仅保留音拍+p
                rtttl_note = f"{duration}p"
            else:
                # 拆分音符和音阶（如a5 → 音符a，音阶5）
                note_char = note[:-1] if note[-1].isdigit() else note
                octave = note[-1] if note[-1].isdigit() else str(self.rtttl_defaults['octave'])
                
                # 若音阶与默认一致，省略音阶
                if octave == str(self.rtttl_defaults['octave']):
                    rtttl_note = f"{duration}{note_char}"
                else:
                    rtttl_note = f"{duration}{note_char}{octave}"
            
            rtttl_notes.append(rtttl_note)
        
        # 拼接完整RTTTL
        rtttl = f"{ringtone_name}:{defaults}:{','.join(rtttl_notes)}"
        return rtttl

    def convert(self, mp4_path: str, output_rtttl: str, ringtone_name: str = "Converted"):
        """
        完整转换流程：MP4 → WAV → 音符提取 → RTTTL
        :param mp4_path: 输入MP4路径
        :param output_rtttl: 输出RTTTL文件路径
        :param ringtone_name: 铃声名称
        """
        # 1. 提取音频
        wav_path = "temp_audio.wav"
        self.extract_audio_from_mp4(mp4_path, wav_path)
        
        # 2. 提取音符
        print("开始分析音频音高和节拍...")
        note_list = self.extract_notes(wav_path)
        
        # 3. 生成RTTTL
        rtttl_str = self.generate_rtttl(note_list, ringtone_name)
        
        # 4. 保存RTTTL文件
        with open(output_rtttl, "w", encoding="utf-8") as f:
            f.write(rtttl_str)
        
        # 清理临时文件
        if os.path.exists(wav_path):
            os.remove(wav_path)
        
        print(f"转换完成！RTTTL文件已保存至：{output_rtttl}")
        print(f"RTTTL内容：\n{rtttl_str}")

# ------------------- 示例使用 -------------------
if __name__ == "__main__":
    # 初始化转换器（若FFmpeg不在PATH中，需指定路径，如：r"C:\ffmpeg\bin\ffmpeg.exe"）
    rtttlConverter = MP4ToRTTTLConverter(ffmpeg_path="ffmpeg")
    
    # 配置输入输出路径
    INPUT_MP4 = "./RTTTL/3000万年前的迪迦童年珍贵片段.mp4"    # 替换为你的MP4文件路径
    filename = os.path.basename(INPUT_MP4).split(".")[0]
    filePath = os.path.dirname(INPUT_MP4)
    OUTPUT_RTTTL = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件
    
    try:
        # 执行转换
        rtttlConverter.convert(
            mp4_path=INPUT_MP4,
            output_rtttl=OUTPUT_RTTTL,
            ringtone_name=filename[0:14]
        )
    except Exception as e:
        print(f"转换失败：{str(e)}")