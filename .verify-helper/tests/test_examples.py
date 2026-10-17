from pathlib import Path
import re
import subprocess
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]


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


if __name__ == "__main__":
    unittest.main()
