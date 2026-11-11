#!/usr/bin/env python3
"""Build and run the skirmish lifecycle test with Apple Clang sanitizers."""
import os
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).resolve()
game = Path(os.environ['SWGB_GAME_DIR']).resolve()
build.mkdir(parents=True, exist_ok=True)
core = re.search(r'set\(SWGB_CORE_SOURCES\s+(.*?)\)', (root/'CMakeLists.txt').read_text(), re.S).group(1).split()
sources = ['tools/skirmish_test.cpp', *core, 'src/render/soft_renderer.cpp']
command = ['xcrun', 'clang++', '-std=c++17', '-O1', '-g', '-fsanitize=address,undefined',
           '-fno-sanitize-recover=all', *[str(root/p) for p in sources], '-lz', '-o', str(build/'skirmish-test')]
with (build/'test-build.log').open('w') as log:
    subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=180, check=True)
subprocess.run([str(build/'skirmish-test'), str(game/'Data')], cwd=build, timeout=180, check=True)
