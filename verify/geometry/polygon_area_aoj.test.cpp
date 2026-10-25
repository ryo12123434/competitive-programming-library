#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/CGL_3_A"

#include <iostream>
#include <vector>
#include "../../library/geometry/polygon.hpp"

int main() {
    using namespace cpl::geometry;
    int n;
    std::cin >> n;
    std::vector<Point<long long>> polygon(n);
    for (auto& p : polygon) std::cin >> p.x >> p.y;
    // この問題の面積の2倍は long long に収まる。整数のまま .0 / .5 を出力する。
    auto area = signed_area2(polygon);
    if (area < 0) area = -area;
    const auto twice = static_cast<long long>(area);
    std::cout << twice / 2 << (twice % 2 ? ".5\n" : ".0\n");
}
