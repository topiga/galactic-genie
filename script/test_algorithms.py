#!/usr/bin/env python3
"""Run asset-independent RMS and military tests with Apple Clang sanitizers."""
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).resolve()
build.mkdir(parents=True, exist_ok=True)
cases = {
    'rms': ['src/engine/rms_script.cpp', 'src/core/drs.cpp'],
    'rms_map': ['src/engine/rms_early_forest.cpp', 'src/engine/rms_terrain_cleanup.cpp', 'src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp', 'src/engine/rms_terrain.cpp', 'src/engine/rms_map.cpp', 'src/engine/rms_script.cpp', 'src/engine/rms_objects.cpp', 'src/engine/rms_player_starts.cpp', 'src/core/drs.cpp'],
    'rms_terrain_cleanup': ['src/engine/rms_terrain_cleanup.cpp'],
    'rms_terrain': ['src/engine/rms_terrain.cpp'],
    'rms_player_starts': ['src/engine/rms_player_starts.cpp'],
    'rms_scenery': ['src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'rms_scenery_appearance': ['src/engine/rms_scenery_appearance.cpp'],
    'rms_early_forest': ['src/engine/rms_early_forest.cpp', 'src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'rms_object': ['src/engine/rms_objects.cpp'],
    'ai_military': ['src/engine/ai_military.cpp'],
    'ai_targets': ['src/engine/ai_targets.cpp'],
}
for name, sources in cases.items():
    executable = build/(name+'-test')
    command = ['xcrun', 'clang++', '-std=c++17', '-I'+str(root/'src'),
               '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined',
               '-fno-sanitize-recover=all', str(root/'tools'/(name+'_test.cpp')),
               *[str(root/p) for p in sources], '-o', str(executable)]
    subprocess.run(command, cwd=build, timeout=180, check=True)
    subprocess.run([str(executable)], cwd=build, timeout=60, check=True)
