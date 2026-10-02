from pathlib import Path
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
        # ACL 自体に非 inline 定義があるため、ACL 依存の FPS は単独コンパイルだけ検証する。
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


if __name__ == "__main__":
    unittest.main()
