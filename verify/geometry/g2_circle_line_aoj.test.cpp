#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_7_D"
#define ERROR 1e-8
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_circle.hpp"
int main() {
    using namespace cpl::geometry;
    RealCircle c;int q;std::cin>>c.center.x>>c.center.y>>c.radius>>q;
    std::cout<<std::setprecision(20);
    while(q--) {
        RealPoint a,b;std::cin>>a.x>>a.y>>b.x>>b.y;
        auto hit=intersections(RealLine::through(a,b),c);
        assert(hit.kind==IntersectionKind::Finite && !hit.points.empty());
        // この問題は接点も同じ点を2回出力する形式。
        if(hit.points.size()==1)hit.points.push_back(hit.points[0]);
        std::sort(hit.points.begin(),hit.points.end());
        for(auto p:hit.points)std::cout<<p.x<<' '<<p.y<<' ';
        std::cout<<'\n';
    }
}
