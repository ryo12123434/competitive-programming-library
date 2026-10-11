import json
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
SNIPPETS = REPOSITORY / "snippets/competitive-programming.code-snippets"


def expand(body, replacements=None):
    """今回のスニペットで使う既定値・連動タブストップを展開する。"""
    values = dict(replacements or {})

    def placeholder(match):
        number = int(match[1])
        default = match[2]
        if default is not None:
            values.setdefault(number, default)
        if number == 0:
            return ""
        if number not in values:
            raise ValueError(f"Undefined tab stop: {number}")
        return values[number]

    source = "\n".join(body)
    result = re.sub(r"\$\{(\d+)(?::([^}]*))?\}|\$(0)\b", lambda m: "" if m[3] else placeholder(m), source)
    if "$" in result:
        raise ValueError("Unsupported or unexpanded placeholder")
    return result


class ExampleTest(unittest.TestCase):
    def compile_and_run(self, source):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "example"
            result = subprocess.run(
                ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", "-I", str(REPOSITORY),
                 "-x", "c++", "-", "-o", str(executable)],
                input=source, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_doxygen_examples(self):
        for header in sorted((REPOSITORY / "library").rglob("*.hpp")):
            blocks = re.findall(r"@code\s*\n(.*?)@endcode", header.read_text(encoding="utf-8"), re.S)
            self.assertTrue(blocks, f"No usage example in {header}")
            for number, block in enumerate(blocks):
                source = re.sub(r"^\s*\* ?", "", block, flags=re.M).rstrip()
                with self.subTest(header=header.name, example=number):
                    self.compile_and_run(source)

    def test_snippets(self):
        snippets = json.loads(SNIPPETS.read_text(encoding="utf-8"))
        prefixes = [snippet["prefix"] for snippet in snippets.values()]
        self.assertEqual(len(prefixes), len(set(prefixes)))
        by_prefix = {}
        for snippet in snippets.values():
            self.assertEqual(snippet["scope"], "cpp")
            self.assertTrue(snippet["description"])
            self.assertIsInstance(snippet["body"], list)
            self.assertTrue(all(isinstance(line, str) for line in snippet["body"]))
            by_prefix[snippet["prefix"]] = snippet

        def code(prefix, replacements=None):
            return expand(by_prefix[prefix]["body"], replacements)

        # 設定と生成を組み合わせ、合成順序が非可換なアフィン写像も検査する。
        self.compile_and_run(code("cp-treap-add-config") + "\n#include <cassert>\nint main() {\n" +
                             code("cp-treap-init") + "\n"
                             "tree.apply(0, 2, 4); tree.reverse(0, 3); assert(tree.all_prod() == 14);\n}")
        self.compile_and_run(code("cp-treap-affine-config") + "\n#include <cassert>\nint main() {\n" +
                             code("cp-treap-init", {2: "treap_affine::S"}) + "\n"
                             "tree.apply(0, 3, {2, 1}); tree.apply(0, 3, {3, 4});\n"
                             "assert(tree.get(0).val() == 13); assert(tree.all_prod().val() == 57);\n}")
        self.compile_and_run(code("cp-sparse-min-config") + "\n#include <cassert>\nint main() {\n"
                             "std::vector<int> a{3, 1, 2};\n" + code("cp-sparse-init", {6: "3"}) + "\n"
                             "assert(minimum == 1 && table.size() == 3);\n}")
        self.compile_and_run(code("cp-sparse-min-config", {1: "sparse_max", 2: "long long", 3: "StaticMax",
                                                           4: "a < b ? b : a"}) +
                             "\n#include <cassert>\nint main() { std::vector<long long> values{3,1,2};\n" +
                             code("cp-sparse-init", {1: "StaticMax", 2: "rmq", 3: "values", 4: "maximum", 6: "3"}) +
                             "\nassert(maximum == 3);\n}")
        mo_prelude = ('#include "library/algorithm/mo.hpp"\n#include <cassert>\n#include <utility>\n'
                      'int main() { std::vector<long long> a{1,2,3};\n'
                      'std::vector<std::pair<int,int>> queries{{0,3},{1,1},{1,3},{0,3}};\n')
        self.compile_and_run(mo_prelude + code("cp-mo-sum") + "\n"
                             "assert((answers == std::vector<long long>{6,0,5,6}));\n}")
        # 枠だけでも構文が正しく、編集した左右別コールバックで元の順序を復元できる。
        self.compile_and_run(mo_prelude + code("cp-mo-directional") + "\n}")
        self.compile_and_run(mo_prelude + "std::vector<long long> state;\n"
                             "std::vector<std::vector<long long>> answers(queries.size());\n" +
                             code("cp-mo-directional", {
                                 4: "state.insert(state.begin(), a[index]);",
                                 5: "state.push_back(a[index]);",
                                 6: "(void)index; state.erase(state.begin());",
                                 7: "(void)index; state.pop_back();",
                                 8: "answers[query_id] = state;",
                             }) + "\n"
                             "for (int i=0; i<4; ++i) { auto [l,r] = queries[i];\n"
                             "assert((answers[i] == std::vector<long long>(a.begin()+l,a.begin()+r))); }\n}")
        tested = {"cp-treap-add-config", "cp-treap-affine-config", "cp-treap-init", "cp-sparse-min-config",
                  "cp-sparse-init", "cp-mo-sum", "cp-mo-directional"}
        self.assertEqual(set(by_prefix), tested, "Add an API check for each new snippet")


if __name__ == "__main__":
    unittest.main()
