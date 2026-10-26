#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_1_B"
#define ERROR 1e-8
#include <iostream>
#include <iomanip>
#include "../../library/geometry/real_linear.hpp"
int main() {
    using namespace cpl::geometry;
    RealPoint a,b;int q;std::cin>>a.x>>a.y>>b.x>>b.y>>q;
    auto line=RealLine::through(a,b);std::cout<<std::setprecision(20);
    while(q--) {RealPoint p;std::cin>>p.x>>p.y;auto x=*reflection(line,p);std::cout<<x.x<<' '<<x.y<<'\n';}
}
