#!/usr/bin/env python3
"""Run asset-independent RMS and military tests with Apple Clang sanitizers."""
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).resolve()
build.mkdir(parents=True, exist_ok=True)
cases = {
    'terrain_draw_scan': [],
    'rms': ['src/engine/rms_script.cpp', 'src/core/drs.cpp'],
    'rms_object_rows': ['src/engine/rms_object_rows.cpp'],
    'rms_map': ['src/engine/rms_walls.cpp', 'src/engine/rms_land_placement.cpp', 'src/engine/rms_zones.cpp', 'src/engine/rms_connections.cpp', 'src/engine/rms_elevation.cpp', 'src/engine/rms_cliffs.cpp', 'src/engine/rms_slopes.cpp', 'src/engine/rms_early_forest.cpp', 'src/engine/rms_terrain_cleanup.cpp', 'src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp', 'src/engine/rms_terrain.cpp', 'src/engine/rms_map.cpp', 'src/engine/rms_object_rows.cpp', 'src/engine/rms_script.cpp', 'src/engine/rms_objects.cpp', 'src/engine/rms_player_starts.cpp', 'src/core/drs.cpp'],
    'rms_terrain_cleanup': ['src/engine/rms_terrain_cleanup.cpp'],
    'rms_land_placement': ['src/engine/rms_land_placement.cpp'],
    'rms_connections': ['src/engine/rms_connections.cpp', 'src/engine/rms_script.cpp', 'src/engine/rms_terrain_cleanup.cpp', 'src/core/drs.cpp'],
    'rms_slopes': ['src/engine/rms_slopes.cpp'],
    'rms_cliffs': ['src/engine/rms_cliffs.cpp'],
    'rms_elevation': ['src/engine/rms_elevation.cpp', 'src/engine/rms_slopes.cpp'],
    'rms_terrain': ['src/engine/rms_terrain.cpp'],
    'rms_player_starts': ['src/engine/rms_player_starts.cpp'],
    'rms_scenery': ['src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'rms_scenery_appearance': ['src/engine/rms_scenery_appearance.cpp'],
    'rms_early_forest': ['src/engine/rms_early_forest.cpp', 'src/engine/rms_scenery.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'rms_zones': ['src/engine/rms_zones.cpp'],
    'rms_group': ['src/engine/rms_zones.cpp', 'src/engine/rms_objects.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'rms_object': ['src/engine/rms_zones.cpp', 'src/engine/rms_objects.cpp', 'src/engine/rms_scenery_appearance.cpp'],
    'ai_military': ['src/engine/ai_military.cpp'],
    'ai_targets': ['src/engine/ai_targets.cpp'],
    'ai_group_search': [],
    'ai_attack_memory': [],
    'ai_player_facts': [],
    'ai_unit_line': [],
    'ai_food_facts': [],
    'ai_resource_knowledge': [],
    'ai_session_context': [],
    'ai_build_admission': [],
    'ai_build_request_queue': [],
    'ai_gather_allocation': [],
    'ai_gather_quota': [],
    'ai_resource_selection': [],
    'ai_resource_lifecycle': [],
    'ai_economic_delivery': [],
    'ai_gather_controller': [],
    'ai_build_placement': [],
    'ai_builder_selection': [],
    'ai_prefab_grid': [],
    'ai_resource_build_placement': [],

    'ai_group_targets': ['src/engine/ai_group_targets.cpp', 'src/engine/ai_targets.cpp'],
}
for name, sources in cases.items():
    executable = build/(name+'-test')
    command = ['xcrun', 'clang++', '-std=c++17', '-I'+str(root/'src'),
               '-O1', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined',
               '-fno-sanitize-recover=all', str(root/'tools'/(name+'_test.cpp')),
               *[str(root/p) for p in sources], '-o', str(executable)]
    subprocess.run(command, cwd=build, timeout=180, check=True)
    subprocess.run([str(executable)], cwd=build, timeout=120 if name=="rms_map" else 60, check=True)
