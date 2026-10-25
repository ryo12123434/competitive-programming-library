from fractions import Fraction
from itertools import product
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile
import unittest


REPOSITORY = Path(__file__).resolve().parents[2]
LIMIT = (1 << 62) - 1


def determinant(a, b):
    return a[0] * b[1] - a[1] * b[0]


def subtract(a, b):
    return a[0] - b[0], a[1] - b[1]


def orientation(a, b, c):
    return determinant(subtract(b, a), subtract(c, a))


def on_segment(p, a, b):
    u, v = subtract(p, a), subtract(p, b)
    return determinant(u, subtract(b, a)) == 0 and u[0] * v[0] + u[1] * v[1] <= 0


def intersect(a, b, c, d):
    if a == b:
        return on_segment(a, c, d)
    if c == d:
        return on_segment(c, a, b)
    u, v, w = subtract(b, a), subtract(d, c), subtract(c, a)
    denominator = determinant(u, v)
    if denominator:
        t = Fraction(determinant(w, v), denominator)
        s = Fraction(determinant(w, u), denominator)
        return 0 <= t <= 1 and 0 <= s <= 1
    if determinant(u, w):
        return False
    axis = 0 if u[0] else 1
    left, right = sorted((a[axis], b[axis]))
    other_left, other_right = sorted((c[axis], d[axis]))
    return max(left, other_left) <= min(right, other_right)


def norm(p):
    return p[0] ** 2 + p[1] ** 2


def polar_key(p):
    # 半平面と外積による実装とは別に、4象限の有理数の傾きで厳密に比較する。
    x, y = p
    if x == y == 0:
        return -1, Fraction(0), 0
    if x > 0 and y >= 0:
        quadrant, slope = 0, Fraction(y, x)
    elif y > 0:
        quadrant, slope = 1, Fraction(-x, y)
    elif x < 0:
        quadrant, slope = 2, Fraction(y, x)
    else:
        quadrant, slope = 3, Fraction(x, -y)
    return quadrant, slope, norm(p)


def hull_reference(points, keep=False):
    # Gift wrapping と各辺上の点の列挙。Andrew 法と独立に構築する。
    points = sorted(set(points))
    if len(points) <= 2:
        return points
    hull, current = [], points[0]
    while True:
        hull.append(current)
        following = next(p for p in points if p != current)
        for p in points:
            turn = orientation(current, following, p)
            if turn < 0 or (turn == 0 and norm(subtract(p, current)) > norm(subtract(following, current))):
                following = p
        current = following
        if current == hull[0]:
            break
    if not keep:
        return hull
    if len(hull) == 2:
        return points
    boundary = []
    for a, b in zip(hull, hull[1:] + hull[:1]):
        edge = [p for p in points if p != b and on_segment(p, a, b)]
        boundary.extend(sorted(edge, key=lambda p: norm(subtract(p, a))))
    return boundary


def encode(points):
    return " ".join(str(value) for p in points for value in p)


def vector_output(points):
    return f"{len(points)} " + encode(points)


class GeometryTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.directory.cleanup)
        cls.root = Path(cls.directory.name)
        cls.oracle = cls.root / "oracle.exe"
        cls.flags = [] if os.name == "nt" else ["-fsanitize=undefined", "-fno-sanitize-recover=all"]
        cls.compile(REPOSITORY / ".verify-helper/tests/fixtures/geometry_oracle.cpp", cls.oracle)

    @classmethod
    def compile(cls, source, output, include=True):
        result = subprocess.run(
            ["g++", "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-Werror", *cls.flags,
             *(["-I", str(REPOSITORY)] if include else []), str(source), "-o", str(output)],
            text=True, capture_output=True, cwd=cls.root,
        )
        if result.returncode:
            raise AssertionError(result.stderr)

    def check_cases(self, commands, expected):
        result = subprocess.run([str(self.oracle)], input="\n".join(commands) + "\n",
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        actual = [line.split() for line in result.stdout.splitlines()]
        self.assertEqual(actual, [line.split() for line in expected])

    def test_numeric_against_big_integers(self):
        rng = random.Random(1729)
        triples = [((0,0), (1,0), (0,1)), ((-LIMIT,-LIMIT), (LIMIT,LIMIT), (LIMIT,-LIMIT)),
                   ((-(1 << 63),0), (-(1 << 63)+1,0), ((1 << 63)-1,1)),
                   ((-(1 << 63),0), (1,0), (0,1)),
                   (((1 << 63)-1,0), (0,0), (-(1 << 63),1))]
        triples += [tuple((rng.randint(-LIMIT, LIMIT), rng.randint(-LIMIT, LIMIT)) for _ in range(3)) for _ in range(1000)]
        commands, expected = [], []
        for a, b, c in triples:
            area = orientation(a,b,c)
            values = [a[0]*b[0] + a[1]*b[1], determinant(a,b), area, norm(a), norm(subtract(a,b)), (area > 0) - (area < 0)]
            self.assertTrue(all(-(1 << 127) <= value < (1 << 127) for value in values))
            commands.append("point " + encode((a,b,c)))
            expected.append(" ".join(map(str, values)))
        self.check_cases(commands, expected)

    def test_segments_exhaustive_and_random(self):
        grid = list(product(range(-1,2), repeat=2))
        cases = list(product(grid, repeat=4))
        rng = random.Random(42)
        cases += [tuple((rng.randint(-LIMIT,LIMIT), rng.randint(-LIMIT,LIMIT)) for _ in range(4)) for _ in range(500)]
        commands, expected = [], []
        for a,b,c,d in cases:
            commands.append("segment " + encode((a,b,c,d)))
            expected.append(f"{int(on_segment(c,a,b))} {int(intersect(a,b,c,d))}")
        self.check_cases(commands, expected)

    def test_polar_order_against_fraction_slopes(self):
        rng = random.Random(861)
        cases = [[], [(0,0)] * 4, list(product(range(-2,3), repeat=2)) * 2,
                 [(1,0), (-1,0), (0,-1), (0,1), (LIMIT,0), (0,-LIMIT), (0,0), (2,0)]]
        for _ in range(500):
            points = [(rng.randint(-20,20), rng.randint(-20,20)) for _ in range(rng.randrange(51))]
            points += [(LIMIT,LIMIT), (-LIMIT,-LIMIT), (0,0)]
            rng.shuffle(points)
            cases.append(points)
        self.check_cases([f"polar {len(p)} " + encode(p) for p in cases],
                         [vector_output(sorted(p, key=polar_key)) for p in cases])

    def test_hull_exhaustive_and_gift_wrapping(self):
        grid = list(product(range(-1,2), repeat=2))
        cases = [[p for i,p in enumerate(grid) if mask >> i & 1] for mask in range(1 << len(grid))]
        cases += [[], [(1,2)] * 5, [(i,2*i) for i in range(-4,5)] * 2,
                  [(-LIMIT,-LIMIT), (LIMIT,-LIMIT), (LIMIT,LIMIT), (-LIMIT,LIMIT), (0,0)]]
        rng = random.Random(2510)
        cases += [[(rng.randint(-8,8), rng.randint(-8,8)) for _ in range(rng.randrange(25))] for _ in range(500)]
        commands, expected = [], []
        for points in cases:
            for keep in (False, True):
                commands.append(f"hull {int(keep)} {len(points)} " + encode(points))
                expected.append(vector_output(hull_reference(points, keep)))
        self.check_cases(commands, expected)

    def test_polygon_area_against_triangle_fan(self):
        rng = random.Random(65537)
        cases = [[], [(1,2)], [(1,2),(3,4)], [(0,0),(3,0),(0,1)],
                 [(-LIMIT,-LIMIT), (LIMIT,-LIMIT), (LIMIT,LIMIT), (-LIMIT,LIMIT)],
                 [(0,0),(1,1),(2,2),(0,0)]]
        for _ in range(500):
            polygon = hull_reference([(rng.randint(-10**12,10**12), rng.randint(-10**12,10**12)) for _ in range(20)])
            if rng.randrange(2):
                polygon.reverse()
            shift = rng.randrange(len(polygon))
            polygon = polygon[shift:] + polygon[:shift]
            if rng.randrange(2):
                polygon.append(polygon[0])
            cases.append(polygon)
        expected = []
        for p in cases:
            area = sum(orientation(p[0],p[i],p[i+1]) for i in range(1,len(p)-1))
            expected.append(f"{area} {(area > 0) - (area < 0)}")
        self.check_cases([f"polygon {len(p)} " + encode(p) for p in cases], expected)

    def test_regression_and_standalone_bundle(self):
        source = REPOSITORY / "verify/geometry/regression_library_checker.test.cpp"
        executable = self.root / "regression.exe"
        self.compile(source, executable)
        result = subprocess.run([str(executable)], input="3 4\n", text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "7\n")
        result = subprocess.run([sys.executable, str(REPOSITORY / "bundle.py"), str(source), "--stdout"],
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(any(line.lstrip().startswith('#include "') for line in result.stdout.splitlines()))
        bundled = self.root / "bundled.cpp"
        bundled.write_text(result.stdout, encoding="utf-8")
        self.compile(bundled, executable, include=False)
        result = subprocess.run([str(executable)], input="3 4\n", text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "7\n")

    def test_judge_adapters(self):
        cases = [
            ("ccw_aoj.test.cpp", "0 0 2 0\n5\n0 1\n0 -1\n-1 0\n3 0\n1 0\n",
             "COUNTER_CLOCKWISE\nCLOCKWISE\nONLINE_BACK\nONLINE_FRONT\nON_SEGMENT\n"),
            ("segment_aoj.test.cpp", "4\n0 0 2 2 0 2 2 0\n0 0 1 0 1 0 2 0\n0 0 0 0 0 0 0 0\n0 0 0 0 1 1 2 2\n", "1\n1\n1\n0\n"),
            ("convex_hull_aoj.test.cpp", "7\n0 0\n1 0\n2 0\n2 2\n0 2\n1 1\n0 0\n", "5\n0 0\n1 0\n2 0\n2 2\n0 2\n"),
            ("polygon_area_aoj.test.cpp", "3\n0 0\n0 1\n3 0\n", "1.5\n"),
            ("polar_angle_sort_library_checker.test.cpp", "7\n1 0\n0 1\n-1 0\n0 -1\n0 0\n2 0\n1 -1\n",
             "0 -1\n1 -1\n0 0\n1 0\n2 0\n0 1\n-1 0\n"),
        ]
        for filename, data, expected in cases:
            with self.subTest(filename=filename):
                executable = self.root / "judge.exe"
                self.compile(REPOSITORY / "verify/geometry" / filename, executable)
                result = subprocess.run([str(executable)], input=data, text=True, capture_output=True, timeout=10)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)


if __name__ == "__main__":
    unittest.main()
