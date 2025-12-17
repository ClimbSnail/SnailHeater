# 安装依赖
# pip install mido python-rtmidi -i https://mirrors.aliyun.com/pypi/simple/

import mido
import math
import os

def midi_to_rtttl(midi_file_path, output_path, song_name="Converted"):
    """
    将MIDI文件转换为RTTTL格式
    简化版本 - 处理单声道旋律
    """
    mid = mido.MidiFile(midi_file_path)
    
    # RTTTL基础设置
    bpm = 120  # 默认BPM
    default_duration = 4  # 4分音符
    default_octave = 5
    
    # 提取音符
    notes = []
    for track in mid.tracks:
        for msg in track:
            if msg.type == 'set_tempo':
                bpm = mido.tempo2bpm(msg.tempo)
            elif msg.type == 'note_on' and msg.velocity > 0:
                # 将MIDI音符转换为RTTTL格式
                note_name = midi_note_to_name(msg.note)
                notes.append(note_name)
    
    # 构建RTTTL字符串
    rtttl_str = f"{song_name}:d={default_duration},o={default_octave},b={int(bpm)}:"
    rtttl_str += ','.join(notes[:50])  # 限制长度
    
    # 保存文件
    with open(output_path, 'w') as f:
        f.write(rtttl_str)
    
    return rtttl_str

def midi_note_to_name(midi_note):
    """将MIDI音符编号转换为RTTTL音符名称"""
    note_names = ['c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#', 'a', 'a#', 'b']
    octave = (midi_note // 12) - 1
    note_index = midi_note % 12
    return f"{note_names[note_index]}{octave}"

# 使用示例
if __name__ == "__main__":

    input_file = "./RTTTL/youre_only_lonely_L.mid"  # 你的MP4文件
    filename = os.path.basename(input_file).split(".")[0]
    filePath = os.path.dirname(input_file)
    output_rtttl = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件

    rtttl = midi_to_rtttl(input_file, output_rtttl, "MySong")
    print(rtttl)