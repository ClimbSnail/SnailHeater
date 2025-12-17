import mido
import math
import os


def midi_to_rtttl(
        midi_file_path,
        output_rtttl_path="output.rtttl",
        target_track=None  # 新增：指定轨道索引（如0、1、2...），默认None=自动选主旋律轨
):
    # 加载MIDI文件
    mid = mido.MidiFile(midi_file_path)

    # ---------------------- 新增：多音轨处理逻辑 ----------------------
    # 1. 扫描所有轨道，统计每个轨道的音符数量和名称
    track_info = []
    for track_idx, track in enumerate(mid.tracks):
        note_count = 0
        track_name = f"Track_{track_idx}"  # 默认轨道名
        for msg in track:
            # 获取轨道名称（MIDI的track_name消息）
            if msg.type == "track_name":
                track_name = msg.name.strip()
            # 统计音符数量（只算有效音符消息）
            if msg.type in ["note_on", "note_off"] and hasattr(msg, "note"):
                note_count += 1
        track_info.append((track_idx, track_name, note_count))

    # 2. 选择目标轨道（默认自动选音符最多的主旋律轨）
    if target_track is None:
        # 过滤无音符的轨道，选音符数最多的
        valid_tracks = [t for t in track_info if t[2] > 0]
        if not valid_tracks:
            raise ValueError("MIDI文件无有效音符轨道！")
        target_track = max(valid_tracks, key=lambda x: x[2])[0]  # 音符最多的轨道索引
        target_track_name = track_info[target_track][1]
        print(f"自动选择主旋律轨道：轨道{target_track}（名称：{target_track_name}，音符数：{track_info[target_track][2]}）")
    else:
        # 手动指定轨道：检查索引有效性
        if target_track < 0 or target_track >= len(mid.tracks):
            raise ValueError(f"轨道索引{target_track}无效！MIDI共{len(mid.tracks)}个轨道（0~{len(mid.tracks) - 1}）")
        target_track_name = track_info[target_track][1]
        print(f"手动选择轨道：轨道{target_track}（名称：{target_track_name}，音符数：{track_info[target_track][2]}）")

    # 3. 获取目标轨道的所有消息
    target_track_msgs = mid.tracks[target_track]
    # ------------------------------------------------------------------

    # RTTTL配置（速度自动读取MIDI实际值，避免硬编码偏差）
    default_bpm = 120
    # 读取MIDI自带速度（优先目标轨道的set_tempo消息）
    actual_bpm = default_bpm
    for msg in target_track_msgs:
        if msg.type == "set_tempo":
            actual_bpm = mido.tempo2bpm(msg.tempo)
            break
    rtttl_header = f"midi_converted:b={int(actual_bpm)}:o=4:d=4"  # 保持原默认配置

    rtttl_notes = []

    # 保留你原有的MIDI音高映射（C2-C8完整支持）
    midi_note_map = {
        36: "c2", 37: "c#2", 38: "d2", 39: "d#2", 40: "e2", 41: "f2", 42: "f#2", 43: "g2", 44: "g#2", 45: "a2",
        46: "a#2", 47: "b2",
        48: "c3", 49: "c#3", 50: "d3", 51: "d#3", 52: "e3", 53: "f3", 54: "f#3", 55: "g3", 56: "g#3", 57: "a3",
        58: "a#3", 59: "b3",
        60: "c4", 61: "c#4", 62: "d4", 63: "d#4", 64: "e4", 65: "f4", 66: "f#4", 67: "g4", 68: "g#4", 69: "a4",
        70: "a#4", 71: "b4",
        72: "c5", 73: "c#5", 74: "d5", 75: "d#5", 76: "e5", 77: "f5", 78: "f#5", 79: "g5", 80: "g#5", 81: "a5",
        82: "a#5", 83: "b5",
        84: "c6", 85: "c#6", 86: "d6", 87: "d#6", 88: "e6", 89: "f6", 90: "f#6", 91: "g6", 92: "g#6", 93: "a6",
        94: "a#6", 95: "b6",
        96: "c7", 97: "c#7", 98: "d7", 99: "d#7", 100: "e7", 101: "f7", 102: "f#7", 103: "g7", 104: "g#7", 105: "a7",
        106: "a#7", 107: "b7",
        108: "c8"
    }

    # 音长映射优化（支持附点音符，解决节奏不准）
    midi_ticks_per_beat = mid.ticks_per_beat
    note_length_map = {
        midi_ticks_per_beat * 4: "1",  # 全音符
        int(midi_ticks_per_beat * 4 * 1.5): "1.",  # 附点全音符
        midi_ticks_per_beat * 2: "2",  # 二分音符
        int(midi_ticks_per_beat * 2 * 1.5): "2.",  # 附点二分音符
        midi_ticks_per_beat: "4",  # 四分音符（默认）
        int(midi_ticks_per_beat * 1.5): "4.",  # 附点四分音符
        midi_ticks_per_beat // 2: "8",  # 八分音符
        int((midi_ticks_per_beat // 2) * 1.5): "8.",  # 附点八分音符
        midi_ticks_per_beat // 4: "16",  # 十六分音符
        int((midi_ticks_per_beat // 4) * 1.5): "16.",  # 附点十六分音符
        midi_ticks_per_beat // 8: "32",  # 三十二分音符
    }

    # 解析目标轨道的音符（修复时间计算错误，用全局时间戳）
    current_tick = 0  # 全局时间戳，精准计算音符时长
    current_notes = {}  # 存储正在播放的音符: {note: (start_tick, velocity)}

    for msg in target_track_msgs:
        current_tick += msg.time  # 累加全局时间，替代原相对时间

        if msg.type == "note_on" and msg.velocity > 0:
            # 音符开始：记录起始时间和力度（过滤弱音）
            if msg.note in midi_note_map:
                current_notes[msg.note] = (current_tick, msg.velocity)
        elif (msg.type == "note_off" or (msg.type == "note_on" and msg.velocity == 0)) and msg.note in current_notes:
            # 音符结束，计算音长
            start_tick, velocity = current_notes.pop(msg.note)
            total_ticks = current_tick - start_tick

            # 过滤杂音：长度小于32分音符或力度太弱的音忽略
            min_duration = midi_ticks_per_beat // 8
            if total_ticks < min_duration or velocity < 30:
                continue

            # 匹配最接近的音长
            closest_length = min(note_length_map.keys(), key=lambda x: abs(x - total_ticks))
            rtttl_length = note_length_map[closest_length]

            # 获取RTTTL音符（未知音高忽略）
            rtttl_note = midi_note_map[msg.note]
            # 组合音长+音符（默认音长可省略）
            if rtttl_length == "4":
                rtttl_notes.append(rtttl_note)
            else:
                rtttl_notes.append(f"{rtttl_length}{rtttl_note}")

    # 移除连续重复音符（避免卡顿）
    cleaned_notes = []
    for note in rtttl_notes:
        if not cleaned_notes or note != cleaned_notes[-1]:
            cleaned_notes.append(note)
    rtttl_notes = cleaned_notes

    # 生成完整RTTTL字符串
    if not rtttl_notes:
        raise ValueError("所选轨道无有效音符！可尝试更换轨道索引，或降低弱音/短音过滤阈值")
    rtttl_full = f"{rtttl_header}:{','.join(rtttl_notes)}"

    # 保存到文件（保留原保存逻辑）
    with open(output_rtttl_path, "w", encoding="utf-8") as f:
        f.write(rtttl_full)

    print("转换完成！RTTTL内容：")
    print(rtttl_full)
    return rtttl_full


if __name__ == "__main__":
    # 完全保留你原有的使用方式，无任何改变！
    input_file = "./RTTTL/youre_only_lonely_L.mid"  # 你的MIDI文件路径
    filename = os.path.basename(input_file).split(".")[0]
    filePath = os.path.dirname(input_file)
    output_rtttl = f"{filePath}/{filename}.rtttl"  # 输出RTTTL文件

    # 方式1：默认自动选择主旋律轨道（和你原代码使用方式完全一致）
    rtttl = midi_to_rtttl(input_file, output_rtttl)

    # 方式2（可选）：手动指定轨道索引（如想转第2个轨道，索引为1）
    # rtttl = midi_to_rtttl(input_file, output_rtttl, target_track=1)