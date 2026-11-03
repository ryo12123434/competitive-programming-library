#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_3_B"
#include <iostream>
#include "../../library/geometry/polygon_query.hpp"
int main() {
    using namespace cpl::geometry;
    int n;std::cin>>n;std::vector<Point<>> p(n);
    for(auto& v:p)std::cin>>v.x>>v.y;
    // AOJ は面積0の全共線も凸として扱う。ライブラリでは面積0を false とするため補う。
    std::cout<<(is_convex(p) || convex_hull(p).size()<3)<<'\n';
}
