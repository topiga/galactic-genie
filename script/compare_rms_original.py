#!/usr/bin/env python3
"""Compare local original-engine captures with the six-column native RMS probe.

No original binaries, assets or captures belong in the repository. See RMS_PARITY.md.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import math
import os
from pathlib import Path
import subprocess


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def differences(actual, expected):
    """Count missing tails as differences too; zip alone can report false parity."""
    common = min(len(actual), len(expected))
    count = abs(len(actual) - len(expected))
    first = None
    for index in range(common):
        if actual[index] != expected[index]:
            count += 1
            if first is None:
                first = index
    if first is None and len(actual) != len(expected):
        first = common
    return dict(actualLength=len(actual), expectedLength=len(expected), count=count, first=first)


def compare(oracle, records, grids):
    lines = records.splitlines()
    if not lines or len(lines[0].split()) != 2 or lines[0].split()[0] != 'rng':
        raise ValueError('invalid native RNG header')
    rows = [[float(v) for v in line.split()] for line in lines[1:]]
    if any(len(row) not in (6, 10, 11) or not all(math.isfinite(v) for v in row) for row in rows):
        raise ValueError('invalid native object record')
    if len({row[0] for row in rows}) != len(rows):
        raise ValueError('duplicate native object IDs')
    expected = [[obj[key] for key in ('id', 'owner', 'unit', 'facet', 'x', 'y')]
                for obj in oracle['objects']]
    objects = differences([row[:6] for row in rows], expected)
    if objects['first'] is not None:
        index = objects['first']
        objects['actual'] = rows[index][:6] if index < len(rows) else None
        objects['expected'] = expected[index] if index < len(expected) else None
    by_id = {row[0]: row for row in rows}
    poses = []
    checked = 0
    for key in ('poses', 'gaiaPoses'):
        for pose in oracle[key]:
            checked += 1
            row = by_id.get(pose['id'])
            owned = key == 'poses'
            offset = 7 if owned else 6
            if row is None or len(row) != (11 if owned else 10):
                poses.append([pose['id'], 'missing pose'])
                continue
            if owned and row[6] != pose['pendingFoundation']:
                poses.append([pose['id'], 'foundation'])
            if int(row[offset + 3]) & 1:
                if row[offset] != pose['frame'] or abs(row[offset + 1] + row[offset + 2] - pose['hold']) > 1e-5:
                    poses.append([pose['id'], 'animation'])
            elif row[offset] != 0 or row[offset + 1] != 0:
                poses.append([pose['id'], 'static animation'])
    result = dict(objects=objects, rng=dict(actual=int(lines[0].split()[1]), expected=oracle['rng']),
                  grids={key: differences(list(value), oracle[key]) for key, value in grids.items()},
                  poses=dict(checked=checked, differences=poses))
    result['matches'] = (not objects['count'] and result['rng']['actual'] == oracle['rng']
                         and not poses and not any(v['count'] for v in result['grids'].values()))
    return result


def run_case(case, args):
    work = args.output / case['id']
    work.mkdir()
    result = dict(case=case, status='error')
    try:
        probe = Path(case['probe'])
        oracle_path = Path(case['oracle'])
        result['probeSha256'] = digest(probe)
        if not args.existing:
            before = oracle_path.stat().st_mtime_ns if oracle_path.exists() else None
            env = dict(os.environ, SWGB_CLIFF_SEED=str(case['seed']), SWGB_OWNER_CIVS=','.join(map(str, case['civilizations'])))
            env.pop('SWGB_PARSE_ONLY', None)
            with (work / 'original.log').open('w') as log:
                run = subprocess.run([str(args.python), str(probe)], env=env, stdout=log,
                                     stderr=subprocess.STDOUT, timeout=args.timeout)
            if run.returncode:
                result.update(status='original_error', exitCode=run.returncode)
                return result
            if not oracle_path.exists() or oracle_path.stat().st_mtime_ns == before:
                raise ValueError('original run did not produce a fresh oracle')
        oracle = json.loads(oracle_path.read_text())
        if oracle['seed'] != case['seed'] or oracle['civs'] != case['civilizations'] or oracle['probeSha256'] != result['probeSha256']:
            raise ValueError('oracle seed, civilizations or probe hash does not match requested case')
        result['sourceSha256'] = digest(case['source'])
        if oracle['sourceFileSha256'] != result['sourceSha256']:
            raise ValueError('oracle RMS source hash does not match requested resource')
        original_fixed = any(v['name'] == 'FIXED_POSITIONS' for v in oracle['presetRegistrations'])
        if original_fixed != (case['positions'] == 'fixed'):
            raise ValueError('oracle placement mode does not match requested case')
        result['oracleSha256'] = digest(oracle_path)
        result['originalExeSha256'] = oracle['exeSha256']
        result['originalLimitations'] = oracle.get('limitations', [])
        if digest(args.native) != args.native_sha:
            raise ValueError('native executable changed during matrix')
        terrain = work / 'terrain.bin'
        command = [str(args.native), str(args.data / 'genie_x1.dat'), str(case['seed']),
                   str(terrain), *map(str, case['civilizations']), str(args.data / 'gamedata_x1.drs'), str(case['resource']), 'tiny' if case['positions'] == 'fixed' else 'tiny-random']
        with (work / 'native.records').open('w') as records, (work / 'native.log').open('w') as log:
            run = subprocess.run(command, stdout=records, stderr=log, timeout=args.timeout)
        if digest(args.native) != args.native_sha:
            raise ValueError('native executable changed during case')
        if run.returncode:
            detail = (work / 'native.log').read_text()
            rejected = run.returncode == 1 and any(word in detail for word in ('unsupported', 'unverified', 'requires', 'verified'))
            result.update(status='native_rejected' if rejected else 'native_error', exitCode=run.returncode, diagnostic=detail)
            return result
        grids = {key: Path(str(terrain) + suffix).read_bytes()
                 for key, suffix in [('terrain', ''), ('heights', '.heights'), ('slopes', '.slopes')]}
        if any(len(oracle[key]) != 120 * 120 for key in grids):
            raise ValueError('oracle does not contain complete Tiny120 grids')
        result['comparison'] = compare(oracle, (work / 'native.records').read_text(), grids)
        result['status'] = 'match' if result['comparison']['matches'] else 'mismatch'
    except (OSError, ValueError, KeyError, TypeError, IndexError, subprocess.TimeoutExpired) as error:
        result['diagnostic'] = str(error)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('manifest', 'output', 'native', 'data', 'python'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--jobs', type=int, choices=(1, 2), default=2)
    parser.add_argument('--timeout', type=int, default=600)
    parser.add_argument('--existing', action='store_true', help='Use matching saved captures; do not execute original code')
    args = parser.parse_args()
    for name in ('manifest', 'native', 'data', 'python'):
        setattr(args, name, getattr(args, name).absolute())
    cases = json.loads(args.manifest.read_text())
    for case in cases:
        case.setdefault('positions', 'fixed')
        case.setdefault('civilizations', [1, 1])
        if not isinstance(case['civilizations'], list) or len(case['civilizations']) != 2 or any(type(c) is not int or not 1 <= c <= 8 for c in case['civilizations']):
            parser.error('this full-map adapter requires two civilizations in1..8')
        if case['positions'] not in ('fixed', 'random'):
            parser.error('positions must be fixed or random')
    ids = [case['id'] for case in cases]
    if not cases or len(ids) != len(set(ids)) or any(not name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-_' for c in name) for name in ids):
        parser.error('case IDs must be unique safe names')
    if any(type(c['seed']) is not int or not 0 <= c['seed'] <= 0xffffffff for c in cases):
        parser.error('seeds must be uint32')
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    args.native_sha = digest(args.native)
    report = dict(context='Tiny120, default resources, two enemy players; civilizations and placement mode per case',
                  limitations=['Full-map adapter remains Tiny120/two players; only listed civilizations and modes are tested.',
                               'Compares ordered object identity/XY/facet, grids, final RNG and captured initial poses.',
                               'Does not compare HP, absolute object height, runtime gameplay or UI.'],
                  nativeSha256=args.native_sha, manifestSha256=digest(args.manifest),
                  dataSha256={name: digest(args.data / name) for name in ('genie_x1.dat', 'gamedata_x1.drs')}, cases=[])
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for future in as_completed([pool.submit(run_case, case, args) for case in cases]):
            result = future.result()
            report['cases'].append(result)
            report['cases'].sort(key=lambda row: row['case']['id'])
            report['counts'] = dict(Counter(row['status'] for row in report['cases']))
            temp = args.output / 'report.tmp'
            temp.write_text(json.dumps(report, indent=2) + '\n')
            temp.replace(args.output / 'report.json')
            print(result['case']['id'], result['status'], flush=True)
    return 0 if report['counts'].get('match') == len(cases) else 1


if __name__ == '__main__':
    raise SystemExit(main())
