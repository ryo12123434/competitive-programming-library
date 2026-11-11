from pathlib import Path
import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from test_snippets import expand, load_snippets

ROOT = Path(__file__).resolve().parents[2]
PHASES = json.loads((ROOT / '.verify-helper/tests/round5_manifest.json').read_text(encoding='utf-8'))

class Round5Test(unittest.TestCase):
    def test_e_all_headers_release(self):
        # assert除去時の警告・名前の解決も、実際の提出設定で確認する。
        for standard in ['c++20', 'gnu++23']:
            for header in sorted((ROOT / 'library').rglob('*.hpp')):
                with self.subTest(header=str(header.relative_to(ROOT)), standard=standard):
                    self.command(['g++', '-std='+standard, '-DNDEBUG', '-Wall', '-Wextra', '-Werror',
                                  '-fsyntax-only', '-x', 'c++', '-'], input=f'#include "{header}"\n')

    def test_c_acl_string_dependency_boundary(self):
        if 'C' not in PHASES: self.skipTest('Phase C is not implemented')
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            (root/'a.cpp').write_text('#include <atcoder/string>\nint other();int main(){return other();}\n')
            (root/'b.cpp').write_text('#include <atcoder/string>\nint other(){return 0;}\n')
            result=subprocess.run(['g++','-std=c++20',str(root/'a.cpp'),str(root/'b.cpp'),'-o',str(root/'acl')],text=True,capture_output=True)
            self.assertNotEqual(result.returncode,0,'ACL linking is fixed; reconsider the precomputed-array API rationale')
            symbols=set(re.findall(r"multiple definition of [`']([^']+)'",result.stderr))
            self.assertEqual(len(symbols),6,result.stderr)
            for prefix in ['atcoder::internal::sa_naive(', 'atcoder::internal::sa_doubling(',
                           'atcoder::suffix_array(', 'atcoder::lcp_array(', 'atcoder::z_algorithm(']:
                self.assertTrue(any(s.startswith(prefix) for s in symbols),result.stderr)

    def test_c_dynamic_hash_adapter(self):
        if 'C' not in PHASES: self.skipTest('Phase C is not implemented')
        import random
        rng=random.Random(331)
        with tempfile.TemporaryDirectory() as directory:
            exe=Path(directory)/'adapter.exe'
            self.command(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',str(ROOT/'verify/string/round5_dynamic_rolling_hash_atcoder.test.cpp'),'-o',str(exe)])
            for n in range(1,40):
                values=[rng.choice('abc') for _ in range(n)];initial=''.join(values);ops=[];expected=[]
                for _ in range(100):
                    if rng.randrange(2):
                        i=rng.randrange(n);c=rng.choice('abc');values[i]=c;ops.append(f'1 {i+1} {c}')
                    else:
                        l=rng.randrange(n);r=rng.randrange(l+1,n+1);part=values[l:r];expected.append('Yes' if part==part[::-1] else 'No');ops.append(f'2 {l+1} {r}')
                output=self.command([str(exe)],input=f'{n} {len(ops)}\n{initial}\n'+'\n'.join(ops)+'\n')
                self.assertEqual(output.stdout.splitlines(),expected)

    def test_b_one_indexed_aoj_adapter(self):
        if 'B' not in PHASES: self.skipTest('Phase B is not implemented')
        import random
        rng=random.Random(512)
        with tempfile.TemporaryDirectory() as directory:
            exe=Path(directory)/'adapter.exe'
            self.command(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',str(ROOT/'verify/data_structure/round5_dual_segment_tree_aoj.test.cpp'),'-o',str(exe)])
            for n in range(1,20):
                values=[0]*n;ops=[];expected=[]
                for _ in range(100):
                    if rng.randrange(2):
                        l=rng.randrange(n);r=rng.randrange(l,n);x=rng.randrange(-10,11)
                        ops.append(f'0 {l+1} {r+1} {x}')
                        for i in range(l,r+1):values[i]+=x
                    else:
                        i=rng.randrange(n);ops.append(f'1 {i+1}');expected.append(str(values[i]))
                result=self.command([str(exe)],input=f'{n} {len(ops)}\n'+'\n'.join(ops)+'\n')
                self.assertEqual(result.stdout.splitlines(),expected)

    def command(self, args, **kwargs):
        result = subprocess.run(args, text=True, capture_output=True, **kwargs)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result

    def compile_run(self, source, root, standard='c++20', flags=(), include=True, environment=None):
        exe = root / 'test.exe'
        self.command(['g++', '-std='+standard, '-O2', '-Wall', '-Wextra', '-Werror', *flags,
                      *(['-I',str(ROOT)] if include else []), '-x', 'c++', '-', '-o',str(exe)], input=source, env=environment)
        return self.command([str(exe)], timeout=180)

    def regression(self, phase):
        source = (ROOT / f'.verify-helper/tests/fixtures/round5_{phase.lower()}.cpp').read_text(encoding='utf-8')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            sanitizer = ['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'] if sys.platform != 'win32' else []
            if sys.platform.startswith('linux'): sanitizer.append('-no-pie')
            self.compile_run(source, root, flags=sanitizer)
            self.compile_run(source, root, standard='gnu++23')

    def integration(self, phase):
        info = PHASES[phase]
        includes = ''.join(f'#include "library/{h}.hpp"\n' for h in info['headers'])
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for standard in ['c++20','gnu++23']:
                for h in info['headers']:
                    with self.subTest(header=h,standard=standard):
                        self.command(['g++','-std='+standard,'-Wall','-Wextra','-Werror','-DNDEBUG','-I',str(ROOT),
                                      '-fsyntax-only','-x','c++','-'], input=f'#include "library/{h}.hpp"\n')
            (root/'a.cpp').write_text(includes+'int other();int main(){return other();}\n')
            (root/'b.cpp').write_text(includes+'int other(){return 0;}\n')
            self.command(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror','-I',str(ROOT),
                          str(root/'a.cpp'),str(root/'b.cpp'),'-o',str(root/'multi')])
            self.command([str(root/'multi')])
            for h in info['headers']:
                blocks = re.findall(r'@code\s*\n(.*?)@endcode',(ROOT/f'library/{h}.hpp').read_text(encoding='utf-8'),re.S)
                self.assertTrue(blocks,h)
                for block in blocks:
                    with self.subTest(example=h):
                        self.compile_run(re.sub(r'^\s*\* ?', '', block, flags=re.M), root)
            snippets = load_snippets()
            source = includes+''.join(f'#include "library/{h}.hpp"\n' for h in info.get("extra_includes", []))+'\n#include <atcoder/modint>\n#include <cassert>\nusing M=atcoder::modint998244353;\n'
            # 複雑な設定のスニペットはグローバルに挿入する。
            for prefix in info.get('global_snippets',[]):source += expand(snippets[prefix]['body'])+'\n'
            source += ''.join(f'#include <{h}>\n' for h in info.get('extra_system_includes', []))
            source += info.get('global_code', '') + '\nint main(){\n'
            for prefix,(before,after) in info['snippets'].items():
                source += '{\n'+before+'\n'+expand(snippets[prefix]['body'])+'\n'+after+'\n}\n'
            source += '}\n'
            self.compile_run(source,root)
            # ライブラリのassertを無効化した定義でも、使用側はassertを保って実際に全APIをインスタンス化する。
            release_source=source.replace('int main(){', '#undef NDEBUG\n#include <cassert>\nint main(){',1)
            self.compile_run(release_source,root,standard='gnu++23',flags=['-DNDEBUG'])
            path=root/'snippet.cpp';path.write_text(source)
            bundled=self.command([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout']).stdout
            self.compile_run(bundled,root,standard='gnu++23',include=False)
            # ACLも展開すれば、提出側にはACL/自作ヘッダのincludeパスが不要。
            expanded=self.command([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout','--expand-acl']).stdout
            self.assertFalse(re.search(r'(?m)^\s*#\s*include\s*[<"]atcoder/', expanded),
                             'ACL include directive remains after --expand-acl')
            environment=dict(os.environ, CPLUS_INCLUDE_PATH='', CPATH='')
            self.compile_run(expanded,root,standard='gnu++23',include=False,environment=environment)
            # assertを使用しない提出側でもテンプレート本体が警告なく生成されることを確認する。
            self.compile_run(includes+'int main(){return 0;}\n',root,flags=['-DNDEBUG'])


def add_phase(phase):
    def regression(self):self.regression(phase)
    def integration(self):self.integration(phase)
    setattr(Round5Test,'test_'+phase.lower()+'_regression',regression)
    setattr(Round5Test,'test_'+phase.lower()+'_integration',integration)
for phase in PHASES:add_phase(phase)

if __name__ == '__main__':unittest.main()
