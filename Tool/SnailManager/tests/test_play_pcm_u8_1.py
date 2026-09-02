import struct
import tempfile
import unittest
import wave
from pathlib import Path

from tools import play_pcm_u8_1


class PcmU8_1PlayerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.root = Path(self.temp_dir.name)
        self.source = self.root / "sample.pcm_u8_1"
        self.samples = b"\x00\x40\x80\xc0\xff"
        self.source.write_bytes(struct.pack("<H", 8000) + self.samples)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def test_read_returns_little_endian_sample_rate_and_pcm_payload(self) -> None:
        sample_rate, samples = play_pcm_u8_1.read_pcm_u8_1(self.source)

        self.assertEqual(sample_rate, 8000)
        self.assertEqual(samples, self.samples)

    def test_write_preview_wav_preserves_unsigned_8bit_mono_samples(self) -> None:
        output = self.root / "preview.wav"
        play_pcm_u8_1.write_preview_wav(self.samples, output, sample_rate=8000)

        with wave.open(str(output), "rb") as wav_file:
            self.assertEqual(wav_file.getnchannels(), 1)
            self.assertEqual(wav_file.getsampwidth(), 1)
            self.assertEqual(wav_file.getframerate(), 8000)
            self.assertEqual(wav_file.readframes(wav_file.getnframes()), self.samples)

    def test_main_can_export_without_playback(self) -> None:
        output = self.root / "export.wav"

        result = play_pcm_u8_1.main(
            [str(self.source), "--export-wav", str(output), "--no-play"]
        )

        self.assertEqual(result, 0)
        self.assertTrue(output.is_file())
        with wave.open(str(output), "rb") as wav_file:
            self.assertEqual(wav_file.getframerate(), 8000)
            self.assertEqual(wav_file.readframes(wav_file.getnframes()), self.samples)

    def test_main_passes_samples_and_options_to_player(self) -> None:
        calls = []

        result = play_pcm_u8_1.main(
            [str(self.source), "--sample-rate", "16000", "--repeat", "2"],
            player=lambda data, repeat, sample_rate: calls.append((data, repeat, sample_rate)),
        )

        self.assertEqual(result, 0)
        self.assertEqual(calls, [(self.samples, 2, 16000)])


if __name__ == "__main__":
    unittest.main()
