"""Play SnailManager ``.pwm_song_8bit`` files on a PC.

The file starts with a two-byte little-endian unsigned sample-rate field, then
contains one unsigned 8-bit mono PCM sample per following byte.  On Windows,
the script wraps those samples in a temporary WAV container and plays it using
only the standard ``winsound`` module.
"""

import argparse
import struct
import sys
import tempfile
import wave
from pathlib import Path
from typing import Callable, Optional, Tuple


SAMPLE_RATE_STRUCT = struct.Struct("<H")


def read_pcm_u8_1(path: Path) -> Tuple[int, bytes]:
    """Read a sample-rate-prefixed unsigned 8-bit mono PCM PWM music file."""
    data = path.read_bytes()
    if len(data) <= SAMPLE_RATE_STRUCT.size:
        raise ValueError(
            f"8bit PWM Song file is too short: {path}; "
            "it requires a 2-byte sample-rate header and at least one sample"
        )

    sample_rate = SAMPLE_RATE_STRUCT.unpack_from(data)[0]
    if sample_rate == 0:
        raise ValueError(f"8bit PWM Song has an invalid zero sample rate: {path}")
    return sample_rate, data[SAMPLE_RATE_STRUCT.size :]


def read_legacy_raw_pcm_u8_1(path: Path) -> bytes:
    """Read a pre-header raw unsigned 8-bit PCM file for manual recovery."""
    data = path.read_bytes()
    if not data:
        raise ValueError(f"Legacy raw 8bit PWM Song file is empty: {path}")
    return data


def write_preview_wav(data: bytes, path: Path, sample_rate: int) -> None:
    """Wrap raw unsigned 8-bit mono PCM samples in a standard WAV container."""
    if sample_rate <= 0:
        raise ValueError("Sample rate must be positive")
    if not data:
        raise ValueError("8bit PWM Song data is empty")

    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(1)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(data)


def play_pcm_u8_1(data: bytes, repeat: int, sample_rate: int) -> None:
    """Play raw 8-bit PWM music through the normal Windows audio device."""
    if sys.platform != "win32":
        raise RuntimeError("This player currently requires Windows")
    if repeat < 0:
        raise ValueError("Repeat must be zero or a positive integer")

    import winsound

    with tempfile.TemporaryDirectory(prefix="pcm_u8_1_preview_") as temp_dir:
        preview_path = Path(temp_dir) / "preview.wav"
        write_preview_wav(data, preview_path, sample_rate)
        # PlaySound is synchronous by default. SND_SYNC is not exposed by
        # winsound on some Python 3.8 Windows builds, so do not reference it.
        flags = winsound.SND_FILENAME
        count = 0
        while repeat == 0 or count < repeat:
            count += 1
            suffix = f"/{repeat}" if repeat else ""
            print(f"Playing 8bit PWM preview {count}{suffix}...", flush=True)
            winsound.PlaySound(str(preview_path), flags)


def _format_duration(sample_count: int, sample_rate: int) -> str:
    return f"{sample_count / sample_rate:.2f} s"


def main(
    argv: Optional[list] = None,
    player: Callable[[bytes, int, int], None] = play_pcm_u8_1,
) -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Play a .pwm_song_8bit file whose first two bytes are a little-endian "
            "uint16 sample rate and whose remaining bytes are unsigned 8-bit mono PCM."
        )
    )
    parser.add_argument("file", type=Path, help="Path to the .pwm_song_8bit file")
    parser.add_argument(
        "--sample-rate",
        type=int,
        help="Override the sample rate in the file header; required with --legacy-raw",
    )
    parser.add_argument(
        "--legacy-raw",
        action="store_true",
        help="Treat the entire file as an old headerless raw PCM asset",
    )
    parser.add_argument(
        "--repeat",
        type=int,
        default=1,
        help="Number of times to repeat; 0 means repeat forever (default: 1)",
    )
    parser.add_argument(
        "--export-wav",
        type=Path,
        metavar="PATH",
        help="Also export the PCM samples as a standard WAV file",
    )
    parser.add_argument(
        "--no-play",
        action="store_true",
        help="Only validate/export the file; do not play it",
    )
    args = parser.parse_args(argv)

    if args.sample_rate is not None and not 0 < args.sample_rate <= 0xFFFF:
        parser.error("--sample-rate must be between 1 and 65535")
    if args.legacy_raw and args.sample_rate is None:
        parser.error("--legacy-raw requires --sample-rate")
    if args.repeat < 0:
        parser.error("--repeat must be zero or a positive integer")
    if not args.file.is_file():
        print(f"File not found: {args.file}", file=sys.stderr)
        return 2

    try:
        if args.legacy_raw:
            stored_sample_rate = None
            data = read_legacy_raw_pcm_u8_1(args.file)
        else:
            stored_sample_rate, data = read_pcm_u8_1(args.file)
        sample_rate = args.sample_rate or stored_sample_rate

        print(f"File: {args.file}")
        if stored_sample_rate is None:
            print("Format: legacy headerless unsigned 8-bit PCM, mono")
        else:
            print(f"Stored sample rate: {stored_sample_rate} Hz (little-endian uint16 header)")
        if args.sample_rate is not None and args.sample_rate != stored_sample_rate:
            print(f"Using sample-rate override: {sample_rate} Hz")
        print(f"PCM: unsigned 8-bit, mono, {sample_rate} Hz")
        print(f"Samples: {len(data)}, duration: {_format_duration(len(data), sample_rate)}")

        if args.export_wav:
            write_preview_wav(data, args.export_wav, sample_rate)
            print(f"WAV exported: {args.export_wav}")
        if not args.no_play:
            player(data, args.repeat, sample_rate)
    except KeyboardInterrupt:
        print("\nPlayback stopped.")
        return 130
    except (OSError, RuntimeError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    # python tools\play_pcm_u8_1.py "Generate\Cache\Wallpaper\example.pwm_song_8bit"
    # python tools\play_pcm_u8_1.py "old.pwm_song_8bit" --legacy-raw --sample-rate 8000
    raise SystemExit(main())
