#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_7_F"
#define ERROR 1e-8
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_circle.hpp"
int main() {
    using namespace cpl::geometry;
    RealPoint p;RealCircle c;std::cin>>p.x>>p.y>>c.center.x>>c.center.y>>c.radius;
    auto hit=tangents_from(c,p);assert(hit.kind==IntersectionKind::Finite);
    std::vector<RealPoint> points;for(auto l:hit.lines)points.push_back(l.p);
    std::sort(points.begin(),points.end());std::cout<<std::setprecision(20);
    for(auto v:points)std::cout<<v.x<<' '<<v.y<<'\n';
}
