#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_7_E"
#define ERROR 1e-8
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_circle.hpp"
int main() {
    using namespace cpl::geometry;
    RealCircle a,b;std::cin>>a.center.x>>a.center.y>>a.radius>>b.center.x>>b.center.y>>b.radius;
    auto hit=intersections(a,b);assert(hit.kind==IntersectionKind::Finite && !hit.points.empty());
    if(hit.points.size()==1)hit.points.push_back(hit.points[0]);
    std::sort(hit.points.begin(),hit.points.end());std::cout<<std::setprecision(20);
    for(auto p:hit.points)std::cout<<p.x<<' '<<p.y<<' ';
    std::cout<<'\n';
}
