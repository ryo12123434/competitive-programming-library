#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_2_D"
#define ERROR 1e-8
#include <cassert>
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_linear.hpp"
int main() {
    using namespace cpl::geometry;
    int q;std::cin>>q;std::cout<<std::setprecision(20);
    while(q--) {
        RealSegment a,b;std::cin>>a.a.x>>a.a.y>>a.b.x>>a.b.y>>b.a.x>>b.a.y>>b.b.x>>b.b.y;
        auto d=distance(a,b);assert(d);std::cout<<*d<<'\n';
    }
}
