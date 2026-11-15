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

    cliffs=run(['render-rms',data,root/'tools/fixtures/cliff-diagnostic.rms',120,1,work/'cliffs.png'],0)
    assert '29 native objects, final RNG 1150416911' in cliffs
    assert (work/'cliffs.png').stat().st_size>1000
    corner_cliffs=work/'cliff-corner.rms'
    corner_cliffs.write_text((root/'tools/fixtures/cliff-diagnostic.rms').read_text().replace('land_position 50 50','land_position 0 0'))
    corner=run(['render-rms',data,corner_cliffs,120,1,work/'cliff-corner.png'],0)
    assert '19 native objects, final RNG 958887679' in corner
    assert (work/'cliff-corner.png').stat().st_size>1000
    player_cliffs=work/'cliff-players.rms'
    player_cliffs.write_text('<LAND_GENERATION> base_terrain 6 create_player_lands { '
        'terrain_type 6 base_size 0 number_of_tiles 0 } <CLIFF_GENERATION>')
    players=run(['render-rms',data,player_cliffs,120,1,work/'cliff-players.png'],0)
    assert '44 native objects, final RNG 1152740336' in players
    assert (work/'cliff-players.png').stat().st_size>1000
    player_cliffs.write_text(player_cliffs.read_text().replace('base_size 0 number_of_tiles 0',
        'base_size 3 number_of_tiles 1000'))
    growing=run(['render-rms',data,player_cliffs,120,1,work/'cliff-player-growth.png'],0)
    assert '63 native objects, final RNG 1003445079' in growing
    assert (work/'cliff-player-growth.png').stat().st_size>1000
    for terrain_type in [18,20,21]:
        for avoid,count,state in [(False,2053,405893282),(True,2055,2281347096)]:
            forest_players=work/f'cliff-player-forest-{terrain_type}-{avoid}.rms'
            forest_players.write_text(player_cliffs.read_text()+
                f' <TERRAIN_GENERATION> create_terrain {terrain_type} '+ '{ base_terrain 6 number_of_tiles 2000 '
                'number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 '+
                ('set_avoid_player_start_areas ' if avoid else '')+'}')
            output=work/f'cliff-player-forest-{terrain_type}-{avoid}.png'
            rendered=run(['render-rms',data,forest_players,120,1,output],0)
            assert f'{count} native objects, final RNG {state}' in rendered
            assert output.stat().st_size>1000
    for terrain_type in [18,20,21]:
        for avoid,count,state in [(False,2069,685587785),(True,2048,3019377153)]:
            output=work/f'cliff-fixed-player-forest-{terrain_type}-{avoid}.png'
            rendered=run(['render-rms',data,work/f'cliff-player-forest-{terrain_type}-{avoid}.rms',120,1,output,
                '--define','FIXED_POSITIONS'],0)
            assert f'{count} native objects, final RNG {state}' in rendered
            assert output.stat().st_size>1000
    cliff_owned=work/'cliff-owned.png'
    owned_cliffs=run(['render-rms',data,root/'tools/fixtures/cliff-owned-start.rms',120,1,cliff_owned,
        '--players',2,'--civilization',1,'--civilization',1],0)
    assert '6045 native objects, final RNG 2896867181' in owned_cliffs
    assert cliff_owned.stat().st_size>1000
    mixed_cliffs=run(['render-rms',data,root/'tools/fixtures/cliff-owned-start.rms',120,1,work/'cliff-mixed.png',
        '--players',2,'--civilization',1,'--civilization',4],0)
    assert '6045 native objects, final RNG 266867314' in mixed_cliffs
    assert (work/'cliff-mixed.png').stat().st_size>1000
    resources_cliff=run(['render-rms',data,root/'tools/fixtures/cliff-resource-start.rms',120,12345,work/'cliff-resources.png',
        '--players',2,'--civilization',1,'--civilization',1],0)
    assert '6100 native objects, final RNG 146046165' in resources_cliff
    assert (work/'cliff-resources.png').stat().st_size>1000
    run(['render-rms',data,root/'tools/fixtures/cliff-resource-start.rms',120,1,work/'cliff-resources-invalid.png',
        '--players',2,'--civilization',1,'--civilization',2],1)
    assert not (work/'cliff-resources-invalid.png').exists()
    full_cliff=run(['render-rms',data,root/'tools/fixtures/cliff-full-start.rms',120,1,work/'cliff-full.png',
        '--players',2,'--civilization',1,'--civilization',1],0)
    assert '6260 native objects, final RNG 3558270440' in full_cliff
    assert (work/'cliff-full.png').stat().st_size>1000
    mixed_full=run(['render-rms',data,root/'tools/fixtures/cliff-full-start.rms',120,1,work/'cliff-full-mixed.png',
        '--players',2,'--civilization',4,'--civilization',8],0)
    assert '6261 native objects, final RNG 1121268031' in mixed_full
    assert (work/'cliff-full-mixed.png').stat().st_size>1000
    fixed_full=run(['render-rms',data,root/'tools/fixtures/cliff-full-start.rms',120,1,work/'cliff-full-fixed.png',
        '--players',2,'--civilization',1,'--civilization',1,'--define','FIXED_POSITIONS'],0)
    assert '6202 native objects, final RNG 664476888' in fixed_full
    assert (work/'cliff-full-fixed.png').stat().st_size>1000
    fixed_mixed=run(['render-rms',data,root/'tools/fixtures/cliff-full-start.rms',120,1,work/'cliff-full-fixed-mixed.png',
        '--players',2,'--civilization',4,'--civilization',8,'--define','FIXED_POSITIONS'],0)
    assert '6203 native objects, final RNG 3105553768' in fixed_mixed
    assert (work/'cliff-full-fixed-mixed.png').stat().st_size>1000
    for levels,tiles,clumps,count,state in [(3,400,8,6215,4040677865),(7,8000,30,6239,272038666)]:
        elevated_full=work/f'cliff-full-hills-{levels}.rms'
        elevated_full.write_text((root/'tools/fixtures/cliff-full-start.rms').read_text()+
            f'\n<ELEVATION_GENERATION> create_elevation {levels} {{ base_terrain 6 number_of_tiles {tiles} number_of_clumps {clumps} }}')
        output=work/f'cliff-full-hills-{levels}.png'
        rendered=run(['render-rms',data,elevated_full,120,1,output,
            '--players',2,'--civilization',1,'--civilization',1,'--define','FIXED_POSITIONS'],0)
        assert f'{count} native objects, final RNG {state}' in rendered
        assert output.stat().st_size>1000
        for first,second,fixed in [(1,2,True),(1,1,False)]:
            rejected=work/f'cliff-full-hills-unverified-{levels}-{first}-{second}-{fixed}.png'
            run(['render-rms',data,elevated_full,120,1,rejected,
                '--players',2,'--civilization',first,'--civilization',second]+
                (['--define','FIXED_POSITIONS'] if fixed else []),1)
            assert not rejected.exists()
    unverified_cliff=work/'cliff-full-unverified.rms'
    unverified_cliff.write_text((root/'tools/fixtures/cliff-full-start.rms').read_text().replace(
        'set_loose_grouping number_of_objects 4','set_loose_grouping number_of_objects 8',1))
    run(['render-rms',data,unverified_cliff,120,1,work/'cliff-full-unverified.png',
        '--players',2,'--civilization',1,'--civilization',1],1)
    assert not (work/'cliff-full-unverified.png').exists()
    cliff_grass=work/'cliff-grass.rms'
    cliff_grass.write_text((root/'tools/fixtures/cliff-diagnostic.rms').read_text()+
        '<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 30 '
        'number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only }')
    painted=run(['render-rms',data,cliff_grass,120,1,work/'cliff-grass.png'],0)
    assert '29 native objects, final RNG 646082904' in painted
    assert (work/'cliff-grass.png').stat().st_size>1000
    cliff_elevation=work/'cliff-elevation.rms'
    cliff_elevation.write_text((root/'tools/fixtures/cliff-diagnostic.rms').read_text()+
        '<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 number_of_clumps 8 }')
    raised=run(['render-rms',data,cliff_elevation,120,1,work/'cliff-elevation.png'],0)
    assert '29 native objects, final RNG 2790043587' in raised
    assert (work/'cliff-elevation.png').stat().st_size>1000
    cliff_composed=work/'cliff-composed.rms'
    cliff_composed.write_text(cliff_elevation.read_text()+
        '<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 30 '
        'number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only }')
    combined=run(['render-rms',data,cliff_composed,120,1,work/'cliff-composed.png'],0)
    assert '29 native objects, final RNG 1669067597' in combined
    assert (work/'cliff-composed.png').stat().st_size>1000
    cliff_players_high=work/'cliff-players-high.rms'
    cliff_players_high.write_text('<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 1000 } '
        '<CLIFF_GENERATION> <ELEVATION_GENERATION> create_elevation 3 { base_terrain 6 number_of_tiles 400 number_of_clumps 8 }')
    players_high=run(['render-rms',data,cliff_players_high,120,1,work/'cliff-players-high.png'],0)
    assert '35 native objects, final RNG 1128436093' in players_high
    assert (work/'cliff-players-high.png').stat().st_size>1000
    cliff_players_high.write_text(cliff_players_high.read_text().replace('create_elevation 3','create_elevation 7')
        .replace('number_of_tiles 400 number_of_clumps 8','number_of_tiles 8000 number_of_clumps 30'))
    fixed_high=run(['render-rms',data,cliff_players_high,120,1,work/'cliff-fixed-high.png','--define','FIXED_POSITIONS'],0)
    assert '28 native objects, final RNG 4066850008' in fixed_high
    assert (work/'cliff-fixed-high.png').stat().st_size>1000
    cliff_high=work/'cliff-high.rms'
    cliff_high.write_text((root/'tools/fixtures/cliff-diagnostic.rms').read_text()+
        '<ELEVATION_GENERATION> create_elevation 7 { base_terrain 6 number_of_tiles 8000 number_of_clumps 30 }')
    high=run(['render-rms',data,cliff_high,120,1,work/'cliff-high.png'],0)
    assert '33 native objects, final RNG 2215531909' in high
    assert (work/'cliff-high.png').stat().st_size>1000
    for terrain_type in [18,20,21]:
        cliff_forest=work/f'cliff-forest-{terrain_type}.rms'
        cliff_forest.write_text((root/'tools/fixtures/cliff-diagnostic.rms').read_text()+
            f'<TERRAIN_GENERATION> create_terrain {terrain_type} '+ '{ base_terrain 6 number_of_tiles 2000 '
            'number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 }')
        wooded=run(['render-rms',data,cliff_forest,120,1,work/f'cliff-forest-{terrain_type}.png'],0)
        assert '2065 native objects, final RNG 1567879473' in wooded
        assert (work/f'cliff-forest-{terrain_type}.png').stat().st_size>1000

    rendered=run(['render-rms',data,work/'map.rms',32,1,work/'file.png'],0)
    assert 'randomized-position mode' in rendered
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
    fixed = run([*base,'--define','TEST_MAP','--define','FIXED_POSITIONS'],0)
    assert 'fixed-position mode' in fixed
    assert (work/'archive.png').read_bytes()!=two_player_image, 'Fixed setting used randomized starts'
    (work/'archive.png').unlink()
    run([*base,'--define'],2)
    for invalid in [0,9,-1,'bad']:
        run([*base,'--players',invalid],2)
    run([*base,'--players'],2)
    run(['render-rms-resource',data,work/'test.drs',-1,32,1,work/'archive.png'],2)
    run(['render-rms-resource',data,work/'test.drs',60002,32,1,work/'archive.png'],1)
    assert not (work/'archive.png').exists(), 'Rejected input created output'
    (work/'invalid.rms').write_text(source.replace('base_terrain 0','base_terrain 256'))
    assert 'base_terrain' in run(
        ['render-rms',data,work/'invalid.rms',32,1,work/'rejected.png'],1)
    assert not (work/'rejected.png').exists(), 'Wrapped terrain ID created output'
    for directive, original in [('base_terrain', 0), ('terrain_type', 3)]:
        for value in [-256, 256, 259]:
            path=work/'invalid-land-terrain.rms'
            path.write_text(source.replace(f'{directive} {original}', f'{directive} {value}', 1))
            run(['render-rms',data,path,32,1,work/'rejected.png'],1)
            assert not (work/'rejected.png').exists(), 'Wrapped land terrain ID created output'
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


    # Original4e3850 defaults object quantity to1, including the shared CC/worker rows.
    explicit = (root/'tools/fixtures/owned-forest-cleanup-workers.rms').read_text()
    implicit = explicit.replace('number_of_objects 1', '')
    assert implicit != explicit
    results=[]
    for name,source in [('explicit-count',explicit),('default-count',implicit)]:
        path=work/(name+'.rms');path.write_text(source)
        output=run(['render-rms',data,path,32,1,work/(name+'.png'),
                    '--players',2,'--civilization',1,'--civilization',1],0)
        results.append(re.search(r'final RNG (\d+)',output).group(1))
        assert '45 native objects' in output
    assert results==['4221457422','4221457422'], 'Default quantity changed original RNG'
    assert (work/'explicit-count.png').read_bytes()==(work/'default-count.png').read_bytes(), 'Default quantity changed map or objects'
    for quantity in [0,2]:
        invalid=work/'invalid-default-count.rms'
        invalid.write_text(explicit.replace('number_of_objects 1',f'number_of_objects {quantity}',1))
        run(['render-rms',data,invalid,32,1,work/'invalid-default-count.png',
             '--players',2,'--civilization',1,'--civilization',1],1)
    assert not (work/'invalid-default-count.png').exists()


    output=run(['render-rms',data,root/'tools/fixtures/owned-scouts.rms',32,1,work/'scouts.png',
                '--players',2,'--civilization',1,'--civilization',1],0)
    assert '10 native objects' in output and '512005877' in output
    run(['render-rms',data,root/'tools/fixtures/owned-scouts.rms',32,1,work/'invalid-scouts.png',
         '--players',3,'--civilization',1,'--civilization',2,'--civilization',3],1)
    assert not (work/'invalid-scouts.png').exists()
    nova=work/'near-nova.rms'
    nova.write_text((root/'tools/fixtures/owned-scouts.rms').read_text()+'''
create_object 66 {
    number_of_objects 7
    set_place_for_every_player
    set_gaia_object_only
    set_tight_grouping
    min_distance_to_players 12
    max_distance_to_players 16
    min_distance_group_placement 7
    max_distance_to_other_zones 7
}
''')
    nova_args=['render-rms',data,nova,32,1,work/'nova.png','--players',2,
               '--civilization',1,'--civilization',1]
    output=run(nova_args,0)
    assert '24 native objects' in output and '517217389' in output
    (work/'nova.png').unlink()
    assert 'near-Nova' in run([*nova_args,'--define','FIXED_POSITIONS'],1)
    assert not (work/'nova.png').exists(), 'Unverified fixed-start Nova created output'


print('RMS file/DRS parity, includes, defines and rejection checks passed')
