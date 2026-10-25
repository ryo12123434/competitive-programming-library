#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 退化ケース・数値上限・狭義弱順序・小規模全探索を外部入力に関係なく実行する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>
#include "../../library/geometry/convex_hull.hpp"
#include "../../library/geometry/polar_angle_sort.hpp"
#include "../../library/geometry/polygon.hpp"
#include "../../library/geometry/segment.hpp"

using namespace cpl::geometry;
using P = Point<long long>;

void numeric_cases() {
    constexpr Point<int> a{3, 4}, b{-2, 1};
    static_assert(dot(a, b) == -2 && cross(a, b) == 11 && norm2(a) == 25 && dist2(a, b) == 34);
    static_assert((a + b == Point<int>{1, 5}) && (a - b == Point<int>{5, 3}));
    static_assert(std::is_same_v<decltype(dot(a, b)), Wide>);
    Point<short> small{1, -2};
    small += Point<short>{3, 4}; small -= Point<short>{1, 1};
    assert((small == Point<short>{3, 1}));
    const long long m = (1LL << 62) - 1;
    const Wide diagonal = Wide(8) * m * m;
    assert(dist2(P{-m,-m}, P{m,m}) == diagonal);
    assert(cross(P{-m,-m}, P{m,-m}, P{-m,m}) == Wide(4) * m * m);
    std::vector<P> square{{-m,-m}, {m,-m}, {m,m}, {-m,m}};
    assert(signed_area2(square) == diagonal && polygon_orientation(square) == 1);
    std::reverse(square.begin(), square.end());
    assert(signed_area2(square) == -diagonal && polygon_orientation(square) == -1);
    const auto lo = std::numeric_limits<long long>::min(), hi = std::numeric_limits<long long>::max();
    // 一律の保証範囲外でも、中間値が収まる例。64bitの差を先に計算するとあふれる。
    assert(cross(P{lo,0}, P{hi,0}, P{lo,1}) == Wide(hi) - lo);
    assert(norm2(P{lo,0}) == Wide(lo) * lo);
    assert(dist2(P{lo,0}, P{1,0}) == (Wide(1) - lo) * (Wide(1) - lo));
    assert(ccw(P{lo,0}, P{hi,0}, P{0,1}) == 1);
    assert(on_segment(P{0,0}, Segment<long long>{{lo,0}, {hi,0}}));
    assert(segments_intersect(Segment<long long>{{lo,0}, {hi,0}}, Segment<long long>{{0,0}, {0,0}}));
    assert(signed_area2(std::vector<P>{}) == 0 && polygon_orientation(std::vector<P>{{1,2}}) == 0);
    assert(signed_area2(std::vector<P>{{1,2}, {3,4}}) == 0);
}

void check_order() {
    std::vector<P> points;
    for (long long x = -2; x <= 2; ++x) for (long long y = -2; y <= 2; ++y) points.push_back({x,y});
    points.push_back({0,0}); points.push_back({1,0});
    const long long m = (1LL << 62) - 1;
    for (P p : std::vector<P>{{m,m}, {-m,m}, {-m,-m}, {m,-m}, {m,0}, {0,-m}}) points.push_back(p);
    auto less = polar_angle_less<long long>;
    for (auto a : points) {
        assert(!less(a,a));
        for (auto b : points) {
            assert(!(less(a,b) && less(b,a)));
            for (auto c : points) {
                if (less(a,b) && less(b,c)) assert(less(a,c));
                if (!less(a,b) && !less(b,a) && !less(b,c) && !less(c,b)) assert(!less(a,c) && !less(c,a));
            }
        }
    }
    polar_angle_sort(points);
    assert((points.front() == P{0,0}));
    assert(std::is_sorted(points.begin(), points.end(), less));
}

void check_hull(const std::vector<P>& input) {
    auto unique = input;
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    for (bool keep : {false, true}) {
        auto h = convex_hull(input, keep);
        if (unique.empty()) { assert(h.empty()); continue; }
        assert(!h.empty() && h.front() == unique.front());
        auto sorted = h;
        std::sort(sorted.begin(), sorted.end());
        assert(std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end());
        for (auto p : h) assert(std::binary_search(unique.begin(), unique.end(), p));
        if (h.size() <= 2 || signed_area2(h) == 0) {
            if (keep) assert(h == unique);
            else assert(h.size() == std::min<std::size_t>(unique.size(), 2));
            continue;
        }
        assert(polygon_orientation(h) == 1);
        for (std::size_t i = 0; i < h.size(); ++i) {
            auto a = h[i], b = h[(i+1) % h.size()], c = h[(i+2) % h.size()];
            assert(keep ? ccw(a,b,c) >= 0 : ccw(a,b,c) > 0);
            for (auto p : unique) {
                assert(ccw(a,b,p) >= 0);
                if (keep && on_segment(p, Segment<long long>{a,b})) assert(std::binary_search(sorted.begin(), sorted.end(), p));
            }
        }
    }
}

int main() {
    numeric_cases();
    check_order();
    for (unsigned mask = 0; mask < (1U << 9); ++mask) {
        std::vector<P> points;
        for (int i = 0; i < 9; ++i) if (mask >> i & 1U) points.push_back({i / 3 - 1, i % 3 - 1});
        check_hull(points);
    }
    std::mt19937_64 rng(20261025);
    for (int trial = 0; trial < 1000; ++trial) {
        std::vector<P> points(rng() % 21);
        for (auto& p : points) p = {static_cast<long long>(rng() % 13) - 6, static_cast<long long>(rng() % 13) - 6};
        check_hull(points);
        auto h = convex_hull(points, true);
        std::shuffle(points.begin(), points.end(), rng);
        assert(convex_hull(points, true) == h);
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
