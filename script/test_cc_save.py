#!/usr/bin/env python3
"""Authored save-prefix fixtures; no proprietary save data is embedded."""
import struct
import zlib
from inspect_cc_save import inspect_save


def compressed(data):
    encoder = zlib.compressobj(wbits=-15)
    return encoder.compress(data) + encoder.flush()


def fixture(ai=False, zone=False):
    data = bytearray(b'VER 9.4\0' + struct.pack('<fi', 1.1, int(ai)))
    if ai:
        data += struct.pack('<hhI', 0, 1, 0x1234)
        data += struct.pack('<i', 3) + b'abc'
        data += struct.pack('<hhh', 120, 100, 1)
        data += struct.pack('<IihhI', 1, 0, 0, 1, 0x1234) + bytes(0x250)
        data += struct.pack('<hhI', 0, 2, 0x5678) + bytes(4)
        data += struct.pack('<f', .14) + bytes(36 + 640 + 3072 + 2048)
    world_offset = len(data)
    world = bytearray(110)
    struct.pack_into('<IIhh', world, 37, 123, 456, 1, 3)
    data += world
    data += struct.pack('<iii', 2, 2, int(zone))
    if zone:
        data += bytes(255 + 1020 + 4) + struct.pack('<iiii', 2, 0, 1, 4)
    data += bytes(2) + bytes([6, 0, 21, 1, 27, 2, 6, 0])
    data += b'opaque remaining world records'
    return data, world_offset


for ai in (False, True):
    for zone in (False, True):
        data, world_offset = fixture(ai, zone)
        summary, terrain, elevation = inspect_save(compressed(data))
        assert summary['world_offset'] == world_offset
        assert summary['save_rng_checkpoint'] == 123
        assert summary['world_rng_checkpoint'] == 456
        assert summary['generation_seed'] is None
        assert summary['local_player'] == 1 and summary['players_including_gaia'] == 3
        assert terrain == bytes([6, 21, 27, 6]) and elevation == bytes([0, 1, 2, 0])
        assert len(summary['zones']) == int(zone)
        for end in (0, 7, 12, world_offset, summary['tile_offset'] + 7):
            try:
                inspect_save(compressed(data[:end]))
            except ValueError:
                pass
            else:
                raise AssertionError(f'truncation accepted: {ai}, {zone}, {end}')

data, world_offset = fixture()
invalid = []
for offset, value in [(8, struct.pack('<f', 1.0)), (12, struct.pack('<i', 2)),
                      (world_offset + 110, struct.pack('<i', -1)),
                      (world_offset + 118, struct.pack('<i', 100000))]:
    changed = data.copy()
    changed[offset:offset + len(value)] = value
    invalid.append(compressed(changed))
invalid += [compressed(data) + b'trailing', compressed(data)[:-1]]
for payload in invalid:
    try:
        inspect_save(payload)
    except (ValueError, zlib.error):
        pass
    else:
        raise AssertionError('invalid save accepted')
print('Clone Campaigns save-prefix fixtures and malformed-input checks passed')
