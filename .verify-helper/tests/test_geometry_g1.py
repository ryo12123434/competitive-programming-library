import itertools
import os
from pathlib import Path
import random
import subprocess
import tempfile
import unittest
from fractions import Fraction
import math

ROOT = Path(__file__).resolve().parents[2]
M = (1 << 62) - 1

def cross(a,b,c):
    return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])

def location(poly,p):
    hits=0
    for a,b in zip(poly,poly[1:]+poly[:1]):
        if cross(a,b,p)==0 and all(min(a[i],b[i])<=p[i]<=max(a[i],b[i]) for i in (0,1)):
            return 1
        if (a[1]>p[1]) != (b[1]>p[1]):
            x=a[0]+Fraction((p[1]-a[1])*(b[0]-a[0]),b[1]-a[1])
            hits+=x>p[0]
    return 2 if hits%2 else 0

def hull(points):
    points=sorted(set(points))
    if len(points)<2:return points
    # 全有向辺の支持半平面で頂点を見つける Jarvis march。
    result=[points[0]]
    while True:
        candidate=next(p for p in points if p!=result[-1])
        for p in points:
            turn=cross(result[-1],candidate,p)
            d=lambda q:sum((q[i]-result[-1][i])**2 for i in (0,1))
            if turn<0 or (turn==0 and d(p)>d(candidate)):candidate=p
        if candidate==result[0]:return result
        result.append(candidate)

class GeometryG1Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.binary=Path(cls.tmp.name)/'g1.exe'
        flags=['-fsanitize=undefined','-fno-sanitize-recover=all'] if os.name!='nt' else []
        p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,'-I',str(ROOT),
                          str(ROOT/'.verify-helper/tests/fixtures/geometry_g1.cpp'),'-o',str(cls.binary)],capture_output=True,text=True)
        if p.returncode:cls.tmp.cleanup();raise RuntimeError(p.stderr)
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def execute(self,cases):
        p=subprocess.run([str(self.binary)],input='\n'.join(cases)+'\n',capture_output=True,text=True,timeout=60)
        self.assertEqual(p.returncode,0,p.stderr)
        return p.stdout.splitlines()
    def encode(self,mode,points):return f'{mode} {len(points)} '+' '.join(f'{x} {y}' for x,y in points)
    def test_pairs_brute_force(self):
        grid=list(itertools.product(range(-1,2),repeat=2));rng=random.Random(20261026)
        sets=[[p for i,p in enumerate(grid) if mask>>i&1] for mask in range(512)]
        sets += [[(rng.randint(-10,10),rng.randint(-10,10)) for _ in range(rng.randrange(35))] for _ in range(700)]
        sets += [[(rng.choice([-M,0,M]),rng.choice([-M,0,M])) for _ in range(20)] for _ in range(100)]
        sets += [[(-(1<<63),0),(0,0)]]
        outputs=self.execute([self.encode('pair',p) for p in sets])
        for p,output in zip(sets,outputs):
            pairs=[(sum((a[k]-b[k])**2 for k in (0,1)),i,j) for i,a in enumerate(p) for j,b in enumerate(p) if i<j]
            if not pairs:self.assertEqual(output.split(),['none','none']);continue
            near=min(pairs);far=min(pairs,key=lambda v:(-v[0],v[1],v[2]))
            self.assertEqual(list(map(int,output.split())),[near[1],near[2],near[0],far[1],far[2],far[0]],p)
    def test_containment_and_convex(self):
        rng=random.Random(42);grid=list(itertools.product(range(-1,2),repeat=2));polys=[]
        for mask in range(512):
            p=hull([v for i,v in enumerate(grid) if mask>>i&1]);polys.extend([p,p[::-1]])
        polys += [[(0,0),(4,0),(4,4),(2,2),(0,4)],[(0,0),(2,0),(4,0)],[(0,0)]*3]
        for _ in range(100):
            xs=sorted(rng.sample(range(-10,11),5))
            polys.append([(x,rng.randint(1,8)) for x in xs]+[(x,-rng.randint(1,8)) for x in xs[::-1]])
        cases=[];expected=[]
        for p in polys:
            qlist=[(rng.randint(-5,5),rng.randint(-5,5)) for _ in range(5)]+p
            h=hull(p)
            for q in qlist:
                cases.append(self.encode('query',p)+f' {q[0]} {q[1]}')
                turns=[cross(p[i],p[(i+1)%len(p)],p[(i+2)%len(p)]) for i in range(len(p))]
                weak=bool(turns) and (any(t>0 for t in turns) and all(t>=0 for t in turns) or any(t<0 for t in turns) and all(t<=0 for t in turns))
                strict=weak and all(turns)
                expected.append([location(p,q),location(h,q),int(weak),int(strict)]+([location(p,q)] if len(p)==3 else []))
        for out,want in zip(self.execute(cases),expected):self.assertEqual(list(map(int,out.split())),want)
    def test_lattice_enumeration(self):
        rng=random.Random(7);polys=[[],[(0,0)],[(0,0),(2,0)],[(0,0)]*3]
        polys += [hull([(rng.randint(-5,5),rng.randint(-5,5)) for _ in range(10)]) for _ in range(400)]
        cases=[];expected=[]
        for p in polys:
            for q in [p,p[::-1],p+p[:1]]:
                area=abs(sum(a[0]*b[1]-a[1]*b[0] for a,b in zip(q,q[1:]+q[:1])))
                boundary_points=set()
                for a,b in zip(q,q[1:]+q[:1]):
                    steps=math.gcd(abs(a[0]-b[0]),abs(a[1]-b[1]))
                    if not steps:boundary_points.add(a);continue
                    for k in range(steps+1):
                        boundary_points.add(tuple(a[i]+(b[i]-a[i])*k//steps for i in (0,1)))
                boundary=len(boundary_points)
                interior=sum(location(q,(x,y))==2 for x in range(-5,6) for y in range(-5,6)) if area else None
                cases.append(self.encode('lattice',q));expected.append([str(boundary),str(interior) if interior is not None else 'none'])
        for out,want in zip(self.execute(cases),expected):self.assertEqual(out.split(),want)

    def test_large_predicates_and_pick(self):
        p=[(-M,-M),(M,-M),(M,M),(-M,M)]
        cases=[self.encode('query',p)+f' {x} {y}' for x,y in [(0,0),(M,0),(M+1,0)]]
        cases += [self.encode('lattice',p),self.encode('lattice',p[::-1]),
                  self.encode('lattice',[(-M,-M),(M,M)]),self.encode('lattice',[(M,M)])]
        output=self.execute(cases)
        self.assertEqual([o.split() for o in output[:3]],[['2','2','1','1'],['1','1','1','1'],['0','0','1','1']])
        self.assertEqual(output[3].split(),[str(8*M),str((2*M-1)**2)])
        self.assertEqual(output[4].split(),output[3].split())
        self.assertEqual(output[5].split(),[str(2*M+1),'none'])
        self.assertEqual(output[6].split(),['1','none'])

    def test_triangles_exhaustive_and_repeated_vertices(self):
        grid=list(itertools.product(range(-1,2),repeat=2));cases=[];expected=[]
        for triangle in itertools.product(grid,repeat=3):
            p=list(triangle);convex=int(cross(*p)!=0)
            for q in grid:
                loc=location(p,q)
                cases.append(self.encode('query',p)+f' {q[0]} {q[1]}')
                expected.append([loc,loc,convex,convex,loc])
        p=[(0,0),(2,0),(2,0),(2,2),(0,2),(0,0)]
        cases.append(self.encode('query',p)+' 1 1');expected.append([2,2,1,1])
        outputs=self.execute(cases)
        self.assertEqual(len(outputs),len(expected))
        for out,want in zip(outputs,expected):self.assertEqual(list(map(int,out.split())),want)

if __name__=='__main__':unittest.main()
