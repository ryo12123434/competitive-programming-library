#define PROBLEM "https://judge.yosupo.jp/problem/minimum_enclosing_circle"
#include "../../library/geometry/minimum_enclosing_circle.hpp"
#include <cassert>
#include <iostream>
using W = __int128_t;
int main() {
    int n;
    std::cin >> n;
    std::vector<std::pair<long long, long long>> a(n);
    std::vector<cpl::geometry::RealPoint> p;
    for (auto &[x, y] : a) {
        std::cin >> x >> y;
        p.push_back({static_cast<long double>(x), static_cast<long double>(y)});
    }
    auto r = cpl::geometry::minimum_enclosing_circle(p);
    auto ids = r->support;
    assert(ids.size() >= 2);
    auto [ax, ay] = a[ids[0]];
    auto [bx, by] = a[ids[1]];
    W dx = W(bx) - ax, dy = W(by) - ay;
    W denominator = 0, ux = 0, uy = 0;
    if (ids.size() == 3) {
        auto [cx, cy] = a[ids[2]];
        W ex = W(cx) - ax, ey = W(cy) - ay;
        denominator = 2 * (dx * ey - dy * ex);
        ux = (dx * dx + dy * dy) * ey - (ex * ex + ey * ey) * dy;
        uy = dx * (ex * ex + ey * ey) - ex * (dx * dx + dy * dy);
        assert(denominator != 0);
    }
    for (auto [x, y] : a) {
        W value;
        if (ids.size() == 2) {
            W ex = 2 * W(x) - ax - bx, ey = 2 * W(y) - ay - by;
            value = ex * ex + ey * ey - dx * dx - dy * dy;
        } else {
            W ex = W(x) - ax, ey = W(y) - ay;
            value = denominator * (ex * ex + ey * ey) - 2 * ux * ex - 2 * uy * ey;
            if (denominator < 0)
                value = -value;
        }
        assert(value <= 0);
        std::cout << (value == 0 ? '1' : '0');
    }
    std::cout << "\n";
}
