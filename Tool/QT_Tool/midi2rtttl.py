# 安装依赖
# pip install mido python-rtmidi -i https://mirrors.aliyun.com/pypi/simple/
# 完整版MIDI转RTTTL转换器
# 保存为 midi2rtttl.py

import mido
import os
import argparse
from collections import defaultdict


class MidiToRtttl:
    def __init__(self):
        self.notes = []
        self.tempo = 500000  # 默认tempo (120 BPM)
        self.time_signature = (4, 4)

    def parse_midi(self, midi_path):
        """解析MIDI文件"""
        mid = mido.MidiFile(midi_path)

        # 查找全局信息
        for track in mid.tracks:
            for msg in track:
                if msg.type == 'set_tempo':
                    self.tempo = msg.tempo
                elif msg.type == 'time_signature':
                    self.time_signature = (msg.numerator, msg.denominator)

        # 简化的音符提取（取第一个音轨的主旋律）
        active_notes = {}
        current_time = 0

        for msg in mid.tracks[0]:
            current_time += msg.time

            if msg.type == 'note_on' and msg.velocity > 0:
                # 计算时长（简化）
                duration = self.calculate_duration(msg.time)
                note_name = self.midi_to_rtttl_note(msg.note, duration)
                if note_name:
                    self.notes.append(note_name)

    def calculate_duration(self, delta_time):
        """计算RTTTL音符时长"""
        # 简化的时长计算
        ticks_per_beat = 480  # 标准MIDI ticks
        beat_duration = ticks_per_beat / 4  # 4分音符

        if delta_time < beat_duration * 0.3:
            return 16  # 16分音符
        elif delta_time < beat_duration * 0.6:
            return 8  # 8分音符
        elif delta_time < beat_duration * 1.2:
            return 4  # 4分音符
        else:
            return 2  # 2分音符

    def midi_to_rtttl_note(self, midi_note, duration):
        """MIDI音符转RTTTL格式"""
        if midi_note < 24 or midi_note > 96:  # 限制音域
            return None

        notes = ['c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#', 'a', 'a#', 'b']
        octave = (midi_note // 12) - 1
        note_index = midi_note % 12

        # RTTTL格式：音符+可选时长
        if duration != 4:  # 如果不是默认4分音符
            return f"{duration}{notes[note_index]}{octave}"
        else:
            return f"{notes[note_index]}{octave}"

    def generate_rtttl(self, song_name):
        """生成RTTTL字符串"""
        bpm = int(mido.tempo2bpm(self.tempo))

        # RTTTL格式：名称:d=默认时长,o=音阶,b=速度:音符序列
        rtttl = f"{song_name}:d=4,o=5,b={bpm}:"
        rtttl += ','.join(self.notes[:32])  # 限制长度避免过长

        return rtttl

    def convert(self, midi_path, output_path, song_name="Converted"):
        """主转换函数"""
        self.parse_midi(midi_path)
        rtttl_text = self.generate_rtttl(song_name)

        with open(output_path, 'w') as f:
            f.write(rtttl_text)

        print(f"转换完成！保存到 {output_path}")
        print(rtttl_text[:100] + "...")  # 预览前100字符

        return rtttl_text


# 命令行使用
if __name__ == "__main__":
    # parser = argparse.ArgumentParser(description='转换MIDI到RTTTL格式')
    # parser.add_argument('input_file', help='输入MIDI文件路径')
    # parser.add_argument('--output', '-o', default='output.rtttl', help='输出RTTTL文件路径')
    # parser.add_argument('--name', '-n', default='Song', help='铃声名称')
    #
    # args = parser.parse_args()

    input_file = "./RTTTL/youre_only_lonely_L.mid"  # 你的MP4文件
    filename = os.path.basename(input_file).split(".")[0]
    filePath = os.path.dirname(input_file)
    output_rtttl = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件

    converter = MidiToRtttl()
    converter.convert(input_file, output_rtttl, "MySong")