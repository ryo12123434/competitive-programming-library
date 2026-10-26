from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
HEADERS=['point','ccw','polar_angle_sort','segment','convex_hull','polygon',
         'polygon_query','point_pair','lattice_polygon','real_point','real_linear','real_circle']
SOURCE=''.join(f'#include "library/geometry/{h}.hpp"\n' for h in HEADERS)+r'''
int main() {
    using namespace cpl::geometry;
    std::vector<Point<>> p{{0,0},{2,0},{2,2},{0,2}};
    ConvexPolygon<> convex(p);
    if(convex.contains({1,1})!=Location::Inside || !is_convex(p)) return 1;
    if(closest_pair(p)->distance2!=4 || farthest_pair(p)->distance2!=8) return 2;
    if(boundary_lattice_points(p)!=8 || interior_lattice_points(p)!=1) return 3;
    if(contains_triangle(p[0],p[1],p[2],Point<>{1,0})!=Location::Boundary) return 4;
    RealCircle c{{0,0},1};
    auto line=RealLine::through({-2,0},{2,0});
    if(intersections(line,c).points.size()!=2 || tangents_from(c,{2,0}).lines.size()!=2) return 5;
    if(common_tangents(c,RealCircle{{4,0},1}).lines.size()!=4) return 6;
    if(!projection(line,{0,1}) || !reflection(line,{0,1})) return 7;
    if(signed_area2(p)!=8 || !segments_intersect(Segment<>{p[0],p[2]},Segment<>{p[1],p[3]})) return 8;
    return 0;
}
'''

class GeometryIntegrationTest(unittest.TestCase):
    def compile_run(self,source,root,flags=()):
        binary=root/'integration.exe'
        p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,
                          '-x','c++','-','-o',str(binary)],input=source,text=True,capture_output=True,cwd=root)
        self.assertEqual(p.returncode,0,p.stderr)
        p=subprocess.run([str(binary)],text=True,capture_output=True,timeout=30)
        self.assertEqual(p.returncode,0,p.stderr)
    def test_release_and_bundle(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            self.compile_run(SOURCE,root,('-DNDEBUG','-I',str(ROOT)))
            path=root/'main.cpp';path.write_text(SOURCE,encoding='utf-8')
            p=subprocess.run([sys.executable,str(ROOT/'bundle.py'),str(path),'--stdout'],text=True,capture_output=True)
            self.assertEqual(p.returncode,0,p.stderr)
            self.assertNotRegex(p.stdout,r'(?m)^\s*#\s*include\s*"library/')
            self.compile_run(p.stdout,root)

if __name__=='__main__':unittest.main()
