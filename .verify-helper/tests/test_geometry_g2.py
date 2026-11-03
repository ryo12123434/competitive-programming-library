from decimal import Decimal, getcontext
from fractions import Fraction
from itertools import product
import math
import os
from pathlib import Path
import random
import subprocess
import tempfile
import unittest
from test_geometry import intersect, on_segment, orientation, subtract, determinant

getcontext().prec=80
ROOT=Path(__file__).resolve().parents[2]
def dec(x):
    return Decimal(x.numerator)/Decimal(x.denominator) if isinstance(x,Fraction) else Decimal(x)
def flat(values):return ' '.join(str(x) for v in values for x in v)
def d2(p,a,b):
    v=subtract(b,a);w=subtract(p,a);norm=sum(x*x for x in v)
    t=max(Fraction(0),min(Fraction(1),Fraction(sum(v[i]*w[i] for i in (0,1)),norm))) if norm else 0
    return sum((p[i]-a[i]-t*v[i])**2 for i in (0,1))

class GeometryG2Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.binary=Path(cls.tmp.name)/'g2.exe'
        flags=['-fsanitize=undefined','-fno-sanitize-recover=all'] if os.name!='nt' else []
        p=subprocess.run(['g++','-std=gnu++23','-O2','-Wall','-Wextra','-Werror',*flags,'-I',str(ROOT),
                          str(ROOT/'.verify-helper/tests/fixtures/geometry_g2.cpp'),'-o',str(cls.binary)],capture_output=True,text=True)
        if p.returncode:cls.tmp.cleanup();raise RuntimeError(p.stderr)
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def execute(self,cases):
        p=subprocess.run([str(self.binary)],input='\n'.join(cases)+'\n',capture_output=True,text=True,timeout=60)
        self.assertEqual(p.returncode,0,p.stderr)
        return [line.split() for line in p.stdout.splitlines()]
    def close(self,a,b):
        a,b=dec(a),dec(b)
        self.assertLessEqual(abs(a-b),Decimal('2e-15')*max(Decimal(1),abs(b)),(a,b))
    def test_linear_exhaustive(self):
        grid=list(product(range(-1,2),repeat=2));pairs=list(product(grid,repeat=2))
        cases=[];expect=[]
        for (a,b),(c,d) in product(pairs,repeat=2):
            cases.append('ss '+flat([a,b,c,d]))
            if not intersect(a,b,c,d):expect.append((0,0,None));continue
            shared=set(p for p in [a,b,c,d] if on_segment(p,a,b) and on_segment(p,c,d))
            if len(shared)>1:expect.append((1,0,None));continue
            if shared:expect.append((0,1,next(iter(shared))));continue
            u,v,w=subtract(b,a),subtract(d,c),subtract(c,a)
            t=Fraction(determinant(w,v),determinant(u,v))
            expect.append((0,1,(a[0]+t*u[0],a[1]+t*u[1])))
        for out,(kind,count,p) in zip(self.execute(cases),expect):
            self.assertEqual(list(map(int,out[:2])),[kind,count])
            if p is not None:self.close(out[2],p[0]);self.close(out[3],p[1])
    def test_lines_projection_distances(self):
        rng=random.Random(61);cases=[];expected=[]
        for _ in range(1000):
            a,b,c,d=[(rng.randint(-20,20),rng.randint(-20,20)) for _ in range(4)]
            if a==b:continue
            v=subtract(b,a);w=subtract(c,a);norm=sum(x*x for x in v)
            t=Fraction(sum(v[i]*w[i] for i in (0,1)),norm)
            foot=[a[i]+t*v[i] for i in (0,1)]
            reflected=[2*foot[i]-c[i] for i in (0,1)]
            line_dist=dec(Fraction(determinant(v,w)**2,norm)).sqrt()
            cases.append('metrics '+flat([c,a,b]));expected.append([*foot,*reflected,line_dist,dec(d2(c,a,b)).sqrt()])
            cases.append('distance '+flat([a,b,c,d]))
            minimum=0 if intersect(a,b,c,d) else min(d2(a,c,d),d2(b,c,d),d2(c,a,b),d2(d,a,b))
            expected.append([dec(minimum).sqrt()])
            if c!=d:
                denom=determinant(v,subtract(d,c))
                cases.append('ll '+flat([a,b,c,d]))
                if denom:
                    t=Fraction(determinant(w,subtract(d,c)),denom)
                    expected.append(['hit',a[0]+t*v[0],a[1]+t*v[1]])
                else:expected.append(['infinite' if determinant(v,w)==0 else 'empty'])
            cases.append('ls '+flat([a,b,c,d]))
            signs=[orientation(a,b,p) for p in [c,d]]
            if signs==[0,0]:expected.append(['hit',c[0],c[1]] if c==d else ['infinite'])
            elif signs[0]*signs[1]>0:expected.append(['empty'])
            else:
                t=Fraction(signs[0],signs[0]-signs[1])
                expected.append(['hit',c[0]+t*(d[0]-c[0]),c[1]+t*(d[1]-c[1])])
        for out,want in zip(self.execute(cases),expected):
            if want[0]=='hit':
                self.assertEqual(out[:2],['0','1']);out=out[2:];want=want[1:]
            elif want[0]=='empty':self.assertEqual(out,['0','0']);continue
            elif want[0]=='infinite':self.assertEqual(out,['1','0']);continue
            for x,y in zip(out,want):self.close(x,y)
    def test_circle_intersections_decimal(self):
        rng=random.Random(31);cases=[];circles=[]
        for _ in range(1500):
            a,b=[(rng.randint(-10,10),rng.randint(-10,10),rng.randint(0,8)) for _ in range(2)]
            cases.append('cc '+flat([a,b]));circles.append((a,b))
        for out,(a,b) in zip(self.execute(cases),circles):
            d=(a[0]-b[0])**2+(a[1]-b[1])**2
            if not d and a[2]==b[2]:
                expected=(0,1) if a[2]==0 else (1,0)
            elif not d or d>(a[2]+b[2])**2 or d<(a[2]-b[2])**2:expected=(0,0)
            elif d in [(a[2]+b[2])**2,(a[2]-b[2])**2]:expected=(0,1)
            else:expected=(0,2)
            self.assertEqual(tuple(map(int,out[:2])),expected,(a,b,out))
            for x,y in zip(out[2::2],out[3::2]):
                for c in [a,b]:self.close((dec(x)-c[0])**2+(dec(y)-c[1])**2,c[2]**2)
            if expected==(0,2):self.assertNotEqual(out[2:4],out[4:6])
    def test_circle_line_and_tangents(self):
        rng=random.Random(91);cases=[];metadata=[]
        for _ in range(1000):
            c=(rng.randint(-10,10),rng.randint(-10,10),rng.randint(0,8))
            p=(rng.randint(-15,15),rng.randint(-15,15));q=(rng.randint(-15,15),rng.randint(-15,15))
            if p!=q:
                cases.append('lc '+flat([p,q,c]));metadata.append(('lc',c,p,q))
            cases.append('pt '+flat([c,p]));metadata.append(('pt',c,p,None))
            b=(rng.randint(-10,10),rng.randint(-10,10),rng.randint(0,8))
            cases.append('ct '+flat([c,b]));metadata.append(('ct',c,b,None))
        for out,(mode,c,p,q) in zip(self.execute(cases),metadata):
            kind,count=map(int,out[:2])
            if mode=='lc':
                v=subtract(q,p);w=subtract(c[:2],p);diff=determinant(v,w)**2-c[2]**2*sum(x*x for x in v)
                self.assertEqual((kind,count),(0,0 if diff>0 else 1 if diff==0 else 2))
                for x,y in zip(out[2::2],out[3::2]):
                    self.close((dec(x)-c[0])**2+(dec(y)-c[1])**2,c[2]**2)
                    self.close(v[0]*(dec(y)-p[1])-v[1]*(dec(x)-p[0]),0)
                continue
            d=sum((c[i]-p[i])**2 for i in (0,1))
            if mode=='pt':
                expected=(1,0) if c[2]==0 and d==0 else (0,1 if c[2]==0 or d==c[2]**2 else 0 if d<c[2]**2 else 2)
            elif d==0:expected=(1,0) if c[2]==p[2] else (0,0)
            elif c[2]==p[2]==0:expected=(0,1)
            elif c[2]==0 or p[2]==0:
                r=c[2]+p[2];expected=(0,0 if d<r*r else 1 if d==r*r else 2)
            else:expected=(0,sum(0 if d<r*r else 1 if d==r*r else 2 for r in [c[2]-p[2],c[2]+p[2]]))
            self.assertEqual((kind,count),expected,(mode,c,p,out))
            for i in range(count):
                x,y,dx,dy=map(Decimal,out[2+4*i:6+4*i]);norm=dx*dx+dy*dy
                self.assertGreater(norm,0)
                self.close(((c[0]-x)*dy-(c[1]-y)*dx)**2/norm,c[2]**2)
                self.close((x-c[0])**2+(y-c[1])**2,c[2]**2)
                if mode=='pt':self.close((p[0]-x)*dy-(p[1]-y)*dx,0)
                else:self.close(((p[0]-x)*dy-(p[1]-y)*dx)**2/norm,p[2]**2)
    def test_near_degenerate_regressions(self):
        self.execute([]) # C++ 側で nextafter 相当の機械 epsilon、極小・極大スケールも検査する。

if __name__=='__main__':unittest.main()
