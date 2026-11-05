from pathlib import Path
import os
import re
import subprocess
import sys
import tempfile
import unittest

from test_snippets import expand, load_snippets


ROOT = Path(__file__).resolve().parents[2]
HEADERS = ["int128_io", "integer_division", "integer_sqrt", "safe_arithmetic", "integer_digits",
           "detail/integer_traits"]
INCLUDES = "".join(f'#include "library/math/{name}.hpp"\n' for name in HEADERS)
HAS_BOOST = subprocess.run(
    ["g++", "-std=c++20", "-x", "c++", "-", "-fsyntax-only"],
    input="#include <boost/multiprecision/cpp_int.hpp>\n", text=True, capture_output=True,
).returncode == 0


class IntegerCompileTest(unittest.TestCase):
    def compile_and_run(self, source, standard="gnu++23", flags=(), includes=True):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "integer.exe"
            result = subprocess.run(
                ["g++", f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Werror", *flags,
                 *(["-I", str(ROOT)] if includes else []), "-x", "c++", "-", "-o", str(executable)],
                input=source, text=True, capture_output=True, cwd=directory,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            return result

    def test_standalone_cxx20_and_gnucxx23(self):
        for standard in ("c++20", "gnu++23"):
            for name in HEADERS:
                with self.subTest(standard=standard, header=name):
                    result = subprocess.run(
                        ["g++", f"-std={standard}", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT),
                         "-x", "c++", "-", "-fsyntax-only"],
                        input=f'#include "library/math/{name}.hpp"\n', text=True, capture_output=True,
                    )
                    self.assertEqual(result.returncode, 0, result.stderr)

    def test_multiple_translation_units(self):
        include = INCLUDES + '#include "library/debug/debug.hpp"\n'
        # インライン関数と関数テンプレートを両翻訳単位で実際に使用する。
        first = include + '''
int other();
int main() {
    auto value = parse_int128("-123");
    return !value || to_string_int128(*value) != "-123" || other() != 7;
}
'''
        second = include + '''
int other() {
    if (cpl::debug::repr(__uint128_t(123)) != "123") return 0;
    return *floor_div(-7, 3) + digit_sum(-100) + isqrt(81);
}
'''
        for standard in ("c++20", "gnu++23"):
            with self.subTest(standard=standard), tempfile.TemporaryDirectory() as directory:
                paths = [Path(directory) / f"part{i}.cpp" for i in range(2)]
                for path, text in zip(paths, (first, second)):
                    path.write_text(text, encoding="utf-8")
                exe = Path(directory) / "multi.exe"
                result = subprocess.run(
                    ["g++", f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT),
                     *map(str, paths), "-o", str(exe)], text=True, capture_output=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(subprocess.run([str(exe)], capture_output=True).returncode, 0)

    def test_no_implicit_boost_dependency(self):
        result = subprocess.run(
            ["g++", "-std=c++20", "-I", str(ROOT), "-x", "c++", "-", "-M"],
            input=INCLUDES + '#include "library/debug/debug.hpp"\n', text=True, capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("boost/", result.stdout)

    def test_template_and_digit_sum_compatibility(self):
        template = (ROOT / "template/template.cpp").read_text(encoding="utf-8")
        for first in (False, True):
            source = INCLUDES if first else ""
            source += "#define main original_template_main\n" + template + "\n#undef main\n"
            source += "" if first else INCLUDES
            source += '''
static_assert(std::is_same_v<i128, __int128_t> && std::is_same_v<u128, __uint128_t>);
static_assert(std::is_same_v<ll, long long> && std::is_same_v<ull, unsigned long long>);
int main() {
    assert(digit_sum("123") == 6 && digit_sum(std::string("456")) == 15);
    assert(digit_sum(-123LL) == 6 && digit_sum(i128(-123)) == 6);
    return original_template_main();
}
'''
            with self.subTest(include_first=first):
                # 元のテンプレートに既存の警告だけを除外する。
                self.compile_and_run(source, flags=("-Wno-unknown-pragmas", "-Wno-sign-compare"))

    @unittest.skipUnless(HAS_BOOST, "Boost cpp_int のヘッダがないため相互運用の検証は未実施")
    def test_boost_include_order_and_standard_streams(self):
        for first in (False, True):
            headers = ['#include <boost/multiprecision/cpp_int.hpp>\n', '#include "library/debug/debug.hpp"\n']
            if not first:
                headers.reverse()
            source = "".join(headers) + '''
#include <cassert>
#include <sstream>
int main() {
    boost::multiprecision::cpp_int big = 1;
    big <<= 200;
    std::ostringstream out;
    out << 42 << ' ' << big << ' ' << 3.5;
    assert(out.str() == "42 " + big.str() + " 3.5");
    assert(cpl::debug::repr(big) == big.str());
    assert(cpl::debug::repr(std::vector<boost::multiprecision::cpp_int>{big}) == "[" + big.str() + "]");
    std::istringstream in("42");
    unsigned ordinary = 0;
    in >> ordinary;
    assert(ordinary == 42);
}
'''
            with self.subTest(boost_first=first):
                self.compile_and_run(source, "c++20")

    def test_doxygen_examples(self):
        for name in HEADERS:
            header = ROOT / f"library/math/{name}.hpp"
            examples = re.findall(r"@code\s*\n(.*?)@endcode", header.read_text(encoding="utf-8"), re.S)
            self.assertTrue(examples, name)
            for block in examples:
                source = re.sub(r"^\s*\* ?", "", block, flags=re.M)
                with self.subTest(header=name):
                    self.compile_and_run(source, "c++20")

    @unittest.skipUnless(HAS_BOOST, "Boost cpp_int のヘッダがないため多倍長整数の使用例は未実施")
    def test_boost_documentation_example(self):
        doc = (ROOT / "library/math/integer.md").read_text(encoding="utf-8")
        for block in re.findall(r"```cpp\n(.*?)```", doc, re.S):
            self.compile_and_run(block, "c++20")

    def test_core_bundle_and_release(self):
        source = INCLUDES + '''
int main() {
    auto min = parse_int128("-170141183460469231731687303715884105728");
    auto max = parse_uint128("340282366920938463463374607431768211455");
    if (!min || !max || digit_count(*min) != 39) return 1;
    if (to_string_int128(*min) != "-170141183460469231731687303715884105728") return 2;
    if (isqrt(*max) != __uint128_t(18446744073709551615ULL)) return 3;
    if (floor_div(*min, __int128_t(-1)) || checked_mul(*max, __uint128_t(2))) return 4;
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.cpp"
            path.write_text(source, encoding="utf-8")
            bundled = subprocess.run([sys.executable, str(ROOT / "bundle.py"), str(path), "--stdout"],
                                     text=True, capture_output=True)
            self.assertEqual(bundled.returncode, 0, bundled.stderr)
        self.assertNotRegex(bundled.stdout, r'(?m)^\s*#include\s+"(?:library/|detail/)')
        self.compile_and_run(bundled.stdout, "c++20", ("-DNDEBUG",), includes=False)

    @unittest.skipUnless(HAS_BOOST, "Boost cpp_int のヘッダがないため全整数スニペットの検証は未実施")
    def test_all_integer_snippets_and_bundle(self):
        snippets = load_snippets()

        def snippet(prefix, replacements=None):
            return expand(snippets[prefix]["body"], replacements) + "\n"

        source = INCLUDES + "#include <iostream>\n#include <sstream>\n#include <cassert>\n"
        source += snippet("bigint", {1: "Big"})
        source += '''
int examples() {
    std::istringstream signed_in("-170141183460469231731687303715884105728");
    std::istringstream unsigned_in("340282366920938463463374607431768211455");
    std::ostringstream signed_out, unsigned_out;
'''
        source += snippet("int128-io", {1: "signed_value", 2: "signed_in", 3: "signed_out"})
        source += snippet("uint128-io", {1: "unsigned_value", 2: "unsigned_in", 3: "unsigned_out"})
        source += '''
    assert(signed_out.str() == "-170141183460469231731687303715884105728\\n");
    assert(unsigned_out.str() == "340282366920938463463374607431768211455\\n");
'''
        source += snippet("bigint-convert", {2: "signed_value"})
        source += 'assert(narrowed && *narrowed == signed_value && big == Big(to_string_int128(small)));\n'
        source += "long long a = -7, b = 3; unsigned long long x = ~0ULL;\n"
        source += snippet("integer-div") + "assert(lower == -3 && upper == -2);\n"
        source += snippet("isqrt") + "assert(root == 4294967295ULL);\n"
        source += snippet("wide-mul", {1: "wide_product"}) + "assert(wide_product == -21);\n"
        source += snippet("checked-int") + "assert(sum == -4 && product == -21);\n"
        source += snippet("integer-div", {1: "low128", 2: "__int128_t", 3: "small", 4: "-1", 5: "high128"})
        source += "assert(!low128 && !high128);\nreturn 73;\n}\nint main(){assert(examples() == 73);}\n"
        self.compile_and_run(source, "c++20")
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.cpp"
            path.write_text(source, encoding="utf-8")
            bundled = subprocess.run([sys.executable, str(ROOT / "bundle.py"), str(path), "--stdout"],
                                     text=True, capture_output=True)
            self.assertEqual(bundled.returncode, 0, bundled.stderr)
        self.assertIn("#include <boost/multiprecision/cpp_int.hpp>", bundled.stdout)
        self.compile_and_run(bundled.stdout, includes=False)


@unittest.skipUnless(HAS_BOOST, "Boost cpp_int が必要：整数演算の独立参照実装との比較は未実施")
class IntegerOracleTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.binaries = []
        try:
            for standard, local in (("c++20", True), ("gnu++23", False)):
                exe = Path(cls.tmp.name) / f"oracle-{standard}.exe"
                # LinuxのASanシャドウメモリとPIEの配置衝突を避ける。
                sanitize = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                            "-fno-omit-frame-pointer", "-no-pie"] if os.name != "nt" else []
                flags = ["-DLOCAL"] if local else []
                result = subprocess.run(
                    ["g++", f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Werror", *sanitize,
                     *flags, "-I", str(ROOT), str(ROOT / ".verify-helper/tests/fixtures/integer_arithmetic.cpp"),
                     "-o", str(exe)], text=True, capture_output=True,
                )
                if result.returncode:
                    raise RuntimeError(result.stderr)
                cls.binaries.append(exe)
        except BaseException:
            cls.tmp.cleanup()
            raise

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def run_case(self, mode, index=0):
        result = subprocess.run([str(self.binaries[index]), mode], text=True, capture_output=True, timeout=120)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_io_boundaries_and_random_roundtrip(self):
        self.run_case("io")

    def test_arithmetic_exhaustive_8bit_and_random_128bit(self):
        self.run_case("arithmetic")

    def test_sqrt_digits_boundaries_and_random(self):
        self.run_case("roots_digits")

    def test_recursive_debug_local(self):
        self.run_case("debug")

    def test_all_gnucxx23_without_local(self):
        self.run_case("all", 1)


if __name__ == "__main__":
    unittest.main()
