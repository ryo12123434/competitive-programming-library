import contextlib
import importlib.util
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


REPOSITORY = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("submission_bundle", REPOSITORY / "bundle.py")
bundler = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(bundler)


class BundleTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.library = self.root / "library"
        self.library.mkdir()
        self.patcher = patch.multiple(bundler, ROOT=self.root, LIBRARY_DIR=self.library)
        self.patcher.start()
        self.addCleanup(self.patcher.stop)

    def write(self, name, content):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        return path

    def test_transitive_duplicate_includes_and_comments(self):
        self.write("library/detail.hpp", "#pragma once /* guard */\ninline int value() { return 42; }\n")
        self.write("library/root.hpp", '#pragma once\n#include "detail.hpp" /* dependency */ // note\n')
        source = self.write(
            "submission.cpp",
            '#include <iostream>\n#include "library/root.hpp" /* root */\n'
            '#include "root.hpp" // duplicate\nint main() { std::cout << value(); }\n',
        )
        result = bundler.bundle(source)
        self.assertEqual(result.count("inline int value()"), 1)
        self.assertNotIn("#pragma once", result)
        self.assertNotIn('#include "', result)
        # リポジトリ外に出力した提出ファイルだけでコンパイル・実行できることを確認する。
        with tempfile.TemporaryDirectory() as output_directory:
            output = Path(output_directory) / "submission.cpp"
            executable = Path(output_directory) / "submission"
            output.write_text(result, encoding="utf-8")
            subprocess.run(["g++", "-std=gnu++23", str(output), "-o", str(executable)], check=True, capture_output=True)
            execution = subprocess.run([str(executable)], check=True, capture_output=True, text=True)
            self.assertEqual(execution.stdout, "42")

    def test_cyclic_includes_are_expanded_once(self):
        self.write("library/a.hpp", '#pragma once\n#include "b.hpp"\nstruct A {};\n')
        self.write("library/b.hpp", '#pragma once\n#include "a.hpp"\nstruct B {};\n')
        source = self.write("submission.cpp", '#include "library/a.hpp"\n')
        result = bundler.bundle(source)
        self.assertEqual(result.count("struct A"), 1)
        self.assertEqual(result.count("struct B"), 1)

    def test_external_includes_are_preserved(self):
        source = self.write("submission.cpp", '#include <atcoder/all>\n#include "external.hpp"\n')
        self.assertEqual(bundler.bundle(source), source.read_text(encoding="utf-8"))

    def test_missing_library_dependency_is_an_error(self):
        source = self.write("submission.cpp", '#include "library/missing.hpp"\n')
        with self.assertRaisesRegex(bundler.BundleError, "cannot resolve"):
            bundler.bundle(source)
        self.write("library/root.hpp", '#include "missing.hpp"\n')
        source.write_text('#include "library/root.hpp"\n', encoding="utf-8")
        with self.assertRaisesRegex(bundler.BundleError, "cannot resolve"):
            bundler.bundle(source)

    def test_cli_failure_does_not_overwrite_output(self):
        output = self.write("result.cpp", "keep this file\n")
        with patch.object(bundler.sys, "argv", ["bundle.py", str(self.root / "missing.cpp"), "-o", str(output)]):
            with contextlib.redirect_stderr(io.StringIO()) as errors:
                self.assertEqual(bundler.main(), 1)
        self.assertIn("input file not found", errors.getvalue())
        self.assertEqual(output.read_text(encoding="utf-8"), "keep this file\n")


if __name__ == "__main__":
    unittest.main()
