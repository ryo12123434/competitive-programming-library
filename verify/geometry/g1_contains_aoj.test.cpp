#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_3_C"
#include <iostream>
#include "../../library/geometry/polygon_query.hpp"
int main() {
    using namespace cpl::geometry;
    int n,q;std::cin>>n;std::vector<Point<>> p(n);
    for(auto& v:p)std::cin>>v.x>>v.y;
    std::cin>>q;
    while(q--) {Point<> v;std::cin>>v.x>>v.y;std::cout<<int(contains_polygon(p,v))<<'\n';}
}
