"""Compile and exercise the firmware's actual profile-flash timing logic."""

import importlib.util
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ProfileFlashTest(unittest.TestCase):
    def test_profile_flash_state_and_palette(self):
        compiler = shutil.which("cc")
        if compiler:
            command = [compiler]
        elif importlib.util.find_spec("ziglang"):
            command = [sys.executable, "-m", "ziglang", "cc"]
        else:
            self.fail("Install a C compiler or run uv with --with ziglang")
        with tempfile.TemporaryDirectory() as directory:
            executable = str(Path(directory) / "profile-flash-test")
            subprocess.run(
                command + ["-std=c11", "-Wall", "-Wextra", "-Werror",
                           "-I", str(ROOT / "src"),
                           str(ROOT / "tests/profile_flash_test.c"), "-o", executable],
                check=True,
            )
            subprocess.run([executable], check=True)
