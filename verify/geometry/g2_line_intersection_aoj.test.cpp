#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_2_C"
#define ERROR 1e-8
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_linear.hpp"
int main() {
    using namespace cpl::geometry;
    int q;std::cin>>q;std::cout<<std::setprecision(20);
    while(q--) {
        RealPoint a,b,c,d;std::cin>>a.x>>a.y>>b.x>>b.y>>c.x>>c.y>>d.x>>d.y;
        auto hit=intersections(RealLine::through(a,b),RealLine::through(c,d));
        assert(hit.kind==IntersectionKind::Finite && hit.points.size()==1);
        auto p=hit.points[0];std::cout<<p.x<<' '<<p.y<<'\n';
    }
}
