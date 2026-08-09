# -*- coding: utf-8 -*-
import sys
from dataclasses import dataclass
from pathlib import Path


def application_root() -> Path:
    """返回源码目录或 PyInstaller 解包后的资源根目录。"""
    frozen_root = getattr(sys, "_MEIPASS", None)
    return Path(frozen_root).resolve() if frozen_root else Path(__file__).resolve().parent.parent


def external_data_root() -> Path:
    """返回冻结后可执行文件同目录的数据目录。"""
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return application_root()


@dataclass(frozen=True)
class RuntimePaths:
    root: Path

    @classmethod
    def discover(cls):
        return cls(application_root())

    def resolve(self, value: str) -> Path:
        path = Path(value or ".")
        return path if path.is_absolute() else (self.root / path).resolve()

    @property
    def external_root(self) -> Path:
        """返回供用户单独维护运行时文件的外部目录。"""
        return external_data_root()

    def external_resource_dir(self, name: str) -> Path:
        """Return a user-maintained resource directory beside the executable."""
        return self.external_root / name

    @property
    def generate_dir(self):
        return self.external_root / "Generate"

    @property
    def background_dir(self):
        return self.generate_dir / "Backgroud"

    @property
    def background_cache_dir(self):
        return self.generate_dir / "Cache" / "Backgroud"

    @property
    def wallpaper_cache_dir(self):
        return self.generate_dir / "Cache" / "Wallpaper"

    @property
    def wallpaper_dir(self):
        return self.generate_dir / "Wallpaper"

    @property
    def wallpaper_file(self):
        return self.wallpaper_dir / "Wallpaper.lsw"

    @property
    def coredump_dir(self):
        return self.generate_dir / "Coredump"

    @property
    def stub_flasher_dir(self):
        return self.external_root / "stub_flasher"

    @property
    def ffmpeg(self):
        return self.root / "ffmpeg.exe"

    def require_file(self, path: Path, description: str) -> Path:
        if not path.is_file():
            raise FileNotFoundError(f"缺少{description}：{path}")
        return path

    def ensure_output_dirs(self):
        """
        创建工具运行所需的输出目录。

        Returns:
            函数处理结果；具体类型由调用场景决定。
        """
        for path in (
            self.background_dir,
            self.background_cache_dir,
            self.wallpaper_cache_dir,
            self.wallpaper_dir,
            self.coredump_dir,
        ):
            path.mkdir(parents=True, exist_ok=True)
