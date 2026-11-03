#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_7_G"
#define ERROR 1e-8
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_circle.hpp"
int main() {
    using namespace cpl::geometry;
    RealCircle a,b;std::cin>>a.center.x>>a.center.y>>a.radius>>b.center.x>>b.center.y>>b.radius;
    auto hit=common_tangents(a,b);assert(hit.kind==IntersectionKind::Finite);
    std::vector<RealPoint> points;for(auto l:hit.lines)points.push_back(l.p);
    std::sort(points.begin(),points.end());std::cout<<std::setprecision(20);
    for(auto p:points)std::cout<<p.x<<' '<<p.y<<'\n';
}
