#!/usr/bin/env python3
"""macOS fallback using the upstream CMake source list and installed SDL2."""
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).resolve()
build.mkdir(parents=True, exist_ok=True)
core = re.search(r'set\(SWGB_CORE_SOURCES\s+(.*?)\)', (root/'CMakeLists.txt').read_text(), re.S).group(1).split()
command = ['xcrun', 'clang++', '-std=c++17', '-O1',
           *[str(root/p) for p in ['src/platform/desktop/main_sdl.cpp', *core, 'src/render/soft_renderer.cpp']],
           '-lz', '-framework', 'CoreText', '-framework', 'CoreGraphics', '-framework', 'CoreFoundation', '-o', str(build/'swgb-desktop.next')]
command += shlex.split(subprocess.check_output(['/opt/homebrew/bin/pkg-config', '--cflags', '--libs', 'sdl2', 'vorbisfile'], text=True))
inputs = sorted([*root.glob('src/**/*.h'), *root.glob('src/**/*.cpp'), *root.glob('third_party/**/*.h'), root/'CMakeLists.txt'])
hash_value = hashlib.sha256(json.dumps(command).encode())
for p in inputs:
    hash_value.update(str(p).encode()); hash_value.update(p.read_bytes())
stamp = build/'desktop-source.sha256'
if not (build/'swgb-desktop').exists() or not stamp.exists() or stamp.read_text() != hash_value.hexdigest():
    (build/'desktop-build-command.json').write_text(json.dumps(command, indent=2))
    with (build/'desktop-build.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=180, check=True)
    (build/'swgb-desktop.next').replace(build/'swgb-desktop')
    stamp.write_text(hash_value.hexdigest())
print(build/'swgb-desktop')

import plistlib
import shutil
bundle = build/'Galactic Genie Skirmish.app'/'Contents'
(bundle/'MacOS').mkdir(parents=True, exist_ok=True)
shutil.copy2(build/'swgb-desktop', bundle/'MacOS'/'swgb-desktop.next')
(bundle/'MacOS'/'swgb-desktop.next').replace(bundle/'MacOS'/'swgb-desktop')
with (bundle/'Info.plist').open('wb') as info:
    plistlib.dump({'CFBundleExecutable': 'swgb-desktop', 'CFBundleIdentifier': 'local.swgb.galactic-genie.skirmish',
                  'CFBundleName': 'Galactic Genie Skirmish', 'CFBundlePackageType': 'APPL',
                  'NSHighResolutionCapable': True}, info)
