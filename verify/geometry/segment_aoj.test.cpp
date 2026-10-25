#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_2_B"

#include <iostream>
#include "../../library/geometry/segment.hpp"

int main() {
    using namespace cpl::geometry;
    int q;
    std::cin >> q;
    while (q--) {
        Segment<long long> s, t;
        std::cin >> s.a.x >> s.a.y >> s.b.x >> s.b.y >> t.a.x >> t.a.y >> t.b.x >> t.b.y;
        std::cout << segments_intersect(s, t) << '\n';
    }
}
