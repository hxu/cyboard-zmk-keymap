"""Exercise the lighting callbacks with mocked Zephyr and ZMK boundaries."""

import importlib.util
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class LayerColorsTest(unittest.TestCase):
    def test_startup_and_color_updates_on_both_halves(self):
        compiler = shutil.which("cc")
        if compiler:
            command = [compiler]
        elif importlib.util.find_spec("ziglang"):
            command = [sys.executable, "-m", "ziglang", "cc"]
        else:
            self.fail("Install a C compiler or run uv with --with ziglang")

        with tempfile.TemporaryDirectory() as directory:
            include = Path(directory)
            # SDK declarations are supplied by the C fixture. Empty headers let
            # the host compiler include the actual firmware callback source.
            source = (ROOT / "src/layer_colors.c").read_text()
            for header in re.findall(r"^#include <([^>]+)>", source, re.M):
                path = include / header
                path.parent.mkdir(parents=True, exist_ok=True)
                path.touch()

            for central in (1, 0):
                with self.subTest(central=central):
                    executable = str(include / f"layer-colors-{central}")
                    subprocess.run(
                        command + ["-std=c11", "-Wall", "-Werror",
                                   f"-DCONFIG_ZMK_SPLIT_ROLE_CENTRAL={central}",
                                   "-I", directory,
                                   str(ROOT / "tests/layer_colors_test.c"), "-o", executable],
                        check=True,
                    )
                    subprocess.run([executable], check=True)
