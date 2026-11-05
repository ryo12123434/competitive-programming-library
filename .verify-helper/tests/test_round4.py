from pathlib import Path
import json
import re
import subprocess
import sys
import tempfile
import unittest
from test_snippets import expand, load_snippets

ROOT = Path(__file__).resolve().parents[2]
PHASES = json.loads((ROOT / '.verify-helper/tests/round4_manifest.json').read_text())

class Round4Test(unittest.TestCase):
    def test_c_official_adapters_locally(self):
        if 'C' not in PHASES: self.skipTest('Phase C is not implemented yet')
        import random
        rng = random.Random(934)
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            for path,kind in [('verify/math/round4_linear_recurrence_atcoder.test.cpp','recurrence'),
                              ('verify/algorithm/round4_knuth_atcoder.test.cpp','knuth')]:
                exe=root/'adapter'
                self.command(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',str(ROOT/path),'-o',str(exe)])
                for _ in range(60):
                    if kind=='recurrence':
                        a,n,m=rng.randrange(50),rng.randrange(100),rng.randrange(1,100)
                        expected=sum(pow(a,k,m) for k in range(n))%m
                        data=f'{a} {n} {m}\n'
                    else:
                        n=rng.randrange(1,15);values=[rng.randrange(11) for _ in range(n)]
                        prefix=[0]
                        for x in values:prefix.append(prefix[-1]+x)
                        dp=[[0]*(n+1) for _ in range(n+1)]
                        for length in range(2,n+1):
                            for l in range(n-length+1):
                                r=l+length
                                dp[l][r]=min(dp[l][k]+dp[k][r] for k in range(l+1,r))+prefix[r]-prefix[l]
                        expected=dp[0][n];data=str(n)+'\n'+' '.join(map(str,values))+'\n'
                    self.assertEqual(self.command([str(exe)],input=data,timeout=10).stdout.strip(),str(expected))

    def command(self, args, **kwargs):
        result = subprocess.run(args, text=True, capture_output=True, **kwargs)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result

    def compile_run(self, source, root, standard='c++20', flags=(), include=True):
        exe = root / 'test.exe'
        self.command(['g++', '-std='+standard, '-O2', '-Wall', '-Wextra', '-Werror', *flags,
                      *(['-I',str(ROOT)] if include else []), '-x', 'c++', '-', '-o',str(exe)], input=source)
        return self.command([str(exe)], timeout=180)

    def regression(self, phase):
        source = (ROOT / f'.verify-helper/tests/fixtures/round4_{phase.lower()}.cpp').read_text()
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
                blocks = re.findall(r'@code\s*\n(.*?)@endcode',(ROOT/f'library/{h}.hpp').read_text(),re.S)
                self.assertTrue(blocks,h)
                for block in blocks:
                    with self.subTest(example=h):
                        self.compile_run(re.sub(r'^\s*\* ?', '', block, flags=re.M), root)
            snippets = load_snippets()
            source = includes+''.join(f'#include "library/{h}.hpp"\n' for h in info.get("extra_includes", []))+'\n#include <atcoder/modint>\n#include <cassert>\nusing M=atcoder::modint998244353;\n'
            # 複雑な設定のスニペットはグローバルに挿入する。
            for prefix in info.get('global_snippets',[]):source += expand(snippets[prefix]['body'])+'\n'
            source += info.get('global_code', '') + '\nint main(){\n'
            for prefix,(before,after) in info['snippets'].items():
                source += '{\n'+before+'\n'+expand(snippets[prefix]['body'])+'\n'+after+'\n}\n'
            source += '}\n'
            self.compile_run(source,root)
            path=root/'snippet.cpp';path.write_text(source)
            bundled=self.command([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout']).stdout
            self.compile_run(bundled,root,standard='gnu++23',include=False)
            # assertを使用しない提出側でもテンプレート本体が警告なく生成されることを確認する。
            self.compile_run(includes+'int main(){return 0;}\n',root,flags=['-DNDEBUG'])


def add_phase(phase):
    def regression(self):self.regression(phase)
    def integration(self):self.integration(phase)
    setattr(Round4Test,'test_'+phase.lower()+'_regression',regression)
    setattr(Round4Test,'test_'+phase.lower()+'_integration',integration)
for phase in PHASES:add_phase(phase)

if __name__ == '__main__':unittest.main()
