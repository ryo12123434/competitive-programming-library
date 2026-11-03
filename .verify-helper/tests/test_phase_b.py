from pathlib import Path
import os
import subprocess
import tempfile
import unittest
import sys
from test_snippets import load_snippets, expand

ROOT=Path(__file__).resolve().parents[2]

class PhaseBTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.binary=Path(cls.tmp.name)/'phase-b.exe'
        flags=['-fsanitize=address,undefined','-fno-sanitize-recover=all'] if os.name!='nt' else []
        p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,'-I',str(ROOT),
                          str(ROOT/'.verify-helper/tests/fixtures/phase_b.cpp'),'-o',str(cls.binary)],capture_output=True,text=True)
        if p.returncode:cls.tmp.cleanup();raise RuntimeError(p.stderr)
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def run_case(self,mode):
        p=subprocess.run([str(self.binary),mode],text=True,capture_output=True,timeout=120)
        self.assertEqual(p.returncode,0,p.stderr)
    def test_manacher(self):self.run_case('manacher')
    def test_xor_basis(self):self.run_case('basis')
    def test_binary_trie(self):self.run_case('trie')
    def test_rerooting(self):self.run_case('rerooting')

    def test_snippets_bundle_and_release(self):
        snippets=load_snippets()
        def code(prefix,values=None):return expand(snippets[prefix]['body'],values)
        headers=['string/manacher','math/xor_basis','data_structure/binary_trie','graph/rerooting']
        source=''.join(f'#include "library/{name}.hpp"\n' for name in headers)
        source+='#include <string>\nint main(){std::string s="abba";\n'+code('manacher')
        source+='\nif(!pal.is_palindrome(0,4))return 1;\n'+code('xor-basis')
        source+='\nbasis.insert(3);if(basis.max_xor()!=3)return 2;\n'+code('binary-trie')
        source+='\ntrie.insert(5);if(trie.min_xor(4)!=1)return 3;\n'
        source+='std::vector<std::vector<RerootingEdge>> tree{{{1,0}},{{0,0}}};\n'+code('rerooting')
        source+='\nif(dp[0].sum!=1)return 4;\n'+code('rerooting',{1:'State2',2:'weighted',4:'7LL'})
        source+='\nif(weighted[1].sum!=7)return 5;\n'+code('xor-basis',{1:'std::uint32_t',2:'other'})
        source+='\nother.insert(8);if(other.rank()!=1)return 6;\n'+code('binary-trie',{1:'std::uint32_t',2:'5',3:'small'})
        source+='\nsmall.insert(31);if(small.kth(0)!=31)return 7;\n'+code('manacher',{1:'again',2:'std::vector<int>{1,2,1}'})
        source+='\nif(!again.is_palindrome(0,3))return 8;return 0;}\n'
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);path=root/'main.cpp';path.write_text(source,encoding='utf-8')
            p=subprocess.run([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout'],capture_output=True,text=True)
            self.assertEqual(p.returncode,0,p.stderr)
            for src,flags in [(source,['-I',str(ROOT)]),(p.stdout,['-DNDEBUG'])]:
                binary=root/'snippet.exe'
                p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,'-x','c++','-','-o',str(binary)],input=src,capture_output=True,text=True,cwd=root)
                self.assertEqual(p.returncode,0,p.stderr)
                p=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
                self.assertEqual(p.returncode,0,p.stderr)

if __name__=='__main__':unittest.main()
