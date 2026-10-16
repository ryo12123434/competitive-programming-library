from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest

REPOSITORY = Path(__file__).resolve().parents[2]


class DebugTest(unittest.TestCase):
    def compile_run(self, source, directory, *flags):
        executable = Path(directory) / "debug.exe"
        result = subprocess.run(
            ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", "-I", str(REPOSITORY),
             *flags, str(source), "-o", str(executable)], text=True, capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "")

    def test_regression_local_and_submission(self):
        configurations = [[], ["-DLOCAL"]]
        if os.name != "nt":
            configurations.append(["-DLOCAL", "-fsanitize=undefined", "-fno-sanitize-recover=all"])
        for flags in configurations:
            with self.subTest(flags=flags), tempfile.TemporaryDirectory() as directory:
                self.compile_run(REPOSITORY / ".verify-helper/tests/fixtures/debug_random.cpp", directory, *flags)

    def test_bundled_debug_library(self):
        source = REPOSITORY / ".verify-helper/tests/fixtures/debug_random.cpp"
        result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(source), "--stdout"],
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(any(line.lstrip().startswith('#include "library/debug/') or
                             line.lstrip().startswith('#include "debug.hpp"') for line in result.stdout.splitlines()))
        with tempfile.TemporaryDirectory() as directory:
            bundled = Path(directory) / "bundled.cpp"
            bundled.write_text(result.stdout, encoding="utf-8")
            self.compile_run(bundled, directory)
            self.compile_run(bundled, directory, "-DLOCAL")

    def test_existing_debug_macro_and_local_linkage(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = '#include "library/debug/debug.hpp"\n#include "library/debug/table_debug.hpp"\n#include "library/debug/timer.hpp"\n#include "library/debug/random.hpp"\n'
            first, second = root / "first.cpp", root / "second.cpp"
            first.write_text('#define debug(...) existing_macro_must_remain\n' + header +
                             '#include <cassert>\n#define STRING_INNER(x) #x\n#define STRING(x) STRING_INNER(x)\n'
                             'int other(); int main() { assert(std::string(STRING(debug(1))) == "existing_macro_must_remain"); return other(); }\n')
            second.write_text('#define CPL_DEBUG_NO_SHORT_MACRO\n' + header +
                              'int debug(int x) { return x; }\nint other() { CPL_DEBUG(debug(1)); return 0; }\n')
            self.compile_run(first, root, "-DLOCAL", str(second))


if __name__ == "__main__":
    unittest.main()
