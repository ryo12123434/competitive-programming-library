from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from test_snippets import load_snippets, expand

ROOT = Path(__file__).resolve().parents[2]
HEADERS = [
    'data_structure/swag', 'data_structure/disjoint_sparse_table',
    'data_structure/segment_tree_beats', 'data_structure/persistent_segment_tree',
    'graph/offline_dynamic_connectivity', 'graph/virtual_tree', 'graph/dsu_on_tree',
    'graph/two_edge_connected_components',
    'string/kmp', 'string/aho_corasick', 'string/suffix_automaton', 'string/booth',
]
WRAPPERS = {
    'swag': ('long long value=7;', 'if(aggregate!=7)return 11;'),
    'disjoint-table': ('std::vector<long long>a{2,3,4};int n=3;', 'if(aggregate!=9)return 12;'),
    'dynamic-connect': ('int n=2,u=0,v=1;', 'if(answers[id]!=1)return 7;'),
    'virtual-tree': ('std::vector<std::vector<int>>tree{{1,2},{0},{0}};std::vector<int>selected{1,2};', 'if(compressed.vertices.size()!=3)return 8;'),
    'dsu-tree': ('int n=3;std::vector<std::vector<int>>tree{{1,2},{0},{0}};std::vector<int>color{0,0,1};', 'if(answer[0]!=2||distinct!=0)return 9;'),
    'two-edge': ('Lowlink graph(2);graph.add_edge(0,1);', 'if(components.component_count()!=2)return 10;'),
    'beats': ('std::vector<long long>a{1,5,3};int n=3;long long upper=3;', 'if(sum!=7)return 1;'),
    'persistent-seg': ('std::vector<long long>a{1,5,3};int n=3,index=1;long long value=8;', 'if(answer!=12||seg.all_prod(0)!=9)return 2;'),
    'kmp': ('std::string text="ababa",pattern="aba";', 'if(positions!=std::vector<int>{0,2})return 3;'),
    'aho': ('std::vector<std::string>patterns{"aba","","aba"};std::string text="ababa";', 'if(counts!=std::vector<long long>{2,6,2})return 4;'),
    'sam': ('std::string s="ababa";', 'if(distinct!=9||sam.occurrences("aba")!=2)return 5;'),
    'booth': ('std::string s="baba";', 'if(start!=1)return 6;'),
}


class ExpansionIntegrationTest(unittest.TestCase):
    def compile_run(self, source, directory, *flags):
        exe = Path(directory) / 'example.exe'
        result = subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,
                                 '-x','c++','-','-o',str(exe)], input=source, capture_output=True, text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        result = subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
        self.assertEqual(result.returncode,0,result.stderr)

    def test_headers_and_multiple_translation_units(self):
        includes = ''.join(f'#include "{ROOT}/library/{h}.hpp"\n' for h in HEADERS)
        for header in HEADERS:
            with self.subTest(header=header):
                result = subprocess.run(['g++','-std=gnu++23','-Wall','-Wextra','-Werror','-DNDEBUG',
                                         '-fsyntax-only','-x','c++','-'],
                                        input=f'#include "{ROOT}/library/{header}.hpp"\n',capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'first.cpp').write_text(includes+'int other();int main(){return other();}\n')
            (root/'second.cpp').write_text(includes+'int other(){return 0;}\n')
            result = subprocess.run(['g++','-std=gnu++23','-Wall','-Wextra','-Werror',str(root/'first.cpp'),
                                     str(root/'second.cpp'),'-o',str(root/'multi.exe')],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            subprocess.run([str(root/'multi.exe')],check=True)

    def test_doxygen_examples(self):
        with tempfile.TemporaryDirectory() as directory:
            for header in HEADERS:
                blocks = re.findall(r'@code\s*\n(.*?)@endcode',(ROOT/f'library/{header}.hpp').read_text(encoding='utf-8'),re.S)
                self.assertTrue(blocks,header)
                for block in blocks:
                    with self.subTest(header=header):
                        source = re.sub(r'^\s*\* ?', '', block, flags=re.M).rstrip()
                        self.compile_run(source,directory,'-I',str(ROOT))

    def test_snippets_and_bundle(self):
        snippets = load_snippets()
        source = ''.join(f'#include "library/{h}.hpp"\n' for h in HEADERS)
        source += '\n#include <string>\nlong long op(long long a,long long b){return a+b;}long long e(){return 0;}\nint main(){\n'
        for prefix,(before,after) in WRAPPERS.items():
            source += '{\n'+before+'\n'+expand(snippets[prefix]['body'])+'\n'+after+'\n}\n'
        source += '}\n'
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'source.cpp';path.write_text(source,encoding='utf-8')
            self.compile_run(source,directory,'-I',str(ROOT))
            result = subprocess.run([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout'],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertFalse(any(line.startswith('#include "library/') for line in result.stdout.splitlines()))
            self.compile_run(result.stdout,directory,'-DNDEBUG')


if __name__ == '__main__':
    unittest.main()
