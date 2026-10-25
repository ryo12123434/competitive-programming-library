from pathlib import Path
import itertools
import os
import random
import re
import subprocess
import sys
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
FIXTURE = REPOSITORY / ".verify-helper/tests/fixtures/hld_li_chao_regression.cpp"
HEADERS = ["graph/hld.hpp", "data_structure/li_chao_tree.hpp"]


class HLDLiChaoTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.root = Path(cls.directory.name)
        cls.binary = cls.root / "regression.exe"
        flags = ["-fsanitize=undefined", "-fno-sanitize-recover=all"] if os.name != "nt" else []
        result = subprocess.run(
            ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", *flags,
             "-I", str(REPOSITORY), str(FIXTURE), "-o", str(cls.binary)],
            text=True, capture_output=True, timeout=120,
        )
        if result.returncode:
            cls.directory.cleanup()
            raise RuntimeError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def run_fixture(self, *args, data=None):
        result = subprocess.run([str(self.binary), *args], input=data, text=True, capture_output=True,
                                cwd=self.root, timeout=60)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.splitlines()

    def check_lines(self, xs, operations):
        lines, expected = [], []
        for op in operations:
            if op[0] == "a":
                lines.append(op[1:])
            else:
                x = op[1]
                expected.append(str(min(a * x + b for a, b in lines)) if lines else "empty")
        data = f"{len(xs)} {len(operations)}\n" + " ".join(map(str, xs)) + "\n"
        data += "\n".join(" ".join(map(str, op)) for op in operations) + "\n"
        self.assertEqual(self.run_fixture("li", data=data), expected)

    def compile_run(self, source, *flags, include_repository=True):
        binary = self.root / "example.exe"
        result = subprocess.run(
            ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", *flags,
             *(["-I", str(REPOSITORY)] if include_repository else []),
             "-x", "c++", "-", "-o", str(binary)],
            input=source, text=True, capture_output=True, timeout=120,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(binary)], text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_hld_exhaustive_bfs_and_noncommutative(self):
        self.run_fixture()

    def test_li_chao_empty_duplicate_and_crossing(self):
        self.check_lines([], [("a", 1, 2), ("a", 1, 2)])
        self.check_lines([0], [("q", 0), ("a", 0, 0), ("q", 0), ("a", 0, 1), ("q", 0),
                               ("a", 0, -1), ("q", 0), ("a", 0, -1), ("q", 0)])
        xs = [10, -10, 0, -1, 1, 10]
        ops = [("q", x) for x in xs]
        for line in [(2, 1), (2, 8), (2, -3), (-2, 0), (0, -1), (0, -1)]:
            ops.append(("a", *line))
            ops.extend(("q", x) for x in xs)
        self.check_lines(xs, ops)

    def test_li_chao_full_int64_big_integer_oracle(self):
        limits = [-(1 << 63), -(1 << 63) + 1, -1, 0, 1, (1 << 63) - 2, (1 << 63) - 1]
        operations = [("q", x) for x in limits]
        for a, b in itertools.product(limits, repeat=2):
            operations.append(("a", a, b))
            operations.extend(("q", x) for x in limits)
        self.check_lines(list(reversed(limits)) + limits, operations)
        # 追加順で支配されて隠れないよう、各直線を単独でも比較する。
        for a, b in itertools.product(limits, repeat=2):
            self.check_lines(limits, [("a", a, b)] + [("q", x) for x in limits])

    def test_li_chao_all_small_line_pairs(self):
        xs = [-3, -1, 0, 2, 5]
        lines = list(itertools.product(range(-2, 3), repeat=2))
        for first, second in itertools.product(lines, repeat=2):
            with self.subTest(first=first, second=second):
                self.check_lines(xs, [("a", *first)] + [("q", x) for x in xs] +
                                 [("a", *second)] + [("q", x) for x in xs])

    def test_li_chao_random_operations(self):
        rng = random.Random(20261025)
        for size in [1, 2, 3, 5, 17, 31, 64, 101]:
            xs = [rng.randrange(-(1 << 63), 1 << 63) for _ in range(size)]
            rng.shuffle(xs)
            operations = [("q", x) for x in xs]
            previous = (0, 0)
            for _ in range(1000):
                if rng.randrange(2):
                    a, b = previous if rng.randrange(5) == 0 else (rng.randrange(-(1 << 63), 1 << 63),
                                                                 rng.randrange(-(1 << 63), 1 << 63))
                    previous = (a, b)
                    operations.append(("a", a, b))
                else:
                    operations.append(("q", rng.choice(xs)))
            operations.extend(("q", x) for x in xs)
            self.check_lines(xs, operations)

    def test_invalid_preconditions(self):
        for mode in ["invalid-coordinate", "empty-tree", "disconnected", "cycle", "invalid-root"]:
            with self.subTest(mode=mode):
                result = subprocess.run([str(self.binary), mode], text=True, capture_output=True,
                                        cwd=self.root, timeout=10)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Assertion", result.stderr)
                self.assertNotIn("runtime error:", result.stderr)

    def test_headers_and_release_instantiations(self):
        for name in HEADERS:
            for flags in [[], ["-DNDEBUG"]]:
                result = subprocess.run(["g++", "-std=gnu++23", "-Wall", "-Wextra", "-Werror", *flags,
                                         "-I", str(REPOSITORY), "-x", "c++", "-", "-fsyntax-only"],
                                        input=f'#include "library/{name}"\n', text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
        source = ('#include "library/graph/hld.hpp"\n#include "library/data_structure/li_chao_tree.hpp"\n'
                  'int main() { HLD hld(std::vector<std::vector<int>>{{1},{0}});\n'
                  'if (hld.path_segments(0,1,true).size()!=1 || hld.lca(0,1)!=0) return 1;\n'
                  'LiChaoTree tree({-1,1}); tree.add_line(2,3);\n'
                  'if (tree.query(-1)!=1 || tree.query(1)!=5) return 2; }\n')
        self.compile_run(source, "-DNDEBUG")
        path = self.root / "submission.cpp"
        path.write_text(source, encoding="utf-8")
        result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(path), "--stdout"],
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.compile_run(result.stdout, include_repository=False)

    def test_documentation_examples(self):
        for name in ["graph/hld.md", "data_structure/li_chao_tree.md"]:
            text = (REPOSITORY / "library" / name).read_text(encoding="utf-8")
            blocks = re.findall(r"```cpp\n(.*?)```", text, re.S)
            self.assertTrue(blocks)
            for block in blocks:
                self.compile_run(block)


if __name__ == "__main__":
    unittest.main()
