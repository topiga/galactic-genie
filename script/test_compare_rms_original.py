#!/usr/bin/env python3
"""Checks for false-positive parity reports; no game assets required."""
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from compare_rms_original import compare, differences


class ComparisonTests(unittest.TestCase):
    def setUp(self):
        self.oracle = dict(rng=42, objects=[dict(id=0, owner=1, unit=83, facet=0, x=1.5, y=2.5)],
                           terrain=[0, 1], heights=[0, 0], slopes=[0, 0],
                           poses=[dict(id=0, pendingFoundation=255, frame=2, hold=0.5)], gaiaPoses=[])
        self.records = 'rng 42\n0 1 83 0 1.5 2.5 255 2 0.4 0.1 7\n'
        self.grids = dict(terrain=bytes([0, 1]), heights=bytes(2), slopes=bytes(2))

    def test_exact(self):
        self.assertTrue(compare(self.oracle, self.records, self.grids)['matches'])

    def test_missing_object_tail(self):
        self.assertFalse(compare(self.oracle, 'rng 42\n', self.grids)['matches'])

    def test_extra_object_tail(self):
        self.assertFalse(compare(self.oracle, self.records + '1 0 348 0 0.5 0.5\n', self.grids)['matches'])

    def test_missing_grid_tail(self):
        self.grids['terrain'] = bytes([0])
        self.assertFalse(compare(self.oracle, self.records, self.grids)['matches'])

    def test_rng_and_first_difference(self):
        self.assertFalse(compare(self.oracle, self.records.replace('rng 42', 'rng 43'), self.grids)['matches'])
        self.assertEqual(differences([1, 2], [1, 3])['first'], 1)

    def test_pose_and_foundation(self):
        for field, value in [('frame', 3), ('hold', 1), ('pendingFoundation', 0)]:
            oracle = copy.deepcopy(self.oracle)
            oracle['poses'][0][field] = value
            self.assertFalse(compare(oracle, self.records, self.grids)['matches'])

    def test_malformed_and_duplicate(self):
        for records in ('', 'rng 42\n0 1\n', self.records + self.records.splitlines()[1] + '\n', self.records.replace('1.5', 'nan')):
            with self.assertRaises(ValueError):
                compare(self.oracle, records, self.grids)


class RunnerTests(unittest.TestCase):
    def test_saved_capture_and_stale_source(self):
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            source = work/'map.rms';source.write_text('original fixture')
            probe = work/'probe.py';probe.write_text('raise SystemExit(1)')
            native = work/'native'
            native.write_text('#!' + sys.executable + '\nfrom pathlib import Path\nimport sys\n'
                              'assert sys.argv[4] in ("1,2", "1,2,3")\n'
                              'p=Path(sys.argv[3])\n'
                              'for suffix in ("", ".heights", ".slopes"): Path(str(p)+suffix).write_bytes(bytes(int(sys.argv[7])*int(sys.argv[8])))\n'
                              'print("rng 42")\n')
            native.chmod(0o700)
            for name in ('genie_x1.dat', 'gamedata_x1.drs'):(work/name).write_bytes(b'fixture')
            oracle = work/'oracle.json'
            oracle.write_text(json.dumps(dict(seed=42, civs=[1,2], probeSha256=hashlib.sha256(probe.read_bytes()).hexdigest(),
                presetRegistrations=[dict(name='FIXED_POSITIONS')], sourceFileSha256=hashlib.sha256(source.read_bytes()).hexdigest(), exeSha256='fixture', rng=42,
                objects=[], poses=[], gaiaPoses=[], terrain=[0]*14400, heights=[0]*14400, slopes=[0]*14400)))
            manifest = work/'manifest.json'
            manifest.write_text(json.dumps([dict(id='fixture', resource=1, seed=42, probe=str(probe),
                                                 source=str(source), oracle=str(oracle))]))
            command = [sys.executable, str(Path(__file__).with_name('compare_rms_original.py')),
                       '--manifest', str(manifest), '--native', str(native), '--data', str(work),
                       '--python', sys.executable, '--existing']
            original_capture=json.loads(oracle.read_text())
            for problem in ('good', 'source', 'mode', 'civilizations', 'dimensions', 'larger', 'moreplayers', 'wrongcount'):
                stale = problem not in ('good', 'larger', 'moreplayers')
                source.write_text('changed' if problem == 'source' else 'original fixture')
                cases = json.loads(manifest.read_text())
                cases[0]['positions'] = 'random' if problem == 'mode' else 'fixed'
                cases[0]['civilizations'] = [2,1] if problem == 'civilizations' else [1,2]
                cases[0]['width'] = cases[0]['height'] = 144 if problem in ('dimensions', 'larger') else 120
                capture=copy.deepcopy(original_capture)
                if problem in ('moreplayers', 'wrongcount'):
                    cases[0]['civilizations']=capture['civs']=[1,2,3]
                    capture['playerCount']=3 if problem=='moreplayers' else 2
                if problem == 'larger':
                    capture.update(width=144,height=144,playerCount=2)
                    for key in ('terrain','heights','slopes'):capture[key]=[0]*(144*144)
                oracle.write_text(json.dumps(capture))
                manifest.write_text(json.dumps(cases))
                output = work/problem
                run = subprocess.run(command + ['--output', str(output)], capture_output=True, text=True)
                self.assertEqual(run.returncode, int(stale), run.stderr)
                report = json.loads((output/'report.json').read_text())
                self.assertEqual(report['cases'][0]['status'], 'error' if stale else 'match')


    def test_invalid_civilizations(self):
        with tempfile.TemporaryDirectory() as directory:
            work=Path(directory);manifest=work/'cases.json'
            for civs in (None, [], [1]*9, [1, 9], [True, 1], '12'):
                manifest.write_text(json.dumps([dict(id='bad', seed=1, civilizations=civs)]))
                run=subprocess.run([sys.executable,str(Path(__file__).with_name('compare_rms_original.py')),
                    '--manifest',str(manifest),'--output',str(work/'out'),'--native','missing',
                    '--data','missing','--python',sys.executable],capture_output=True,text=True)
                self.assertEqual(run.returncode,2,run.stderr)
                self.assertFalse((work/'out').exists())

    def test_invalid_dimensions(self):
        with tempfile.TemporaryDirectory() as directory:
            work=Path(directory);manifest=work/'cases.json'
            for size in (0, 1025, True, '144', 144.5):
                manifest.write_text(json.dumps([dict(id='bad', seed=1, width=size)]))
                run=subprocess.run([sys.executable,str(Path(__file__).with_name('compare_rms_original.py')),
                    '--manifest',str(manifest),'--output',str(work/'out'),'--native','missing',
                    '--data','missing','--python',sys.executable],capture_output=True,text=True)
                self.assertEqual(run.returncode,2,run.stderr)
                self.assertFalse((work/'out').exists())


if __name__ == '__main__':
    unittest.main()
