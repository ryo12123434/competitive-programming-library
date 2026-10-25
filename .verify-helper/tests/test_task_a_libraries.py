"""タスク A 専用。既存の共通テスト・設定を変更せずに検証する。"""
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
HEADERS = [
    "graph/functional_graph.hpp", "graph/euler_tour.hpp", "graph/tree_diameter.hpp",
    "data_structure/prefix_sum_2d.hpp", "math/matrix.hpp",
]
INCLUDES = "".join(f'#include "library/{name}"\n' for name in HEADERS)
FIXTURE = REPOSITORY / ".verify-helper/tests/fixtures/task_a_regression.cpp"
SNIPPETS = REPOSITORY / "snippets/task-a/abc.code-snippets"
SMOKE = INCLUDES + r'''
#include <cassert>
int main() {
    FunctionalGraph graph(std::vector<int>{1, 1});
    if (graph.reachable_count(0) != 2) return 1;
    std::vector<std::vector<int>> tree{{1}, {0}};
    EulerTour tour(tree);
    if (tour.tout(0) != 2) return 2;
    TreeDiameter diameter(tree);
    if (diameter.length() != 1) return 3;
    using D = TreeDiameter<int>;
    D weighted(std::vector<std::vector<D::Edge>>{{{1, 0}}, {{0, 0}}});
    if (weighted.length() != 0) return 4;
    PrefixSum2D<int> sum(std::vector<std::vector<int>>{{1, 2}});
    if (sum.query(0, 1, 0, 2) != 3) return 5;
    Matrix<int> a(1, 1, 2);
    if (a.pow(3)(0, 0) != 8) return 6;
}
'''


class TaskALibraryTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.root = Path(cls.directory.name)
        cls.binary = cls.root / "regression.exe"
        flags = ["-fsanitize=undefined", "-fno-sanitize-recover=all"] if os.name != "nt" else []
        command = ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", *flags,
                   "-I", str(REPOSITORY), str(FIXTURE), "-o", str(cls.binary)]
        result = subprocess.run(command, text=True, capture_output=True, timeout=120)
        if result.returncode:
            cls.directory.cleanup()
            raise RuntimeError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def run_program(self, *args, timeout=60):
        result = subprocess.run([str(self.binary), *args], text=True, capture_output=True, timeout=timeout)
        self.assertEqual(result.returncode, 0, result.stderr)

    def compile_run(self, source, *flags, include_repository=True):
        binary = self.root / "example.exe"
        result = subprocess.run(
            ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", *flags,
             *(["-I", str(REPOSITORY)] if include_repository else []),
             "-x", "c++", "-", "-o", str(binary)],
            input=source, text=True, capture_output=True, cwd=self.root, timeout=120,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(binary)], text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_functional_exhaustive_and_random(self):
        self.run_program("functional")

    def test_euler_and_diameter_exhaustive_and_random(self):
        self.run_program("tree")

    def test_prefix_sum_exhaustive_and_random(self):
        self.run_program("prefix")

    def test_matrix_exhaustive_and_random(self):
        self.run_program("matrix")

    def test_invalid_preconditions(self):
        # GNU/MinGW とも assert 有効時の異常終了を確認。例外としてのエラー処理は提供しない。
        for mode in ("matrix-product", "matrix-power", "matrix-ragged", "matrix-negative",
                     "prefix-ragged", "prefix-range", "functional-range", "diameter-negative",
                     "euler-empty", "euler-disconnected"):
            with self.subTest(mode=mode):
                result = subprocess.run([str(self.binary), mode], capture_output=True, timeout=10,
                                        cwd=self.root)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(b"Assertion", result.stderr)
                self.assertNotIn(b"runtime error:", result.stderr)

    def test_headers_debug_and_release(self):
        for name in HEADERS:
            for flags in ([], ["-DNDEBUG"]):
                with self.subTest(header=name, flags=flags):
                    result = subprocess.run(
                        ["g++", "-std=gnu++23", "-Wall", "-Wextra", "-Werror", *flags,
                         "-I", str(REPOSITORY), "-fsyntax-only", "-x", "c++", "-"],
                        input=f'#include "library/{name}"\n', text=True, capture_output=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
        self.compile_run(SMOKE)
        self.compile_run(SMOKE, "-DNDEBUG")

    def test_multiple_translation_units(self):
        first = self.root / "first.cpp"
        second = self.root / "second.cpp"
        first.write_text(SMOKE.replace("int main()", "int other()") + "\nint main() { return other(); }\n",
                         encoding="utf-8")
        second.write_text(SMOKE.replace("int main()", "int another()"), encoding="utf-8")
        # main 以外の成功経路にも return を明示する。
        for path in (first, second):
            path.write_text(path.read_text(encoding="utf-8").replace("return 6;\n}", "return 6;\nreturn 0;\n}"),
                            encoding="utf-8")
        result = subprocess.run(["g++", "-std=gnu++23", "-Wall", "-Wextra", "-Werror",
                                 "-I", str(REPOSITORY), str(first), str(second),
                                 "-o", str(self.root / "multiple.exe")], text=True, capture_output=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(self.root / "multiple.exe")], capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_new_doxygen_examples(self):
        for name in HEADERS:
            blocks = re.findall(r"@code\s*\n(.*?)@endcode", (REPOSITORY / "library" / name).read_text(encoding="utf-8"), re.S)
            self.assertTrue(blocks)
            for number, block in enumerate(blocks):
                with self.subTest(header=name, example=number):
                    self.compile_run(re.sub(r"^\s*\* ?", "", block, flags=re.M).rstrip())

    def test_bundle(self):
        source = self.root / "submission.cpp"
        source.write_text(SMOKE, encoding="utf-8")
        result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(source), "--stdout"],
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotRegex(result.stdout, r'(?m)^\s*#\s*include\s*"library/')
        self.compile_run(result.stdout, include_repository=False)

    def test_snippets(self):
        spec = importlib.util.spec_from_file_location("task_a_snippet_helpers", Path(__file__).with_name("test_snippets.py"))
        helpers = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helpers)
        entries = json.loads(SNIPPETS.read_text(encoding="utf-8"), object_pairs_hook=helpers.unique_object)
        snippets = {entry["prefix"]: entry for entry in entries.values()}
        self.assertEqual(len(snippets), len(entries))
        self.assertEqual(set(snippets), {"functional", "euler", "prefix2d", "diameter", "matrix", "matrix-pow"})
        seen = set()
        for path in (REPOSITORY / "snippets").rglob("*.code-snippets"):
            for entry in json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=helpers.unique_object).values():
                prefixes = entry["prefix"] if isinstance(entry["prefix"], list) else [entry["prefix"]]
                for prefix in prefixes:
                    self.assertNotIn(prefix, seen)
                    seen.add(prefix)
        for entry in snippets.values():
            self.assertEqual(entry["scope"], "cpp")
            self.assertRegex(entry["description"], r"[ぁ-んァ-ヶ一-龠]")
        def code(prefix, replacements=None):
            return helpers.expand(snippets[prefix]["body"], replacements)
        self.compile_run(INCLUDES + "\nint main() {\n"
                         "std::vector<int> next{1, 1};\n" + code("functional") +
                         "\nif (graph.reachable_count(0) != 2) return 1;\n"
                         "std::vector<std::vector<int>> tree{{1}, {0}};\n" + code("euler") + code("diameter") +
                         "\nif (tour.tout(0) != 2 || diameter.length() != 1) return 2;\n"
                         "std::vector<std::vector<long long>> grid{{1, 2}};\n" + code("prefix2d") +
                         "\nif (sum.query(0, 1, 0, 2) != 3) return 3;\n"
                         "int n = 1, m = 1; unsigned long long exponent = 5;\n" + code("matrix") +
                         "\na(0, 0) = 2;\n" + code("matrix-pow") + "\nif (result(0, 0) != 32) return 4;\n}\n")
        self.compile_run(INCLUDES + "#include <atcoder/modint>\nint main() {\n"
                         "std::vector<int> edges{0};\n" + code("functional", {1: "fg", 2: "edges"}) +
                         "\nif (!fg.on_cycle(0)) return 1;\n"
                         "std::vector<std::vector<int>> adjacency(1);\n" +
                         code("euler", {1: "et", 2: "adjacency", 3: "0"}) +
                         "\nif (et.vertex(0) != 0) return 2;\n"
                         "using D = TreeDiameter<int>; std::vector<std::vector<D::Edge>> weighted(1);\n" +
                         code("diameter", {1: "int", 2: "diam", 3: "weighted"}) +
                         "\nif (diam.length() != 0) return 3;\n"
                         "using Mint = atcoder::modint998244353; std::vector<std::vector<Mint>> values{{3}};\n" +
                         code("prefix2d", {1: "Mint", 2: "ps", 3: "values"}) +
                         "\nif (ps.query(0, 1, 0, 1) != 3) return 4;\n" +
                         code("matrix", {1: "Mint", 2: "mat", 3: "1", 4: "1"}) +
                         "\nmat(0, 0) = 3;\n" + code("matrix-pow", {1: "power", 2: "mat", 3: "2"}) +
                         "\nif (power(0, 0) != 9) return 5;\n}\n")


if __name__ == "__main__":
    unittest.main()
