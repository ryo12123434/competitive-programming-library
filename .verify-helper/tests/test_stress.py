import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
import zipfile


REPOSITORY = Path(__file__).resolve().parents[2]
STRESS = REPOSITORY / "tools/stress.py"


class StressTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / "problem with spaces"
        self.root.mkdir()
        self.gen = self.write("gen input.py", "import sys\nprint(int(sys.argv[-2]), int(sys.argv[-1]))\n")
        self.fast = self.write("fast.py", "import sys\nprint(sum(map(int, sys.stdin.buffer.read().split())))\n")
        self.brute = self.write("brute.py", self.fast.read_text())
        self.bad = self.write("bad.py", "import sys\nprint(sum(map(int, sys.stdin.buffer.read().split())) + 1)\n")
        self.out = self.root / ".stress"

    def write(self, name, content):
        path = self.root / name
        path.write_text(content, encoding="utf-8")
        return path

    def invoke(self, *arguments, expected=0):
        result = subprocess.run([sys.executable, str(STRESS), *map(str, arguments)], cwd=self.root,
                                capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        return result

    def arguments(self, fast=None):
        return ["--gen", self.gen, "--fast", fast or self.fast, "--brute", self.brute]

    def meta(self):
        return json.loads((self.out / "meta.json").read_text(encoding="utf-8"))

    def test_success_and_comparison_modes(self):
        result = self.invoke(*self.arguments(), "--tests", 10, "--seed", 20)
        self.assertIn("OK: 10 cases", result.stdout)
        self.assertFalse((self.out / "meta.json").exists())
        self.fast.write_text('print("1  2")\n')
        self.brute.write_text('print("1\\n2")\n')
        self.invoke(*self.arguments(), "--tests", 1)
        self.invoke(*self.arguments(), "--tests", 1, "--compare", "exact", expected=1)
        self.assertEqual(self.meta()["reason"], "wrong_answer")

    def test_first_failure_overwrite_and_seed_reproduction(self):
        result = self.invoke(*self.arguments(self.bad), "--tests", 5, "--seed", 42, expected=1)
        self.assertIn("case=1 seed=42", result.stdout)
        first = (self.out / "failed.in").read_bytes()
        self.assertEqual(first, b"42 1\n")
        self.assertEqual((self.out / "expected.txt").read_bytes(), b"43\n")
        self.assertEqual((self.out / "actual.txt").read_bytes(), b"44\n")
        names = {p.name for p in self.out.iterdir()}
        self.assertFalse((self.out / "collected.zip").exists())
        meta = self.meta()
        self.assertEqual(meta["master_seed"], 42)
        self.assertEqual(meta["case_index"], 1)
        self.assertEqual(meta["cwd"], str(self.root))
        self.assertEqual(meta["results"]["generator"]["command"][-2:], ["42", "1"])
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--seed", 55, "--case", 4, expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"58 4\n")
        self.assertEqual({p.name for p in self.out.iterdir()}, names)
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--seed", 42, expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), first)
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--seed", 2**64 - 1, "--case", 2, expected=1)
        self.assertEqual(self.meta()["case_seed"], 0)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"0 2\n")

    def test_replay_and_saved_configuration(self):
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--seed", 12, "--case", 3,
                    "--timeout", 3, "--compare", "exact", expected=1)
        self.invoke("--replay", expected=1)
        self.assertEqual(self.meta()["case_index"], 3)
        self.assertEqual(self.meta()["case_seed"], 14)
        self.assertEqual(self.meta()["timeout_seconds"], 3)
        self.assertEqual(self.meta()["compare"], "exact")
        self.assertIn("gen", self.meta()["specs"])
        self.bad.write_text(self.fast.read_text())
        self.invoke("--replay")
        self.assertTrue((self.out / "failed.in").is_file())
        self.invoke("--input", self.out / "failed.in", "--fast", self.fast, "--brute", self.brute)

    def test_collection_and_explicit_archive_are_bounded(self):
        for limit in (3, 2):
            self.invoke(*self.arguments(self.bad), "--tests", 20, "--collect", limit, expected=1)
            with zipfile.ZipFile(self.out / "collected.zip") as archive:
                self.assertEqual(len(archive.namelist()), limit * 9)
                self.assertEqual(len({name.split("/")[0] for name in archive.namelist()}), limit)
                self.assertIn(b"wrong_answer", archive.read("0001/meta.json"))
            self.assertEqual(self.meta()["case_index"], limit)
        archive = self.root / "saved" / "bug.zip"
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--archive", archive, expected=1)
        with zipfile.ZipFile(archive) as saved:
            self.assertEqual(len(saved.namelist()), 9)
            self.assertEqual(saved.read("failed.in"), (self.out / "failed.in").read_bytes())
        self.invoke(*self.arguments(), "--tests", 1, "--archive", archive, expected=2)

    def test_abnormal_exit_timeout_and_output_limit(self):
        fixtures = [
            ("crash.py", "import sys\nsys.stderr.write('diagnostic\\n')\nsys.exit(7)\n", "fast_nonzero_exit"),
            ("slow.py", "import time\ntime.sleep(3)\n", "fast_timeout"),
            ("loud.py", "print('x' * 2048)\n", "fast_output_limit"),
        ]
        for name, content, reason in fixtures:
            with self.subTest(reason=reason):
                executable = self.write(name, content)
                self.invoke(*self.arguments(executable), "--tests", 1, "--timeout", .5,
                            "--output-limit", 512, expected=1)
                self.assertEqual(self.meta()["reason"], reason)
                self.assertLessEqual(len((self.out / "actual.txt").read_bytes()), 512)
                if reason == "fast_nonzero_exit":
                    self.assertEqual(self.meta()["results"]["fast"]["returncode"], 7)
                    self.assertIn(b"diagnostic", (self.out / "fast.stderr.txt").read_bytes())
        broken_gen = self.write("broken gen.py", "import sys\nprint('partial input')\nsys.exit(8)\n")
        self.invoke("--gen", broken_gen, "--fast", self.fast, "--brute", self.brute, expected=1)
        self.assertEqual(self.meta()["reason"], "generator_nonzero_exit")
        self.assertEqual((self.out / "failed.in").read_bytes(), b"partial input\n")
        self.invoke(*self.arguments(self.fast.with_name("missing.exe")), "--tests", 1, expected=1)
        self.assertEqual(self.meta()["reason"], "fast_launch_error")
        self.invoke("--gen", self.gen, "--fast", self.fast, "--brute", self.root / "slow.py",
                    "--timeout", .5, "--tests", 1, expected=1)
        self.assertEqual(self.meta()["reason"], "brute_timeout")

    @unittest.skipIf(os.name == "nt", "Windows taskkill は Windows 実機で検証する")
    def test_timeout_kills_child_processes(self):
        marker = self.root / "child survived"
        child = f"import time; from pathlib import Path; time.sleep(1.5); Path({str(marker)!r}).write_text('alive')"
        parent = self.write("parent.py", f"import subprocess, sys, time\nsubprocess.Popen([sys.executable, '-c', {child!r}])\ntime.sleep(5)\n")
        self.invoke(*self.arguments(parent), "--tests", 1, "--timeout", .5, expected=1)
        self.assertEqual(self.meta()["reason"], "fast_timeout")
        time.sleep(1.6)
        self.assertFalse(marker.exists())

    def test_shrinker_accepts_only_smaller_preserved_failure(self):
        self.gen.write_text("print(1000)\n")
        shrinker = self.write("shrink.py", "import sys\nn=int(sys.stdin.read())\nprint(max(1, n//2))\n")
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--shrinker", shrinker, expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"1\n")
        self.assertEqual((self.out / "original.in").read_bytes(), b"1000\n")
        self.assertGreater(self.meta()["shrink"]["accepted_steps"], 0)
        self.invoke("--replay", expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"1\n")
        shrinker.write_text("print('x')\n")
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--shrinker", shrinker, expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"1000\n")
        self.assertEqual(self.meta()["shrink"]["status"], "failure_not_preserved")
        shrinker.write_text("import time\ntime.sleep(3)\n")
        self.invoke(*self.arguments(self.bad), "--tests", 1, "--shrinker", shrinker, "--timeout", .5, expected=1)
        self.assertEqual((self.out / "failed.in").read_bytes(), b"1000\n")
        self.assertEqual(self.meta()["shrink"]["status"], "shrinker_timeout")

    def test_cpp_compilation_local_sanitizer_and_replay_rebuild(self):
        generator = self.write("generator.cpp", '#include "library/debug/random.hpp"\n#include <iostream>\n'
                               'int main(int argc,char** argv) { if(argc<3) return 2;\n'
                               'cpl::random::Random rng(std::stoull(argv[1])); std::cout<<rng.integer(1,100); }\n')
        fast = self.write("solution.cpp", '#include "library/debug/debug.hpp"\n#include <iostream>\n'
                          'int main() { long long x; std::cin>>x; debug(x); std::cout<<x+1; }\n')
        brute = self.write("naive.cpp", '#include <iostream>\nint main(){ long long x; std::cin>>x; std::cout<<x; }\n')
        sanitizer = [] if os.name == "nt" else ["--sanitize", "undefined"]
        self.invoke("--gen", generator, "--fast", fast, "--brute", brute,
                    "--tests", 1, "--local", *sanitizer, expected=1)
        self.assertIn(b"x)", (self.out / "fast.stderr.txt").read_bytes())
        self.assertTrue(self.meta()["local"])
        self.assertEqual(self.meta()["sanitize"], None if os.name == "nt" else "undefined")
        fast.write_text(brute.read_text())
        self.invoke("--replay")
        if os.name != "nt":
            fast.write_text('#include <climits>\n#include <iostream>\n'
                            'int main(){ long long x; std::cin>>x; std::cout<<(LLONG_MAX+x); }\n')
            self.invoke("--gen", generator, "--fast", fast, "--brute", brute,
                        "--tests", 1, "--sanitize", "undefined", expected=1)
            self.assertEqual(self.meta()["reason"], "fast_nonzero_exit")
            self.assertIn(b"runtime error", (self.out / "fast.stderr.txt").read_bytes())
        before = (self.out / "failed.in").read_bytes()
        fast.write_text("not valid C++\n")
        result = self.invoke("--replay", expected=2)
        self.assertIn("compile fast", result.stderr)
        self.assertEqual((self.out / "failed.in").read_bytes(), before)

    def test_json_argv_and_validation(self):
        gen_command = json.dumps([sys.executable, str(self.gen), "extra argument with spaces"])
        self.invoke("--gen", gen_command, "--fast", self.fast, "--brute", self.brute, "--tests", 1)
        for options in [["--tests", 0], ["--seed", -1], ["--timeout", "nan"], ["--collect", 0], ["--output-limit", 0]]:
            with self.subTest(options=options):
                self.invoke(*self.arguments(), *options, expected=2)
        self.invoke(*self.arguments(), "--fast", '["unterminated"', expected=2)


if __name__ == "__main__":
    unittest.main()
