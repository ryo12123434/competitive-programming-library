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
        fixture = (REPOSITORY / ".verify-helper/tests/fixtures/debug_random.cpp").read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as directory:
            for header in ("debug.hpp", "table_debug.hpp"):
                with self.subTest(header=header):
                    source = Path(directory) / "source.cpp"
                    source.write_text(fixture.replace('library/debug/debug.hpp', f'library/debug/{header}'),
                                      encoding="utf-8")
                    result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(source), "--stdout"],
                                            text=True, capture_output=True)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertFalse(any(line.lstrip().startswith('#include "library/debug/') or
                                         line.lstrip().startswith('#include "debug.hpp"') for line in result.stdout.splitlines()))
                    self.assertEqual(result.stdout.count("struct TableOptions {"), 1)
                    bundled = Path(directory) / "bundled.cpp"
                    bundled.write_text(result.stdout, encoding="utf-8")
                    self.compile_run(bundled, directory)
                    self.compile_run(bundled, directory, "-DLOCAL")

    def test_table_header_compatibility(self):
        body = r'''
#include <cassert>
int main() {
    std::vector<std::vector<int>> table{{1, 2}};
    cpl::debug::TableOptions view;
    const std::string expected = "+-----+-----+-----+\n| r/c |   0 |   1 |\n+-----+-----+-----+\n|   0 |   1 |   2 |\n+-----+-----+-----+\n";
    assert(cpl::debug::render_table(table, view) == expected);
    std::ostringstream captured;
    auto* previous = std::cerr.rdbuf(captured.rdbuf());
    cpl::debug::log_table("file", 7, "table", table, view);
    int evaluations = 0;
    CPL_DEBUG_TABLE((++evaluations, table), view);
    debug_table((++evaluations, table));
    std::cerr.rdbuf(previous);
#ifdef LOCAL
    assert(evaluations == 2);
    assert(captured.str().find(expected) != std::string::npos);
#else
    assert(evaluations == 0);
    assert(captured.str() == "file:7 | table\n" + expected);
    CPL_DEBUG_TABLE(undefined_table(), undefined_options());
    debug_table(undefined_table(), undefined_options());
#endif
}
'''
        for headers in [("debug.hpp",), ("table_debug.hpp",),
                        ("debug.hpp", "table_debug.hpp"), ("table_debug.hpp", "debug.hpp")]:
            for flags in ([], ["-DLOCAL"]):
                with self.subTest(headers=headers, flags=flags), tempfile.TemporaryDirectory() as directory:
                    source = Path(directory) / "table.cpp"
                    includes = "".join(f'#include "library/debug/{header}"\n' for header in headers)
                    source.write_text(includes + body, encoding="utf-8")
                    self.compile_run(source, directory, *flags)

    def test_existing_debug_macro_and_local_linkage(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = '#include "library/debug/debug.hpp"\n#include "library/debug/table_debug.hpp"\n#include "library/debug/timer.hpp"\n#include "library/debug/random.hpp"\n'
            first, second = root / "first.cpp", root / "second.cpp"
            first.write_text('#define debug(...) existing_macro_must_remain\n'
                             '#define debug_table(...) existing_table_macro_must_remain\n' + header +
                             '#include <cassert>\n#define STRING_INNER(x) #x\n#define STRING(x) STRING_INNER(x)\n'
                             'int other(); int main() { assert(std::string(STRING(debug(1))) == "existing_macro_must_remain");'
                             'assert(std::string(STRING(debug_table(1))) == "existing_table_macro_must_remain"); return other(); }\n')
            second.write_text('#define CPL_DEBUG_NO_SHORT_MACRO\n' + header +
                              'int debug(int x) { return x; }\nint debug_table(int x) { return x; }\n'
                              'int other() { CPL_DEBUG(debug(1), debug_table(2)); return 0; }\n')
            self.compile_run(first, root, "-DLOCAL", str(second))


if __name__ == "__main__":
    unittest.main()
