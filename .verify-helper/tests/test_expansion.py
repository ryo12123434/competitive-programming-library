from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ExpansionFixture(unittest.TestCase):
    phase = None

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.tmp.name) / 'regression.exe'
        flags = ['-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'] if os.name != 'nt' else []
        if sys.platform.startswith('linux'):
            flags += ['-no-pie']
        result = subprocess.run(['g++', '-std=gnu++23', '-O2', '-Wall', '-Wextra', '-Werror', *flags,
                                 '-I', str(ROOT), str(ROOT / f'.verify-helper/tests/fixtures/expansion_{cls.phase}.cpp'),
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


class ExpansionATest(ExpansionFixture):
    phase = 'a'

    def test_beats(self): self.run_case('beats')
    def test_persistent(self): self.run_case('persistent')
    def test_large(self): self.run_case('large')


class ExpansionBTest(ExpansionFixture):
    phase = 'b'

    def test_kmp_booth(self): self.run_case('kmp_booth')
    def test_aho(self): self.run_case('aho')
    def test_sam(self): self.run_case('sam')
    def test_large(self): self.run_case('large')


class ExpansionCTest(ExpansionFixture):
    phase = 'c'

    def test_dynamic(self): self.run_case('dynamic')
    def test_virtual_dsu(self): self.run_case('virtual_dsu')
    def test_components(self): self.run_case('components')
    def test_large(self): self.run_case('large')


class ExpansionDTest(ExpansionFixture):
    phase = 'd'

    def test_swag(self): self.run_case('swag')
    def test_disjoint(self): self.run_case('disjoint')
    def test_large(self): self.run_case('large')


if __name__ == '__main__':
    unittest.main()
