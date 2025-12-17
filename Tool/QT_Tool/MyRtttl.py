# codeing: utf-8 -*-
import librosa
import numpy as np
import os

# pip install librosa numpy soundfile -i https://mirrors.aliyun.com/pypi/simple/

class MP4ToRTTTLConverter:
    def __init__(self):
        pass

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
        except Exception as e:
            print(f"转换失败：{str(e)}")

    def wav_to_rtttl(self, wav_path, output_rtttl_path, rtttl_title,
                     bpm=120, default_duration=4, default_octave=4):
        """
        将WAV文件转换为RTTTL格式
        注意：这是一个简化的近似转换，无法完美识别复杂音频
        """

        # 1. 加载音频
        # y, sr = librosa.load(wav_path, sr=44100, mono=True)
        y, sr = librosa.load(wav_path, sr=None, mono=True)

        # 2. 提取音高（基频检测）
        f0, voiced_flag, voiced_probs = librosa.pyin(
            y,
            fmin=librosa.note_to_hz('C2'),
            fmax=librosa.note_to_hz('C7'),
            sr=sr
        )

        # 检测BPM
        tempo, beat_frames = librosa.beat.beat_track(y=y, sr=sr)
        bpm = int(tempo)
        print(f"检测到BPM: {bpm}")

        # 3. 将频率转换为音符
        notes = []
        times = librosa.times_like(f0, sr=sr)

        for i, freq in enumerate(f0):
            if np.isnan(freq):
                notes.append('p')  # 休止符
            else:
                # 频率转音符名
                note_name = self.frequency_to_note(freq)
                notes.append(note_name)

        # 4. 量化时值（简化版）
        quantized_notes = self.quantize_notes(times, notes, bpm)

        # 5. 生成RTTTL格式
        rtttl_str = self.generate_rtttl(rtttl_title, quantized_notes, bpm,
                                   default_duration, default_octave)

        # 6. 保存或返回
        if output_rtttl_path:
            with open(output_rtttl_path, 'w') as f:
                f.write(rtttl_str)

        return rtttl_str

    def frequency_to_note(self, freq):
        """将频率转换为音符名称"""
        # 使用A4=440Hz作为参考
        A4 = 440
        C0 = A4 * (2 ** (-4.75))

        if freq <= 0:
            return 'p'

        # 计算半音数
        h = round(12 * np.log2(freq / C0))

        # 音符名称
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

        octave = h // 12
        note_index = h % 12

        # 确保八度在合理范围内
        octave = max(0, min(8, octave))

        return f"{note_names[note_index]}{octave}"

    def quantize_notes(self, times, notes, bpm):
        """量化音符时值"""
        beat_duration = 60 / bpm  # 每拍秒数
        quantized = []

        current_note = None
        start_time = 0
        duration_count = 0

        for i, (time, note) in enumerate(zip(times, notes)):
            if note != current_note or i == len(notes) - 1:
                if current_note is not None:
                    # 计算时值（以16分音符为单位）
                    duration_beats = (time - start_time) / beat_duration
                    duration_16th = round(duration_beats * 4)  # 4个16分音符=1拍

                    # 限制在常见时值
                    duration_16th = max(1, min(16, duration_16th))
                    quantized.append((current_note, duration_16th))

                current_note = note
                start_time = time

        return quantized

    def generate_rtttl(self, rtttl_title, notes, bpm, default_duration, default_octave):
        """生成RTTTL字符串"""

        # 设置部分
        settings = f"d={default_duration},o={default_octave},b={bpm}"

        # 音符序列
        note_sequence = []

        for note, duration_16th in notes:
            if note == 'p':
                note_sequence.append(f"{duration_16th}p")
            else:
                # 提取八度
                octave = int(note[-1])
                note_name = note[:-1]

                # 16分音符时值映射
                duration_map = {
                    1: 16,  # 16分音符
                    2: 8,  # 8分音符
                    3: 8,  # 附点8分（近似）
                    4: 4,  # 4分音符
                    6: 4,  # 附点4分（近似）
                    8: 2,  # 2分音符
                    12: 2,  # 附点2分（近似）
                    16: 1  # 全音符
                }

                duration = duration_map.get(duration_16th, 4)

                # 如果八度与默认不同，需要指定
                if octave != default_octave:
                    note_sequence.append(f"{duration}{note_name}{octave}")
                else:
                    note_sequence.append(f"{duration}{note_name}")

        note_str = ",".join(note_sequence)

        return f"{rtttl_title}:{settings}:{note_str}"

    def convert(self, mp4_path,
                output_rtttl,
                rtttl_title="TestTune",
                max_notes=40000  # 限制音符数量
                ):
        filename = os.path.basename(output_rtttl).split(".")[0]
        filePath = os.path.dirname(output_rtttl)
        output_wav = f"{filePath}/{filename}.wav"  # 输出RTTTL文件
        self.extract_audio_from_mp4(mp4_path, output_wav)
        self.wav_to_rtttl(output_wav, output_rtttl, rtttl_title)

# 使用示例s
if __name__ == "__main__":
    # 测试转换
    converter = MP4ToRTTTLConverter()
    # input_mp4 = "D:\Workspace\OpenWorkspace\SnailHeater\Tool\视频素材\养眼美女/看她摇好爽啊！【雨宝摇~】-480P 清晰-AVC.mp4"  # 你的MP4文件
    input_mp4 = "./RTTTL/3000万年前的迪迦童年珍贵片段.mp4"  # 你的MP4文件
    filename = os.path.basename(input_mp4).split(".")[0]
    filePath = os.path.dirname(input_mp4)
    output_rtttl = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件

    converter.convert(
        mp4_path=input_mp4,
        output_rtttl=output_rtttl,
        rtttl_title= "RTitle")
