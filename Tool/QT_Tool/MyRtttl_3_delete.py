# coding: utf-8
import librosa
import numpy as np
import os
import warnings
warnings.filterwarnings('ignore')


class MP4ToRTTTLConverter:
    def __init__(self):
        pass

    def extract_audio_from_mp4(self, mp4_path: str, output_wav: str = "temp_audio.wav") -> str:
        """改进的音频提取"""
        if not os.path.exists(mp4_path):
            raise FileNotFoundError(f"MP4文件不存在：{mp4_path}")

        cmd = 'ffmpeg -i "%s" -vn -ac 1 -ar 44100 -acodec pcm_s16le -af highpass=f=80,lowpass=f=4000 -y "%s"'
        
        try:
            print(f"正在提取音频...")
            os.system(cmd % (mp4_path, output_wav))
            # 检查文件是否生成成功
            if os.path.exists(output_wav) and os.path.getsize(output_wav) > 0:
                print(f"音频提取完成：{output_wav}")
                # 获取音频时长
                y, sr = librosa.load(output_wav, sr=None, mono=True)
                duration = len(y) / sr
                print(f"音频时长: {duration:.2f}秒")
                return output_wav
            else:
                raise Exception("音频提取失败")
        except Exception as e:
            print(f"转换失败：{str(e)}")
            return None

    def detect_bpm_more_accurate(self, y, sr):
        """使用多种方法检测BPM"""
        print("正在检测BPM...")
        
        # 方法1：librosa的节拍检测
        tempo1, _ = librosa.beat.beat_track(y=y, sr=sr, units='time')
        
        # 方法2：频谱分析
        try:
            onset_env = librosa.onset.onset_strength(y=y, sr=sr)
            tempo2, _ = librosa.beat.beat_track(onset_envelope=onset_env, sr=sr)
        except:
            tempo2 = tempo1
        
        # 如果两个结果都有效，取平均值
        if tempo1 > 0 and tempo2 > 0:
            bpm = int(np.mean([tempo1, tempo2]))
        else:
            bpm = int(max(tempo1, tempo2))
        
        # 常见BPM列表
        common_bpms = [60, 63, 66, 69, 72, 76, 80, 84, 88, 92, 96, 100, 
                       104, 108, 112, 116, 120, 126, 132, 138, 144, 152, 
                       160, 168, 176, 184, 192, 200]
        
        # 如果检测到的BPM与常见BPM差距不大，使用最接近的常见BPM
        if bpm > 0:
            closest = min(common_bpms, key=lambda x: abs(x - bpm))
            if abs(closest - bpm) <= 12:  # 如果差值在12BPM以内，使用常见值
                bpm = closest
        
        # print(f"检测BPM: {tempo1:.1f}, {tempo2:.1f}, 最终使用: {bpm}")
        print(f"检测BPM: {float(tempo1):.1f}, {float(tempo2):.1f}, 最终使用: {bpm}")
        return bpm

    def quantize_duration(self, duration_beats):
        """将时长量化为标准音乐时值"""
        # 将节拍数转换为16分音符数
        sixteenth_notes = duration_beats * 4
        
        # 标准时值（以16分音符为单位）及其容许误差
        standard_durations = {
            1: (0.5, 1.5),    # 16分音符
            2: (1.5, 2.5),    # 8分音符
            3: (2.5, 3.5),    # 附点8分
            4: (3.5, 4.5),    # 4分音符
            6: (5.5, 6.5),    # 附点4分
            8: (7.5, 9.5),    # 2分音符
            12: (11, 13),     # 附点2分
            16: (15, 17)      # 全音符
        }
        
        # 找到最接近的标准时值
        for duration, (min_val, max_val) in standard_durations.items():
            if min_val <= sixteenth_notes <= max_val:
                return duration
        
        # 如果没有匹配，四舍五入到最接近的标准值
        standard_values = list(standard_durations.keys())
        closest = min(standard_values, key=lambda x: abs(x - sixteenth_notes))
        return closest

    def quantize_notes_improved(self, times, notes, bpm):
        """改进的量化算法"""
        if bpm <= 0:
            bpm = 120  # 默认BPM
        
        beat_duration = 60.0 / bpm  # 每拍秒数
        quantized = []
        
        # 计算每个音符的持续时间
        for i in range(len(times) - 1):
            note = notes[i]
            duration_sec = times[i + 1] - times[i]
            
            # 确保最小持续时间（避免过短的音符）
            if duration_sec < 0.01:  # 小于10ms的音符跳过
                continue
            
            # 转换为节拍数
            duration_beats = duration_sec / beat_duration
            
            # 量化时值
            duration_16th = self.quantize_duration(duration_beats)
            
            # 限制时值范围
            duration_16th = max(1, min(16, duration_16th))
            
            quantized.append((note, duration_16th))
        
        return quantized

    def frequency_to_note(self, freq):
        """将频率转换为音符名称"""
        if np.isnan(freq) or freq <= 0:
            return 'p'
        
        # 使用A4=440Hz作为参考
        A4 = 440
        C0 = A4 * (2 ** (-4.75))
        
        # 计算半音数
        h = round(12 * np.log2(freq / C0))
        
        # 音符名称
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
        
        octave = h // 12
        note_index = h % 12
        
        # 确保八度在合理范围内（2-7）
        octave = max(2, min(7, octave))
        
        return f"{note_names[note_index]}{octave}"

    def generate_rtttl(self, rtttl_title, notes, bpm, default_duration=4, default_octave=5):
        """生成RTTTL字符串"""
        # 设置部分
        settings = f"d={default_duration},o={default_octave},b={bpm}"
        
        # 音符序列
        note_sequence = []
        
        # 时值映射：16分音符数 -> RTTTL时值
        duration_map = {
            1: 16,   # 16分音符
            2: 8,    # 8分音符
            3: 8,    # 附点8分（近似）
            4: 4,    # 4分音符
            6: 4,    # 附点4分（近似）
            8: 2,    # 2分音符
            12: 2,   # 附点2分（近似）
            16: 1    # 全音符
        }
        
        for note, duration_16th in notes:
            if duration_16th not in duration_map:
                # 找到最接近的时值
                closest = min(duration_map.keys(), key=lambda x: abs(x - duration_16th))
                duration_16th = closest
            
            rtttl_duration = duration_map.get(duration_16th, 4)
            
            if note == 'p':
                note_sequence.append(f"{rtttl_duration}p")
            else:
                # 提取八度和音符名
                if note[-1].isdigit():
                    octave = int(note[-1])
                    note_name = note[:-1]
                else:
                    octave = default_octave
                    note_name = note
                
                # 如果八度与默认不同，需要指定
                if octave != default_octave:
                    note_sequence.append(f"{rtttl_duration}{note_name}{octave}")
                else:
                    note_sequence.append(f"{rtttl_duration}{note_name}")
        
        note_str = ",".join(note_sequence)
        
        # 限制RTTTL字符串长度（避免过长）
        if len(note_str) > 200:
            note_str = note_str[:200] + "..."
            print("警告：RTTTL序列过长，已截断")
        
        return f"{rtttl_title}:{settings}:{note_str}"

    def wav_to_rtttl(self, wav_path, output_rtttl_path, rtttl_title,
                     target_bpm=None, default_duration=4, default_octave=5,
                     time_correction=1.0):  # 默认增加25%的BPM（缩短20%时长）
        """将WAV文件转换为RTTTL格式"""

        print(f"加载音频文件: {wav_path}")
        y, sr = librosa.load(wav_path, sr=None, mono=True)

        # 检测BPM
        if target_bpm is None:
            detected_bpm = self.detect_bpm_more_accurate(y, sr)
            if detected_bpm <= 0:
                detected_bpm = 120
        else:
            detected_bpm = target_bpm

        print(f"原始检测BPM: {detected_bpm}")

        # 应用时间校正（增加BPM可以缩短播放时间）
        corrected_bpm = int(detected_bpm * time_correction)
        print(f"校正后BPM: {corrected_bpm} (校正因子: {time_correction})")

        # 提取音高
        print("提取音高...")
        f0, voiced_flag, voiced_probs = librosa.pyin(
            y,
            fmin=librosa.note_to_hz('C2'),
            fmax=librosa.note_to_hz('C7'),
            sr=sr,
            frame_length=2048,
            hop_length=512,
            fill_na=0
        )

        # 将频率转换为音符
        notes = []
        times = librosa.times_like(f0, sr=sr)

        print(f"共检测到 {len(f0)} 个音频帧")

        # 简化音符序列：避免过于频繁的音符变化
        prev_note = None
        min_note_duration = 0.1  # 最小音符持续时间（秒）
        note_buffer = []
        time_buffer = []
        times_processed = []  # 初始化这个列表！

        for i, freq in enumerate(f0):
            if i < len(times):
                current_time = times[i]

                if np.isnan(freq) or freq <= 0:
                    current_note = 'p'
                else:
                    current_note = self.frequency_to_note(freq)

                # 如果是第一个音符或音符发生变化
                if prev_note is None or current_note != prev_note:
                    # 如果有缓冲的音符，处理它们
                    if note_buffer:
                        # 使用最频繁的音符
                        unique_notes, counts = np.unique(note_buffer, return_counts=True)
                        dominant_note = unique_notes[np.argmax(counts)]

                        # 计算平均时间
                        avg_time = np.mean(time_buffer)

                        notes.append(dominant_note)
                        times_processed.append(avg_time)  # 直接添加到列表

                        # 重置缓冲区
                        note_buffer = []
                        time_buffer = []

                # 添加到缓冲区
                note_buffer.append(current_note)
                time_buffer.append(current_time)
                prev_note = current_note

        # 处理最后一个缓冲区
        if note_buffer:
            unique_notes, counts = np.unique(note_buffer, return_counts=True)
            dominant_note = unique_notes[np.argmax(counts)]
            avg_time = np.mean(time_buffer)
            notes.append(dominant_note)
            times_processed.append(avg_time)

        print(f"简化后音符数量: {len(notes)}")

        # 量化音符时值
        print("量化音符时值...")
        # 注意：这里传入times_processed而不是原始的times
        quantized_notes = self.quantize_notes_improved(times_processed, notes, corrected_bpm)

        # 进一步简化：合并相同音符
        simplified_notes = []
        current_note = None
        current_duration = 0
        
        for note, duration in quantized_notes:
            if note == current_note:
                # 合并相同音符
                current_duration = min(16, current_duration + duration)
            else:
                if current_note is not None:
                    simplified_notes.append((current_note, current_duration))
                current_note = note
                current_duration = duration
        
        if current_note is not None:
            simplified_notes.append((current_note, current_duration))
        
        print(f"最终音符数量: {len(simplified_notes)}")
        
        # 生成RTTTL格式
        rtttl_str = self.generate_rtttl(rtttl_title, simplified_notes, corrected_bpm,
                                       default_duration, default_octave)
        
        # 保存到文件
        if output_rtttl_path:
            with open(output_rtttl_path, 'w', encoding='utf-8') as f:
                f.write(rtttl_str)
            print(f"RTTTL文件已保存: {output_rtttl_path}")
        
        # 打印RTTTL预览
        print("\n" + "="*50)
        print("生成的RTTTL预览:")
        print(rtttl_str[:200] + "..." if len(rtttl_str) > 200 else rtttl_str)
        print("="*50)
        
        return rtttl_str

    def convert(self, mp4_path, output_rtttl, rtttl_title="TestTune", 
                time_correction=1.0):
        """主转换函数"""
        
        print("="*50)
        print(f"开始转换: {mp4_path}")
        print("="*50)
        
        # 准备输出路径
        filename = os.path.basename(output_rtttl).split(".")[0]
        filePath = os.path.dirname(output_rtttl)
        if filePath == "":
            filePath = "."
        
        output_wav = f"{filePath}/{filename}.wav"
        
        # 提取音频
        wav_path = self.extract_audio_from_mp4(mp4_path, output_wav)
        if wav_path is None:
            print("音频提取失败，停止转换")
            return None
        
        # 转换为RTTTL
        print(f"\n开始转换为RTTTL...")
        try:
            rtttl_str = self.wav_to_rtttl(
                wav_path, 
                output_rtttl, 
                rtttl_title,
                time_correction=time_correction
            )
            
            # 计算估计播放时长
            if "b=" in rtttl_str:
                bpm_start = rtttl_str.find("b=") + 2
                bpm_end = rtttl_str.find(":", bpm_start)
                bpm = int(rtttl_str[bpm_start:bpm_end])
                
                # 简单估计：假设平均每个音符是4分音符
                avg_beats_per_note = 1.0  # 4分音符 = 1拍
                estimated_beats = len(rtttl_str.split(":")) * avg_beats_per_note
                estimated_seconds = (estimated_beats * 60) / bpm
                
                print(f"\n转换完成！")
                print(f"估计播放时长: {estimated_seconds:.1f}秒")
                print(f"使用BPM: {bpm}")
                print(f"时间校正因子: {time_correction}")
            
            return rtttl_str
            
        except Exception as e:
            print(f"转换失败: {str(e)}")
            import traceback
            traceback.print_exc()
            return None


def test_different_corrections():
    """测试不同的时间校正因子"""
    converter = MP4ToRTTTLConverter()
    
    # 测试不同的校正因子
    mp4_path = "./RTTTL/3000万年前的迪迦童年珍贵片段.mp4"
    
    if not os.path.exists(mp4_path):
        print(f"测试文件不存在: {mp4_path}")
        print("请使用其他MP4文件进行测试")
        return
    
    corrections = [1.0, 1.1, 1.2, 1.25, 1.3, 1.4]
    
    for correction in corrections:
        print(f"\n{'='*60}")
        print(f"测试校正因子: {correction}")
        print(f"{'='*60}")
        
        output_file = f"./RTTTL/test_correction_{correction}.rtttl"
        
        rtttl = converter.convert(
            mp4_path=mp4_path,
            output_rtttl=output_file,
            rtttl_title=f"TestCorrection{correction}",
            time_correction=correction
        )
        
        if rtttl:
            print(f"生成文件: {output_file}")


# 使用示例
if __name__ == "__main__":
    # 创建转换器实例
    converter = MP4ToRTTTLConverter()
    
    # 单个文件转换（推荐使用校正因子1.25）
    input_mp4 = "./RTTTL/3000万年前的迪迦童年珍贵片段.mp4"
    # 准备输出路径
    filename = os.path.basename(input_mp4).split(".")[0]
    filePath = os.path.dirname(input_mp4)
    if filePath == "":
        filePath = "."
    
    output_rtttl = f"{filePath}/{filename}.rtttl"
    
    # 执行转换，使用校正因子1.25（缩短20%时长）
    rtttl_result = converter.convert(
        mp4_path=input_mp4,
        output_rtttl=output_rtttl,
        rtttl_title=filename[:10],  # 限制标题长度
        time_correction=1.0  # 关键参数：增加25%的BPM
    )
    
    if rtttl_result:
        print("\n" + "="*60)
        print("转换成功！")
        print(f"输入文件: {input_mp4}")
        print(f"输出文件: {output_rtttl}")
        print("="*60)
        
        # 询问是否测试不同校正因子
        print("\n是否测试不同的校正因子？(y/n)")
        user_input = input().strip().lower()
        if user_input == 'y' or user_input == 'yes':
            test_different_corrections()
    else:
        print("转换失败")