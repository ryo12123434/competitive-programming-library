#define PROBLEM "https://judge.yosupo.jp/problem/closest_pair"
#include <iostream>
#include "../../library/geometry/point_pair.hpp"
int main() {
    using namespace cpl::geometry;
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int t;std::cin>>t;
    while(t--) {
        int n;std::cin>>n;std::vector<Point<>> p(n);
        for(auto& v:p)std::cin>>v.x>>v.y;
        auto pair=closest_pair(p);
        std::cout<<pair->first<<' '<<pair->second<<'\n';
    }
}
