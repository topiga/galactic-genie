#!/usr/bin/env python3
"""Read the verified Clone Campaigns 1.1 save prefix and export its tile bytes."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import zlib

LIMIT = 64 * 1024 * 1024


class Reader:
    def __init__(self, data):
        self.data, self.pos = data, 0

    def take(self, size):
        if size < 0 or size > len(self.data) - self.pos:
            raise ValueError(f'truncated or invalid record at {self.pos}')
        start = self.pos
        self.pos += size
        return self.data[start:self.pos]

    def number(self, fmt):
        return struct.unpack('<' + fmt, self.take(struct.calcsize('<' + fmt)))[0]


def inspect_save(compressed):
    if len(compressed) > LIMIT:
        raise ValueError('save exceeds the 64 MiB inspection limit')
    decoder = zlib.decompressobj(-15)
    data = decoder.decompress(compressed, LIMIT + 1)
    if len(data) > LIMIT or not decoder.eof or decoder.unused_data:
        raise ValueError('oversized, incomplete or trailing DEFLATE stream')
    r = Reader(data)
    if r.take(8) != b'VER 9.4\0' or r.take(4) != struct.pack('<f', 1.1):
        raise ValueError('only the verified Clone Campaigns 1.1 save format is supported')
    has_ai = r.number('i')
    if has_ai not in (0, 1):
        raise ValueError('invalid AI block flag')
    if has_ai:
        # 5fe970 -> 410d70 -> 40f230: serialized AI records, including opaque pointers.
        header = r.take(8)
        strings = struct.unpack_from('<h', header, 2)[0]
        if strings < 0:
            raise ValueError('negative AI string count')
        for _ in range(strings):
            r.take(r.number('i'))
        r.take(4)
        count = r.number('h')
        if not 0 <= count <= 8:
            raise ValueError('unsupported AI record count')
        active = []
        for _ in range(count):
            record = r.take(16)
            enabled = struct.unpack_from('<I', record)[0]
            entries = struct.unpack_from('<h', record, 10)[0]
            pointer = struct.unpack_from('<I', record, 12)[0]
            active.append(enabled)
            if entries < 0:
                raise ValueError('negative AI entry count')
            if enabled and pointer:
                r.take(entries * 0x250)
        for enabled in active:
            record = r.take(8)
            entries, pointer = struct.unpack_from('<hI', record, 2)
            if entries < 0:
                raise ValueError('negative AI list count')
            if enabled and pointer:
                r.take(entries * 2)
        # 5817f0 -> 418190: fixed globals and eight 20-word arrays.
        if r.take(4) != struct.pack('<f', .14):
            raise ValueError('unsupported AI serialization version')
        r.take(36 + 640 + 3072 + 2048)

    world_offset = r.pos
    world = r.take(110)  # 61ffa0: exact prefix before map serializer 494560.
    map_offset = r.pos
    width, height, zones = r.number('i'), r.number('i'), r.number('i')
    if not (1 <= width <= 1024 and 1 <= height <= 1024 and 0 <= zones <= 64):
        raise ValueError('unsupported map dimensions or zone-cache count')
    zone_records = []
    for _ in range(zones):
        r.take(255 + 1020 + width * height)  # 625c70: labels, table and cell IDs.
        count = r.number('i')
        if not 0 <= count <= 256:
            raise ValueError('unsupported zone restriction list length')
        r.take(count * 4)
        zone_records.append({'restriction_entries': count, 'key': r.number('i')})
    r.take(2)
    tile_offset = r.pos
    tiles = r.take(width * height * 2)
    terrain, elevation = tiles[::2], tiles[1::2]
    summary = {
        'compressed_sha256': hashlib.sha256(compressed).hexdigest(),
        'inflated_sha256': hashlib.sha256(data).hexdigest(),
        'inflated_bytes': len(data), 'world_offset': world_offset,
        'map_offset': map_offset, 'tile_offset': tile_offset,
        'width': width, 'height': height, 'zones': zone_records,
        'local_player': struct.unpack_from('<h', world, 45)[0],
        'players_including_gaia': struct.unpack_from('<h', world, 47)[0],
        'save_rng_checkpoint': struct.unpack_from('<I', world, 37)[0],
        'world_rng_checkpoint': struct.unpack_from('<I', world, 41)[0],
        'generation_seed': None,
        'terrain_sha256': hashlib.sha256(terrain).hexdigest(),
        'elevation_sha256': hashlib.sha256(elevation).hexdigest(),
        'terrain_counts': dict(sorted(Counter(terrain).items())),
        'elevation_counts': dict(sorted(Counter(elevation).items())),
        'scope': 'Map prefix only; objects and later records are not decoded. '
                 'Saved RNG checkpoints are not the initial generation seed.',
    }
    return summary, terrain, elevation


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('save', type=Path)
    parser.add_argument('--output-directory', type=Path, help='New directory for raw grids and JSON')
    args = parser.parse_args()
    try:
        with args.save.open('rb') as stream:
            summary, terrain, elevation = inspect_save(stream.read(LIMIT + 1))
        result = json.dumps(summary, indent=2) + '\n'
        if args.output_directory:
            args.output_directory.mkdir(parents=True, exist_ok=False)
            (args.output_directory / 'terrain.bin').write_bytes(terrain)
            (args.output_directory / 'elevation.bin').write_bytes(elevation)
            (args.output_directory / 'summary.json').write_text(result)
        print(result, end='')
    except (OSError, ValueError, zlib.error) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
