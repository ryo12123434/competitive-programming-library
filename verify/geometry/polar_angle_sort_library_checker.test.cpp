#define PROBLEM "https://judge.yosupo.jp/problem/sort_points_by_argument"

#include <algorithm>
#include <iostream>
#include <vector>
#include "../../library/geometry/polar_angle_sort.hpp"

int main() {
    using namespace cpl::geometry;
    int n;
    std::cin >> n;
    std::vector<Point<long long>> points(n);
    for (auto& p : points) std::cin >> p.x >> p.y;
    polar_angle_sort(points);
    // Library Checker は負角からの順。ライブラリの [0,2pi) 順を回転して合わせる。
    // 同方向の順序には複数解があるため、verify-helper が取得する専用 checker で判定する。
    auto first = std::find_if(points.begin(), points.end(), [](const auto& p) { return p.y < 0; });
    std::rotate(points.begin(), first, points.end());
    for (const auto& p : points) std::cout << p.x << ' ' << p.y << '\n';
}
