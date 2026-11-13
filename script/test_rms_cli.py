#!/usr/bin/env python3
"""Check file/DRS RMS rendering with a built swgbtool and installed game assets."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
build = Path(os.environ.get('SWGB_DESKTOP_BUILD', Path.home()/'.local/share/swgb/galactic-genie-build')).resolve()
data = Path(os.environ['SWGB_GAME_DIR']).resolve()/'Data'
tool = build/'swgbtool'

with tempfile.TemporaryDirectory(prefix='rms-cli-', dir=build) as directory:
    work = Path(directory)
    source = (root/'tools/fixtures/player-terrain.rms').read_text()
    (work/'map.rms').write_text(source)
    # Authored resources only: exercise archive includes and startup defines.
    files = [(60000, ('#include_drs test.def 60001\n'+source.replace('terrain_type 3', 'terrain_type GROUND')).encode()),
             (60001, b'if TEST_MAP #const GROUND 3 else #const GROUND 1 endif\n')]
    offset = 96+12*len(files)
    archive = bytearray(b'Local RMS test'.ljust(60,b'\0')+b'1.00'+b'tribe'.ljust(12,b'\0')+
                        struct.pack('<iiIII',1,offset,0x62696e61,96,len(files)))
    for resource, content in files:
        archive += struct.pack('<iII',resource,offset,len(content))
        offset += len(content)
    for _, content in files:
        archive += content
    (work/'test.drs').write_bytes(archive)

    def run(arguments, status):
        result = subprocess.run([str(tool), *map(str, arguments)], text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
        if result.returncode != status:
            raise RuntimeError(result.stdout)
        return result.stdout

    run(['render-rms',data,work/'map.rms',32,1,work/'file.png'],0)
    base = ['render-rms-resource',data,work/'test.drs',60000,32,1,work/'archive.png']
    run([*base,'--define','TEST_MAP'],0)
    assert (work/'file.png').read_bytes() == (work/'archive.png').read_bytes(), 'DRS and file rendering differ'
    two_player_image = (work/'file.png').read_bytes()
    run(['render-rms',data,work/'map.rms',32,1,work/'file.png','--players',8],0)
    run([*base,'--define','TEST_MAP','--players',8],0)
    assert (work/'file.png').read_bytes() == (work/'archive.png').read_bytes(), 'Eight-player DRS and file rendering differ'
    assert (work/'file.png').read_bytes() != two_player_image, 'Configured player count was ignored'
    (work/'archive.png').unlink()
    assert 'unsupported' in run(base,1)
    assert 'FIXED_POSITIONS' in run([*base,'--define','FIXED_POSITIONS'],1)
    run([*base,'--define'],2)
    for invalid in [0,9,-1,'bad']:
        run([*base,'--players',invalid],2)
    run([*base,'--players'],2)
    run(['render-rms-resource',data,work/'test.drs',-1,32,1,work/'archive.png'],2)
    run(['render-rms-resource',data,work/'test.drs',60002,32,1,work/'archive.png'],1)
    assert not (work/'archive.png').exists(), 'Rejected input created output'
    (work/'invalid.rms').write_text(source.replace('base_terrain 0','base_terrain 256'))
    assert 'unsupported terrain parameter/value base_terrain' in run(
        ['render-rms',data,work/'invalid.rms',32,1,work/'rejected.png'],1)
    assert not (work/'rejected.png').exists(), 'Wrapped terrain ID created output'
print('RMS file/DRS parity, includes, defines and rejection checks passed')
