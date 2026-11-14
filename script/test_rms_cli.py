#!/usr/bin/env python3
"""Check file/DRS RMS rendering with a built swgbtool and installed game assets."""
import os
import re
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
             (60001, b'if TEST_MAP #const GROUND 3 else #const GROUND 53 endif\n')]
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

    rendered=run(['render-rms',data,work/'map.rms',32,1,work/'file.png'],0)
    base = ['render-rms-resource',data,work/'test.drs',60000,32,1,work/'archive.png']
    run([*base,'--define','TEST_MAP'],0)
    assert (work/'file.png').read_bytes() == (work/'archive.png').read_bytes(), 'DRS and file rendering differ'
    two_player_image = (work/'file.png').read_bytes()
    zone_file=run(['render-rms',data,work/'map.rms',32,1,work/'zones.png','--zones',4],0)
    run(['render-rms-resource',data,work/'test.drs',60000,32,1,work/'archive-zones.png',
         '--define','TEST_MAP','--zones',4],0)
    assert (work/'zones.png').read_bytes()==(work/'archive-zones.png').read_bytes(), 'File/DRS zones differ'
    assert 'RMS terrain zones: restriction 4' in zone_file
    assert re.search(r'Zone tables: \d+ contact cells, \d+ two-hop bridge cells',zone_file)
    assert re.search(r'final RNG (\d+)',rendered).group(1)==re.search(r'final RNG (\d+)',zone_file).group(1), 'Zone rendering changed map RNG'
    for bad in ['-1','bad','999999']:
        run(['render-rms',data,work/'map.rms',32,1,work/'bad-zones.png','--zones',bad],2)
    assert not (work/'bad-zones.png').exists(), 'Invalid restriction created an image'
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
    for fixture, count in [('dry-terrain',0),('water-terrain',0),('forest-terrain',40),('carbon-terrain',40),('early-forest',756),('nova-resource',492),('grouped-nova',503)]:
        output=run(['render-rms',data,root/'tools/fixtures'/(fixture+'.rms'),32,1,work/(fixture+'.png')],0)
        assert f'{count} native objects' in output, output
        assert (work/(fixture+'.png')).stat().st_size>100, 'Missing terrain rendering'
    owned = root/'tools/fixtures/owned-command-center.rms'
    image = work/'owned.png'
    owned_args = ['render-rms',data,owned,32,1,image,'--players',1]
    output=run([*owned_args,'--civilization',2],0)
    assert '1 native objects' in output and image.stat().st_size>100
    image.unlink()
    run(owned_args,1)
    run([*owned_args,'--civilization',2,'--players',2],1)
    for bad in ['0','9','-1','bad']:
        run([*owned_args,'--civilization',bad],2)
    run([*owned_args,'--civilization',2,'--civilization',2],1)
    assert not image.exists(), 'Rejected owned setup created an image'
    output=run([*owned_args,'--players',2,'--civilization',1,'--civilization',2],0)
    assert '2 native objects' in output and '1410736551' in output and image.stat().st_size>100
    output=run(['render-rms',data,root/'tools/fixtures/owned-workers.rms',32,1,work/'workers.png',
                '--players',1,'--civilization',1],0)
    assert '4 native objects' in output and '1943766570' in output
    assert (work/'workers.png').stat().st_size>100
    run(['render-rms',data,root/'tools/fixtures/owned-workers.rms',32,1,work/'invalid-workers.png',
         '--players',3,'--civilization',1,'--civilization',2],1)
    assert not (work/'invalid-workers.png').exists()
    for players,count in [(3,12),(8,32)]:
        civs=[value for civ in range(1,players+1) for value in ('--civilization',civ)]
        output=run(['render-rms',data,root/'tools/fixtures/owned-workers.rms',32,1,work/f'workers-{players}.png',
                    '--players',players,*civs],0)
        assert f'{count} native objects' in output
    output=run(['render-rms',data,root/'tools/fixtures/owned-workers-clipped.rms',8,1,work/'clipped-workers.png',
                '--players',2,'--civilization',1,'--civilization',1],0)
    assert '8 native objects' in output and '2608792336' in output
    civs=[value for civ in range(1,9) for value in ('--civilization',civ)]
    output=run(['render-rms',data,root/'tools/fixtures/owned-workers-clipped.rms',8,1,work/'dense-workers.png',
                '--players',8,*civs],0)
    assert '30 native objects' in output and '772532579' in output


    output=run(['render-rms',data,root/'tools/fixtures/owned-workers.rms',32,1,work/'two-workers.png',
                '--players',2,'--civilization',1,'--civilization',2],0)
    assert '8 native objects' in output and '892528603' in output
    output=run(['render-rms',data,root/'tools/fixtures/owned-forest.rms',32,1,work/'owned-forest.png',
                '--players',2,'--civilization',1,'--civilization',1],0)
    assert '39 native objects' in output and '2026451952' in output
    for fixture,count,state in [('owned-forest-workers',45,4221457422),('owned-forest-cleanup',39,2026451952),('owned-forest-cleanup-workers',45,4221457422)]:
        output=run(['render-rms',data,root/'tools/fixtures'/(fixture+'.rms'),32,1,work/(fixture+'.png'),
                    '--players',2,'--civilization',1,'--civilization',1],0)
        assert f'{count} native objects' in output and str(state) in output
        assert (work/(fixture+'.png')).stat().st_size>100

    for players,count,state in [(1,42,4000822828),(3,50,3758076468),(8,46,504322075)]:
        civs=[value for civ in range(1,players+1) for value in ('--civilization',civ)]
        for fixture in ['owned-forest-workers','owned-forest-cleanup-workers']:
            output=run(['render-rms',data,root/'tools/fixtures'/(fixture+'.rms'),32,1,
                        work/f'{fixture}-{players}.png','--players',players,*civs],0)
            assert f'{count} native objects' in output and str(state) in output


print('RMS file/DRS parity, includes, defines and rejection checks passed')
