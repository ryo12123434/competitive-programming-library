from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class GapsFixture(unittest.TestCase):
    phase = None

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.tmp.name) / 'regression.exe'
        flags = ['-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'] if os.name != 'nt' else []
        if sys.platform.startswith('linux'):
            flags += ['-no-pie']
        result = subprocess.run(['g++', '-std=gnu++23', '-O2', '-Wall', '-Wextra', '-Werror', *flags,
                                 '-I', str(ROOT), str(ROOT / f'.verify-helper/tests/fixtures/gaps_{cls.phase}.cpp'),
                                 '-o', str(cls.binary)], capture_output=True, text=True)
        if result.returncode:
            cls.tmp.cleanup()
            raise RuntimeError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def run_case(self, mode):
        result = subprocess.run([str(self.binary), mode], capture_output=True, text=True, timeout=120)
        self.assertEqual(result.returncode, 0, result.stderr)


class GapsATest(GapsFixture):
    phase = 'a'
    def test_mst(self): self.run_case('mst')
    def test_walks(self): self.run_case('walks')
    def test_floyd(self): self.run_case('floyd')
    def test_sequences(self): self.run_case('sequences')
    def test_large(self): self.run_case('large')


class GapsBTest(GapsFixture):
    phase = 'b'
    def test_slope(self): self.run_case('slope')
    def test_optimizations(self): self.run_case('optimizations')
    def test_gf2(self): self.run_case('gf2')
    def test_knapsack(self): self.run_case('knapsack')
    def test_large(self): self.run_case('large')


class GapsCTest(GapsFixture):
    phase = 'c'
    def test_fenwick(self): self.run_case('fenwick')
    def test_range_tree(self): self.run_case('range_tree')
    def test_parallel(self): self.run_case('parallel')
    def test_mo(self): self.run_case('mo')
    def test_large(self): self.run_case('large')
