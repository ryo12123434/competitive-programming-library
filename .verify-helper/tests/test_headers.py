from pathlib import Path
import re
import subprocess
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
HEADERS = sorted((REPOSITORY / "library").rglob("*.hpp"))


class HeaderTest(unittest.TestCase):
    def compile(self, command, **kwargs):
        result = subprocess.run(command, text=True, capture_output=True, **kwargs)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_self_contained_headers(self):
        for header in HEADERS:
            with self.subTest(header=str(header.relative_to(REPOSITORY))):
                self.compile(
                    ["g++", "-std=gnu++23", "-Wall", "-Wextra", "-Werror", "-fsyntax-only", "-x", "c++", "-"],
                    input=f'#include "{header}"\n',
                )

    def test_multiple_translation_units(self):
        # ACL の重複定義は下の専用テストで実際に再現し、FPS の追加の衝突がないことも検証する。
        # 残りの自作ヘッダは複数の翻訳単位に読み込み、関数の重複定義も検出する。
        includes = "".join(f'#include "{header}"\n' for header in HEADERS if header.name != "formal_power_series.hpp")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = root / "first.cpp"
            second = root / "second.cpp"
            first.write_text(includes + "int other(); int main() { return other(); }\n", encoding="utf-8")
            second.write_text(includes + "int other() { return 0; }\n", encoding="utf-8")
            self.compile(
                ["g++", "-std=gnu++23", str(first), str(second), "-o", str(root / "headers")],
            )
            subprocess.run([str(root / "headers")], check=True, capture_output=True)

    def test_fps_multiple_translation_unit_exclusion(self):
        # ACL 単体と FPS の衝突を比較する。ACL 更新で解消したら除外を見直す。
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            duplicated = {}
            for name, include in [
                ("acl", "#include <atcoder/convolution>\n"),
                ("fps", f'#include "{REPOSITORY / "library/math/formal_power_series.hpp"}"\n'),
            ]:
                first = root / f"{name}_first.cpp"
                second = root / f"{name}_second.cpp"
                first.write_text(include + "int other(); int main() { return other(); }\n", encoding="utf-8")
                second.write_text(include + "int other() { return 0; }\n", encoding="utf-8")
                result = subprocess.run(
                    ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", str(first), str(second), "-o", str(root / name)],
                    text=True, capture_output=True,
                )
                self.assertNotEqual(result.returncode, 0, "Linking now succeeds; reconsider the FPS exclusion.")
                duplicated[name] = set(re.findall(r"multiple definition of [`']([^']+)'", result.stderr))
                expected = (
                    "atcoder::internal::countr_zero(",
                    "atcoder::internal::floor_sum_unsigned(",
                    "atcoder::convolution_ll(",
                )
                self.assertEqual(len(duplicated[name]), len(expected), result.stderr)
                for prefix in expected:
                    self.assertTrue(any(symbol.startswith(prefix) for symbol in duplicated[name]), result.stderr)
            self.assertEqual(duplicated["fps"], duplicated["acl"])


if __name__ == "__main__":
    unittest.main()
