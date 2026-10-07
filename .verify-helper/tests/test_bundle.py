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

    def test_cli_stdout_modes(self):
        source = self.write("submission.cpp", '#include <iostream>\nint main() { std::cout << 42; }\n')
        expected = bundler.bundle(source)

        for args in [
            ["bundle.py", str(source), "--stdout"],
            ["bundle.py", str(source), "-o", "-"],
        ]:
            with self.subTest(args=args):
                stdout = io.StringIO()
                with patch.object(bundler.sys, "argv", args):
                    with contextlib.redirect_stdout(stdout):
                        self.assertEqual(bundler.main(), 0)
                self.assertEqual(stdout.getvalue(), expected)

        self.assertFalse(bundler.default_output_path(source).exists())

    def test_cli_default_output_is_preserved(self):
        source = self.write("submission.cpp", "int main() {}\n")
        output = bundler.default_output_path(source)
        stdout = io.StringIO()

        with patch.object(bundler.sys, "argv", ["bundle.py", str(source)]):
            with contextlib.redirect_stdout(stdout):
                self.assertEqual(bundler.main(), 0)

        self.assertEqual(output.read_text(encoding="utf-8"), bundler.bundle(source))
        self.assertEqual(stdout.getvalue().strip(), str(output.resolve()))

    def test_cli_failure_does_not_overwrite_output(self):
        output = self.write("result.cpp", "keep this file\n")
        with patch.object(bundler.sys, "argv", ["bundle.py", str(self.root / "missing.cpp"), "-o", str(output)]):
            with contextlib.redirect_stderr(io.StringIO()) as errors:
                self.assertEqual(bundler.main(), 1)
        self.assertIn("input file not found", errors.getvalue())
        self.assertEqual(output.read_text(encoding="utf-8"), "keep this file\n")

    def test_acl_expansion_is_opt_in(self):
        source = self.write("submission.cpp", '#include <atcoder/modint>\n#include "external.hpp"\n')
        with patch.object(bundler, "find_acl_directory", side_effect=AssertionError("ACL must not be searched")):
            self.assertEqual(bundler.bundle(source), source.read_text(encoding="utf-8"))
            self.assertEqual(bundler.bundle(source, acl_dir=self.root / "missing"), source.read_text(encoding="utf-8"))

    def test_acl_transitive_expansion_duplicates_and_cli(self):
        self.write("acl/atcoder/detail.hpp", "#pragma once\ninline int value() { return 42; }\n")
        self.write("acl/atcoder/sample.hpp", '#pragma once\n#include "atcoder/detail.hpp"\n')
        self.write("acl/atcoder/sample", '#include "atcoder/sample.hpp"\n')
        self.write("library/root.hpp", '#pragma once\n#include <atcoder/sample> /* dependency */\n')
        source = self.write(
            "submission.cpp", '#include <iostream>\n#include "library/root.hpp"\n'
            '#include <atcoder/sample.hpp> // duplicate\n#include "atcoder/detail.hpp"\n'
            'int main() { std::cout << value(); }\n',
        )
        output = self.root / "bundled.cpp"
        with patch.object(bundler.sys, "argv", ["bundle.py", str(source), "--expand-acl", "--acl-dir", str(self.root / "acl"), "-o", str(output)]):
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(bundler.main(), 0)
        result = output.read_text(encoding="utf-8")
        self.assertEqual(result.count("inline int value()"), 1)
        self.assertNotIn('#include "', result)
        self.assertNotIn("#include <atcoder/", result)
        self.assertIn("#include <iostream>", result)
        for output_args in [["--stdout"], ["-o", "-"]]:
            with self.subTest(output_args=output_args):
                arguments = ["bundle.py", str(source), "--expand-acl", "--acl-dir", str(self.root / "acl"), *output_args]
                with patch.object(bundler.sys, "argv", arguments):
                    with contextlib.redirect_stdout(io.StringIO()) as stdout:
                        self.assertEqual(bundler.main(), 0)
                self.assertEqual(stdout.getvalue(), result)
        subprocess.run(["g++", "-std=gnu++23", str(output), "-o", str(self.root / "submission")], check=True, capture_output=True)
        execution = subprocess.run([str(self.root / "submission")], check=True, capture_output=True, text=True)
        self.assertEqual(execution.stdout, "42")

    def test_acl_directory_search_and_explicit_override(self):
        package = self.root / "acl" / "atcoder"
        package.mkdir(parents=True)
        other = self.root / "other" / "atcoder"
        other.mkdir(parents=True)
        for variable in ["CPATH", "CPLUS_INCLUDE_PATH"]:
            with patch.dict(bundler.os.environ, {variable: str(package.parent)}, clear=True):
                self.assertEqual(bundler.find_acl_directory(None), package)
                self.assertEqual(bundler.find_acl_directory(other.parent), other)
                self.assertEqual(bundler.find_acl_directory(other), other)
                self.assertIsNone(bundler.find_acl_directory(self.root / "missing"))

    def test_acl_errors_do_not_overwrite_output(self):
        self.write("acl/atcoder/present.hpp", '#include "atcoder/missing.hpp"\n')
        self.write("acl/outside.hpp", "inline int outside() { return 0; }\n")
        for include, directory, message in [
            ("atcoder/present.hpp", self.root / "missing", "cannot find ACL"),
            ("atcoder/missing.hpp", self.root / "acl", "cannot resolve ACL"),
            ("atcoder/present.hpp", self.root / "acl", "cannot resolve ACL"),
            ("atcoder/../outside.hpp", self.root / "acl", "cannot resolve ACL"),
        ]:
            with self.subTest(include=include, directory=directory):
                source = self.write("submission.cpp", f"#include <{include}>\n")
                output = self.write("result.cpp", "keep this file\n")
                with patch.object(bundler.sys, "argv", ["bundle.py", str(source), "--expand-acl", "--acl-dir", str(directory), "-o", str(output)]):
                    with contextlib.redirect_stderr(io.StringIO()) as errors:
                        self.assertEqual(bundler.main(), 1)
                self.assertIn(message, errors.getvalue())
                self.assertEqual(output.read_text(encoding="utf-8"), "keep this file\n")

    def test_installed_acl_all_compiles_without_acl_include_path(self):
        acl_directory = bundler.find_acl_directory(None)
        if acl_directory is None:
            self.skipTest("ACL is not installed; configure CPLUS_INCLUDE_PATH")
        source = self.write(
            "submission.cpp", '#include <atcoder/all>\n#include <atcoder/modint>\n#include <iostream>\n'
            'int main() { atcoder::fenwick_tree<int> f(3); f.add(1, 7); '
            'atcoder::modint998244353 x = 3; std::cout << f.sum(0, 3) + x.pow(2).val(); }\n',
        )
        result = bundler.bundle(source, expand_acl=True, acl_dir=acl_directory)
        self.assertNotIn('#include "atcoder/', result)
        self.assertNotIn("#include <atcoder/", result)
        output = self.write("bundled.cpp", result)
        # include パスの環境変数を外し、展開結果だけでコンパイルできることを確認する。
        environment = {key: value for key, value in bundler.os.environ.items() if key not in {"CPATH", "CPLUS_INCLUDE_PATH"}}
        subprocess.run(["g++", "-std=gnu++23", str(output), "-o", str(self.root / "submission")], env=environment, check=True, capture_output=True)
        execution = subprocess.run([str(self.root / "submission")], check=True, capture_output=True, text=True)
        self.assertEqual(execution.stdout, "16")


if __name__ == "__main__":
    unittest.main()
