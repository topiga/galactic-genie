#!/usr/bin/env python3
"""Build the comparison snapshot with Apple Clang when CMake is unavailable."""
import os
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).absolute()
build.mkdir(parents=True, exist_ok=True)
core = re.search(r'set\(SWGB_CORE_SOURCES\s+(.*?)\)', (root/'CMakeLists.txt').read_text(), re.S).group(1).split()
output = build/'swgb-rms-snapshot'
subprocess.run(['xcrun', 'clang++', '-std=c++17', '-O1', '-g', '-fsanitize=address,undefined',
                '-fno-sanitize-recover=all', *[str(root/p) for p in
                ['tools/rms_snapshot.cpp', *core, 'src/render/soft_renderer.cpp']],
                '-lz', '-o', str(output)+'.next'], check=True, timeout=180)
Path(str(output)+'.next').replace(output)
print(output)
