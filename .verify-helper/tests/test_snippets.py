import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
SNIPPET_DIRECTORY = REPOSITORY / "snippets"


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def load_snippets():
    by_prefix = {}
    for path in sorted(SNIPPET_DIRECTORY.glob("*.code-snippets")):
        entries = json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)
        for snippet in entries.values():
            prefixes = snippet["prefix"]
            if isinstance(prefixes, str):
                prefixes = [prefixes]
            if not isinstance(prefixes, list) or not prefixes:
                raise ValueError(f"Invalid prefix in {path}")
            for prefix in prefixes:
                if prefix in by_prefix:
                    raise ValueError(f"Duplicate snippet prefix: {prefix}")
                by_prefix[prefix] = snippet
    return by_prefix


def expand(body, replacements=None):
    """使用中の数値タブストップ・既定値・連動参照と VSCode のエスケープを展開する。"""
    values = {0: "", **(replacements or {})}

    def token(match):
        if match[1] is not None:
            return match[1]
        number = int(match[2] or match[4])
        if match[3] is not None:
            values.setdefault(number, match[3])
        if number not in values:
            raise ValueError(f"Undefined tab stop: {number}")
        return values[number]

    return re.sub(r"\\([\\$}])|\$\{(\d+)(?::([^}]*))?\}|\$(\d+)\b", token, "\n".join(body))


class SnippetTest(unittest.TestCase):
    def setUp(self):
        self.snippets = load_snippets()

    def code(self, prefix, replacements=None):
        return expand(self.snippets[prefix]["body"], replacements)

    def compile_and_run(self, source, *flags, include_repository=True):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "snippet.exe"
            result = subprocess.run(
                ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror",
                 *(["-I", str(REPOSITORY)] if include_repository else []),
                 *flags, "-x", "c++", "-", "-o", str(executable)],
                input=source, text=True, capture_output=True, cwd=directory,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            return result

    def test_catalog(self):
        self.assertEqual({path.name for path in SNIPPET_DIRECTORY.glob("*.code-snippets")}, {
            "template.code-snippets", "lazy-segtree.code-snippets", "implicit-treap.code-snippets",
            "mo.code-snippets", "sparse-table.code-snippets",
        })
        # 全スニペットを下のコンパイル・動作確認へ対応付ける。追加時は検証も追加する。
        self.assertEqual(set(self.snippets), {
            "main", "atcmain", "treap-add", "treap-affine", "treap-init", "treap-help",
            "mo-sum", "mo-directional", "mo-help", "sparse-min", "sparse-init", "sparse-help",
            "lazy-assign-min", "lazy-assign-max", "lazy-assign-sum",
            "lazy-add-min", "lazy-add-max", "lazy-add-sum", "lazy-init", "lazy-help",
            "lambda", "lambda-rec", "grid-check",
        })
        for prefix, snippet in self.snippets.items():
            with self.subTest(prefix=prefix):
                self.assertRegex(prefix, r"^[a-z][a-z0-9]*(?:-[a-z0-9]+)*$")
                self.assertFalse(prefix.startswith("cp-"))
                self.assertFalse(prefix.endswith("-config"))
                self.assertEqual(snippet["scope"], "cpp")
                self.assertRegex(snippet["description"], r"[ぁ-んァ-ヶ一-龠]")
                self.assertIsInstance(snippet["body"], list)
                self.assertTrue(all(isinstance(line, str) for line in snippet["body"]))
                self.assertNotRegex(self.code(prefix), r"\$\{\d|\$\d+\b")

    def test_main_alias(self):
        self.assertIs(self.snippets["main"], self.snippets["atcmain"])
        self.assertEqual(self.snippets["main"]["prefix"], ["main", "atcmain"])
        self.assertEqual(self.code("main"), self.code("atcmain"))
        body = r'''
    static_assert(el == '\n');
    assert(ctoi('9') == 9 && ctoi('x') == -1 && digit_sum("123") == 6);
    int value = 2;
    assert(chmax(value, 4) && value == 4 && !chmax(value, 3));
    assert(chmin(value, 1) && value == 1 && !chmin(value, 2));
    vi a{3, 1, 2};
    sort(all(a));
    assert((a == vi{1, 2, 3}));
    int sum = 0;
    rep(i, 0, 3) sum += a[i];
    assert(sum == 6);
    int evaluated = 0;
    std::vector<std::vector<int>> dp{{1,2}, {3,4}};
    debug(++evaluated);
    debug_table((++evaluated, dp));
    { CPL_SCOPED_TIMER((++evaluated, "solve")); }
#ifdef LOCAL
    assert(evaluated == 3);
    cpl::debug::Timer timer;
    timer.reset();
    assert(timer.elapsed_ms() >= 0);
    cpl::random::Random rng(42), same(42);
    assert(rng.integer(-10,10) == same.integer(-10,10));
#else
    assert(evaluated == 0);
    debug(undefined_value());
    debug_table(undefined_table());
    CPL_SCOPED_TIMER(undefined_label());
#endif
'''
        # 既存テンプレートの折り畳み用 pragma は GCC が認識しないため、この警告だけ除外する。
        for flags in ([], ["-DLOCAL"]):
            with self.subTest(flags=flags):
                result = self.compile_and_run(self.code("main", {0: body}), "-Wno-unknown-pragmas", *flags)
                self.check_debug_output(result, bool(flags))

        # 通常の自作ライブラリは展開し、LOCAL 内の debug include だけが残ることを実証する。
        source = (self.code("sparse-min") + "\n" +
                  self.code("main", {0: body + '\nStaticRMQ table(std::vector<int>{3,1,2}); assert(table.prod(0,3) == 1);\n'}))
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.cpp"
            path.write_text(source, encoding="utf-8")
            result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(path), "--stdout"],
                                    text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
        bundled = result.stdout
        for name in ("debug", "timer", "random"):
            self.assertIn(f'#include "library/debug/{name}.hpp"', bundled)
        self.assertNotIn('library/debug/table_debug.hpp', bundled)
        self.assertNotIn('// BEGIN: library/debug/', bundled)
        self.assertIn('// BEGIN: library/data_structure/sparse_table.hpp', bundled)
        for local in (False, True):
            with self.subTest(bundled=True, local=local):
                result = self.compile_and_run(bundled, "-Wno-unknown-pragmas", *(["-DLOCAL"] if local else []),
                                              include_repository=local)
                self.check_debug_output(result, local)

    def check_debug_output(self, result, local):
        self.assertEqual(result.stdout, "")
        if local:
            self.assertIn("++evaluated", result.stderr)
            self.assertIn("r/c", result.stderr)
            self.assertIn("solve:", result.stderr)
            self.assertIn(" ms", result.stderr)
        else:
            self.assertEqual(result.stderr, "")

    def test_lambda(self):
        source = '#include <cassert>\nint main() {\n' + self.code("lambda") + '\nassert(func(7) == 7);\n'
        source += 'int offset = 8;\n' + self.code("lambda", {
            1: "add", 2: "offset", 3: "int value", 4: "long long",
            5: "return static_cast<long long>(value) + offset;",
        }) + '\nassert(add(5) == 13);\n'
        source += self.code("lambda", {
            1: "append", 2: "", 3: "int value, int& total", 4: "void", 5: "total += value;",
        }) + '\nint total = 0; append(4, total); assert(total == 4);\n'
        source += self.code("lambda", {
            1: "constant", 2: "", 3: "", 4: "int", 5: "return 42;",
        }) + '\nassert(constant() == 42);\n}\n'
        self.compile_and_run(source, include_repository=False)

    def test_recursive_lambda(self):
        source = '#include <cassert>\nint main() {\n' + self.code("lambda-rec") + '\n'
        source += 'assert(dfs(dfs, 0) == 1 && dfs(dfs, 5) == 120 && dfs(dfs, 20) == 2432902008176640000LL);\n'
        source += self.code("lambda-rec", {
            1: "factorial", 2: "", 3: ", int count", 4: "unsigned long long",
            5: "if (count <= 1) return 1ULL;\n    return count * self(self, count - 1);",
        }) + '\nassert(factorial(factorial, 6) == 720ULL);\n'
        source += 'int total = 0;\n' + self.code("lambda-rec", {
            1: "visit", 3: ", int value", 4: "void",
            5: "if (value < 0) return;\n    total += value;\n    self(self, value - 1);",
        }) + '\nvisit(visit, 4); assert(total == 10);\n'
        source += 'int remaining = 3;\n' + self.code("lambda-rec", {
            1: "countdown", 3: "", 4: "int",
            5: "if (remaining == 0) return 0;\n    --remaining;\n    return 1 + self(self);",
        }) + '\nassert(countdown(countdown) == 3 && remaining == 0);\n}\n'
        self.compile_and_run(source, include_repository=False)

    def test_grid_check(self):
        # 空・1 マス・境界内外を列挙し、continue が内側のループだけを進めることも確認する。
        default = self.code("grid-check")
        renamed = self.code("grid-check", {1: "height", 2: "width", 3: "ny", 4: "nx"})
        source = '#include <cassert>\nint main() {\n'
        for h, w, y, x, code in [("h", "w", "y", "x", default), ("height", "width", "ny", "nx", renamed)]:
            source += f'''
    for (int {h} = 0; {h} <= 3; ++{h}) {{
        for (int {w} = 0; {w} <= 3; ++{w}) {{
            int count = 0;
            for (int {y} = -1; {y} <= {h}; ++{y}) {{
                for (int {x} = -1; {x} <= {w}; ++{x}) {{
                    {code}
                    assert(0 <= {y} && {y} < {h} && 0 <= {x} && {x} < {w});
                    ++count;
                }}
            }}
            assert(count == {h} * {w});
        }}
    }}
'''
        self.compile_and_run(source + '}\n', include_repository=False)

    def test_treap(self):
        self.compile_and_run(self.code("treap-add") + "\n#include <cassert>\nint main() {\n" +
                             self.code("treap-help") + self.code("treap-init") + "\n"
                             "tree.apply(0, 2, 4); tree.reverse(0, 3); assert(tree.all_prod() == 14);\n"
                             "tree.insert(3, 8); tree.erase(0); tree.set(0, 9); tree.rotate(0, 1, 3);\n"
                             "assert((tree.to_vector() == std::vector<long long>{5,8,9}));\n"
                             "assert(tree.get(0) == 5 && tree[1] == 8 && tree.prod(0,0) == 0);\n"
                             "assert(tree.size() == 3 && !tree.empty()); tree.clear(); assert(tree.empty());\n}")
        self.compile_and_run(self.code("treap-affine") + "\n#include <cassert>\nint main() {\n" +
                             self.code("treap-init", {2: "treap_affine::S"}) + "\n"
                             "tree.apply(0, 3, {2, 1}); tree.apply(0, 3, {3, 4});\n"
                             "assert(tree.get(0).val() == 13); assert(tree.all_prod().val() == 57);\n}")

    def test_sparse_table(self):
        self.compile_and_run(self.code("sparse-min") + "\n#include <cassert>\nint main() {\n" +
                             self.code("sparse-help") +
                             "\nstd::vector<int> a{3, 1, 2};\n" + self.code("sparse-init", {6: "3"}) + "\n"
                             "assert(minimum == 1 && table.size() == 3);\n}")
        self.compile_and_run(self.code("sparse-min", {1: "sparse_max", 2: "long long", 3: "StaticMax",
                                                      4: "a < b ? b : a"}) +
                             "\n#include <cassert>\nint main() { std::vector<long long> values{3,1,2};\n" +
                             self.code("sparse-init", {1: "StaticMax", 2: "rmq", 3: "values", 4: "maximum", 6: "3"}) +
                             "\nassert(maximum == 3);\n}")

    def test_mo(self):
        prelude = ('#include "library/algorithm/mo.hpp"\n#include <cassert>\n#include <utility>\n'
                   'int main() { std::vector<long long> a{1,2,3};\n'
                   'std::vector<std::pair<int,int>> queries{{0,3},{1,1},{1,3},{0,3}};\n')
        self.compile_and_run(prelude + self.code("mo-help") + self.code("mo-sum") + "\n"
                             "assert((answers == std::vector<long long>{6,0,5,6}));\n}")
        self.compile_and_run(prelude + self.code("mo-directional") + "\n}")
        # 編集した左右別コールバックで元の要素順を復元できる。
        self.compile_and_run(prelude + "std::vector<long long> state;\n"
                             "std::vector<std::vector<long long>> answers(queries.size());\n" +
                             self.code("mo-directional", {
                                 4: "state.insert(state.begin(), a[index]);",
                                 5: "state.push_back(a[index]);",
                                 6: "(void)index; state.erase(state.begin());",
                                 7: "(void)index; state.pop_back();",
                                 8: "answers[query_id] = state;",
                             }) + "\n"
                             "for (int i=0; i<4; ++i) { auto [l,r] = queries[i];\n"
                             "assert((answers[i] == std::vector<long long>(a.begin()+l,a.begin()+r))); }\n}")

    def test_lazy_segtree(self):
        headers, blocks = [], []
        for update in ("assign", "add"):
            for aggregate, field in (("min", "mn"), ("max", "mx"), ("sum", "sum")):
                namespace = f"lazy_{update}_{aggregate}"
                typename = "Lazy" + update.title() + aggregate.title()
                headers.append(self.code(f"lazy-{update}-{aggregate}"))
                initial = self.code("lazy-init", {1: namespace, 5: typename})
                identity = ("0LL" if aggregate == "sum" else
                            "std::numeric_limits<long long>::max()" if aggregate == "min" else
                            "std::numeric_limits<long long>::lowest()")
                combine = {"min": "std::min(answer, naive[i])", "max": "std::max(answer, naive[i])",
                           "sum": "answer + naive[i]"}[aggregate]
                change = "naive[i] = value;" if update == "assign" else "naive[i] += value;"
                blocks.append("{\n" + self.code("lazy-help") + "\nconst int n = 7;\n" + initial + f'''
    std::vector<long long> naive(n, 0);
    std::mt19937 rng(20261018);
    for (int step = 0; step < 100; ++step) {{
        int l = rng() % (n + 1), r = rng() % (n + 1);
        if (l > r) std::swap(l, r);
        const long long value = static_cast<int>(rng() % 21) - 10;
        if (step % 4 == 0) {{
            const int p = rng() % n;
            seg.set(p, {{value, 1}}); naive[p] = value;
        }} else {{
            seg.apply(l, r, value);
            for (int i = l; i < r; ++i) {{ {change} }}
        }}
        for (int begin = 0; begin <= n; ++begin) {{
            long long answer = {identity};
            for (int end = begin; end <= n; ++end) {{
                const auto result = seg.prod(begin, end);
                assert(result.len == end - begin && result.{field} == answer);
                if (end < n) {{ const int i = end; answer = {combine}; }}
            }}
        }}
        for (int p = 0; p < n; ++p) {{ assert(seg.get(p).len == 1 && seg.get(p).{field} == naive[p]); }}
        assert(seg.all_prod().{field} == seg.prod(0,n).{field});
    }}
    {typename} empty(0);
    empty.apply(0,0,0);
    assert(empty.prod(0,0).len == 0 && empty.all_prod().{field} == {identity});
}}
''')
        self.compile_and_run("\n".join(headers) + "\n#include <cassert>\n#include <random>\n#include <limits>\n"
                             "int main() {\n" + "\n".join(blocks) + "\n}")

    def test_quick_references(self):
        for prefix in ("lazy-help", "treap-help", "mo-help", "sparse-help"):
            with self.subTest(prefix=prefix):
                self.assertTrue(all(not line.strip() or line.startswith("//") for line in self.code(prefix).splitlines()))


if __name__ == "__main__":
    unittest.main()
