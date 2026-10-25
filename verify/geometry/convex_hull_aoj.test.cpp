#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_4_A"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>
#include "../../library/geometry/convex_hull.hpp"

int main() {
    using namespace cpl::geometry;
    int n;
    std::cin >> n;
    std::vector<Point<long long>> points(n);
    for (auto& p : points) std::cin >> p.x >> p.y;
    auto hull = convex_hull(points, true);
    // AOJ は y 最小（同値なら x 最小）の点を始点とし、辺上の点も列挙する。
    const auto first = std::min_element(hull.begin(), hull.end(), [](const auto& a, const auto& b) {
        return std::tie(a.y, a.x) < std::tie(b.y, b.x);
    });
    if (!hull.empty()) std::rotate(hull.begin(), first, hull.end());
    std::cout << hull.size() << '\n';
    for (const auto& p : hull) std::cout << p.x << ' ' << p.y << '\n';
}
