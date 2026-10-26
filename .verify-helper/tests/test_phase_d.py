from pathlib import Path
import os
import subprocess
import tempfile
import unittest
import sys
from test_snippets import load_snippets, expand

ROOT=Path(__file__).resolve().parents[2]
class PhaseDTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.binary=Path(cls.tmp.name)/'phase-d.exe'
        flags=['-fsanitize=address,undefined','-fno-sanitize-recover=all'] if os.name!='nt' else []
        p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,'-I',str(ROOT),
                          str(ROOT/'.verify-helper/tests/fixtures/phase_d.cpp'),'-o',str(cls.binary)],capture_output=True,text=True)
        if p.returncode:cls.tmp.cleanup();raise RuntimeError(p.stderr)
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def run_case(self,mode):
        p=subprocess.run([str(self.binary),mode],capture_output=True,text=True,timeout=180)
        self.assertEqual(p.returncode,0,p.stderr)
    def test_wavelet(self):self.run_case('wavelet')
    def test_rollback(self):self.run_case('rollback')
    def test_dynamic_connectivity(self):self.run_case('dynamic')

    def test_snippets_release_and_bundle(self):
        snippets=load_snippets()
        def code(prefix,values=None):return expand(snippets[prefix]['body'],values)
        source=('#include "library/data_structure/wavelet_matrix.hpp"\n'
                '#include "library/data_structure/rollback_union_find.hpp"\n'
                '#include "library/string/manacher.hpp"\n#include "library/math/xor_basis.hpp"\n'
                '#include "library/data_structure/binary_trie.hpp"\n#include "library/graph/rerooting.hpp"\n'
                'int main(){std::vector<long long> values{-3,2,-3};\n')
        source+=code('wavelet')+'\nif(matrix.kth_smallest(0,3,1)!=-3)return 1;\n'
        source+='int n=3,u=0,v=1;\n'+code('rollback-uf',{0:'if(!uf.same(u,v))return 2;'})
        source+='\nif(uf.same(u,v))return 3;\n'+code('wavelet',{1:'std::uint64_t',2:'wide',3:'std::vector<std::uint64_t>{0,UINT64_MAX}'})
        source+='\nif(wide.kth_largest(0,2,0)!=UINT64_MAX)return 4;\n'+code('rollback-uf',{1:'other',2:'2',3:'checkpoint',4:'0',5:'1',0:'if(other.components()!=1)return 5;'})
        source+='\nif(other.components()!=2)return 6;return 0;}\n'
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
