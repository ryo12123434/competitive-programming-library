from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from test_snippets import load_snippets, expand

ROOT = Path(__file__).resolve().parents[2]
HEADERS = ['data_structure/fenwick_tree_2d', 'data_structure/range_tree_2d', 'algorithm/parallel_binary_search',
           'algorithm/mo_with_updates', 'data_structure/range_add_fenwick_tree','algorithm/slope_trick', 'algorithm/monotone_minima', 'algorithm/divide_and_conquer_dp',
           'math/gauss_elimination_gf2', 'algorithm/knapsack','graph/minimum_spanning_tree', 'graph/cycle_detection', 'graph/euler_trail',
           'graph/warshall_floyd', 'graph/namori', 'algorithm/cartesian_tree', 'algorithm/sliding_window_extrema']
WRAPPERS = {
    'bit2d': ('int height=3,width=4,y=1,x=2;long long value=5;', 'if(sum!=5)return 13;'),
    'range2d': ('std::vector<std::pair<long long,long long>>points{{0,1},{0,1},{3,-1}};long long x_lower=-1,x_upper=2,y_lower=0,y_upper=2;', 'if(count!=2)return 14;'),
    'parallel-bs': ('int updates=4,query_count=3,state=0;std::vector<int>need{0,3,5};auto reset=[&]{state=0;};auto apply=[&](int){++state;};auto check=[&](int q){return state>=need[q];};', 'if(boundaries!=need)return 15;'),
    'mo-updates': ('std::vector<long long>a{1,2,3};int index=1,l=0,r=3;long long value=5;', 'if(answers!=std::vector<long long>{9})return 16;'),
    'range-add-bit': ('int n=4,l=1,r=4;long long value=3;', 'if(sum!=9)return 17;'),
    'slope': ('long long a=7;', 'if(minimum!=0||function.eval(5)!=2)return 8;'),
    'monotone-min': ('int rows=3,columns=5;auto cost=[](int i,int j){return (i-j)*(i-j);};', 'if(minima.argmin!=std::vector<int>{0,1,2})return 9;'),
    'dc-dp': ('std::vector<std::optional<long long>>previous(5);previous[0]=0;auto cost=[](int k,int j){return 1LL*(j-k)*(j-k);};', 'if(next.value[4]!=16)return 10;'),
    'gf2-solve': ('int variables=3;std::vector<std::pair<std::vector<int>,bool>>equations{{{0,1},true},{{1,2},false}};', 'if(!solution.solvable||solution.rank!=2)return 11;'),
    'knapsack': ('std::vector<KnapsackItem>items{{2,5},{3,8}};int capacity=3;', 'if(best!=8)return 12;'),
    'mst': ('int n=3;std::vector<MSTEdge>edges{{0,1,-1},{1,2,5}};', 'if(forest.total_weight!=4)return 1;'),
    'cycle': ('int n=2;std::vector<std::pair<int,int>>edges{{0,1},{1,0}};', 'if(!cycle||cycle->vertices.size()!=2)return 2;'),
    'euler-trail': ('int n=2;std::vector<std::pair<int,int>>edges{{0,1}};', 'if(!trail||trail->edge_ids.size()!=1)return 3;'),
    'floyd': ('int n=3;std::vector<std::tuple<int,int,long long>>weighted_edges{{0,1,-1},{1,2,4}};', 'if(graph.distance(0,2).value!=3)return 4;'),
    'namori': ('int n=2;std::vector<std::pair<int,int>>edges{{0,1},{1,0}};', 'if(!namori||namori->cycle_vertices.size()!=2)return 5;'),
    'cartesian': ('std::vector<int>a{2,1,1};', 'if(tree.root!=1)return 6;'),
    'slide-min': ('std::vector<int>a{2,1,1};int width=2;', 'if(extrema.indices!=std::vector<int>{1,1})return 7;'),
}


class GapsIntegrationTest(unittest.TestCase):
    def compile_run(self, source, directory, *flags):
        exe = Path(directory) / 'example.exe'
        result = subprocess.run(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',*flags,
                                 '-x','c++','-','-o',str(exe)], input=source, capture_output=True, text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        result = subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
        self.assertEqual(result.returncode,0,result.stderr)

    def test_headers_and_multiple_translation_units(self):
        includes = ''.join(f'#include "{ROOT}/library/{h}.hpp"\n' for h in HEADERS)
        for header in HEADERS:
            with self.subTest(header=header):
                result = subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-DNDEBUG',
                                         '-fsyntax-only','-x','c++','-'],
                                        input=f'#include "{ROOT}/library/{header}.hpp"\n',capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'first.cpp').write_text(includes+'int other();int main(){return other();}\n')
            (root/'second.cpp').write_text(includes+'int other(){return 0;}\n')
            result = subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',str(root/'first.cpp'),
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
                        self.compile_run(source,directory,'-std=gnu++23','-I',str(ROOT))

    def test_slope_arc070_adapter(self):
        # AtCoder が取得不能でも提出用adapterを独立な格子DPと比較する。
        import random
        rng = random.Random(8144)
        with tempfile.TemporaryDirectory() as directory:
            exe = Path(directory) / 'slope.exe'
            result = subprocess.run(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',
                                     str(ROOT/'verify/algorithm/slope_trick_atcoder.test.cpp'),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            for _ in range(40):
                intervals = [(rng.randrange(11),rng.randrange(6)) for _ in range(rng.randrange(1,9))]
                domain = range(-12,24)
                previous = [abs(x-intervals[0][0]) for x in domain]
                for i in range(1,len(intervals)):
                    left,width = intervals[i]
                    old_width = intervals[i-1][1]
                    previous = [abs(x-left)+min(previous[j] for j,y in enumerate(domain)
                                                if x<=y+old_width and y<=x+width) for x in domain]
                source = str(len(intervals))+'\n'+''.join(f'{l} {l+w}\n' for l,w in intervals)
                result = subprocess.run([str(exe)],input=source,capture_output=True,text=True,timeout=10)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertEqual(int(result.stdout),min(previous))

    def test_one_based_aoj_adapter(self):
        with tempfile.TemporaryDirectory() as directory:
            exe = Path(directory)/'range-add.exe'
            result = subprocess.run(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',
                                     str(ROOT/'verify/data_structure/range_add_fenwick_tree_aoj.test.cpp'),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            result = subprocess.run([str(exe)],input='3 5\n0 1 3 5\n1 1 3\n0 2 2 -1\n1 2 2\n1 3 3\n',capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(result.stdout,'15\n4\n5\n')

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
            self.compile_run(source,directory,'-std=gnu++23','-I',str(ROOT))
            result = subprocess.run([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout'],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertFalse(any(line.startswith('#include "library/') for line in result.stdout.splitlines()))
            self.compile_run(result.stdout,directory,'-DNDEBUG')


if __name__ == '__main__':
    unittest.main()
