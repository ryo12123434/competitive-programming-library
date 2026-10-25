#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_1_C"

#include <iostream>
#include "../../library/geometry/segment.hpp"

int main() {
    using namespace cpl::geometry;
    using P = Point<long long>;
    P a, b;
    int q;
    std::cin >> a.x >> a.y >> b.x >> b.y >> q;
    while (q--) {
        P p;
        std::cin >> p.x >> p.y;
        const int turn = ccw(a, b, p);
        if (turn > 0) std::cout << "COUNTER_CLOCKWISE\n";
        else if (turn < 0) std::cout << "CLOCKWISE\n";
        else if (on_segment(p, Segment<long long>{a, b})) std::cout << "ON_SEGMENT\n";
        // この問題の座標差は long long に収まる。ccw の0を問題固有の前後関係に分ける。
        else if (dot(p - a, b - a) < 0) std::cout << "ONLINE_BACK\n";
        else std::cout << "ONLINE_FRONT\n";
    }
}
