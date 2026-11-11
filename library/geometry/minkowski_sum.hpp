#pragma once
#include "point.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cpl::geometry {
namespace minkowski_detail {
template <class T> std::vector<Point<T>> normalize(std::vector<Point<T>> points) {
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() > 1 && points.front() == points.back())
        points.pop_back();
    if (points.size() <= 1)
        return points;
    Wide turn = 0;
    for (std::size_t i = 0; i < points.size(); ++i) {
        turn = cross(points[i], points[(i + 1) % points.size()], points[(i + 2) % points.size()]);
        if (turn)
            break;
    }
    if (!turn) {
        auto [a, b] = std::minmax_element(points.begin(), points.end());
        if (*a == *b)
            return {*a};
        return {*a, *b};
    }
    if (turn < 0)
        std::reverse(points.begin(), points.end());
    std::vector<Point<T>> clean;
    for (auto p : points) {
        while (clean.size() >= 2 && cross(clean[clean.size() - 2], clean.back(), p) == 0)
            clean.pop_back();
        clean.push_back(p);
    }
    std::size_t begin = 0, end = clean.size();
    while (end - begin > 2 && cross(clean[end - 1], clean[begin], clean[begin + 1]) == 0)
        ++begin;
    while (end - begin > 2 && cross(clean[end - 2], clean[end - 1], clean[begin]) == 0)
        --end;
    return {clean.begin() + begin, clean.begin() + end};
}
template <class T> Point<T> point(Wide x, Wide y) {
    if (x < std::numeric_limits<T>::lowest() || x > std::numeric_limits<T>::max() ||
        y < std::numeric_limits<T>::lowest() || y > std::numeric_limits<T>::max())
        throw std::overflow_error("Minkowski sum: coordinate type too narrow");
    return {static_cast<T>(x), static_cast<T>(y)};
}
} // namespace minkowski_detail
/**
 * @brief 凸多角形のMinkowski和を辺方向のマージで求める。空集合との和は空。
 * @param a,b 境界順の凸多角形。時計回りも可、隣接重複・始点再掲・辺上の共線点を許す。
 * 全点共線なら順序にかかわらず両端へ縮約する。1点・線分への退化も扱う。
 * @return 辞書順最小点から反時計回りの狭義凸包。始点再掲なし、線分は辞書順の両端。
 * @pre 非退化入力は凸境界順で、同じ境界を何度も周回しないこと。
 * 各座標の絶対値<=2^60-1は128bit中間計算の十分な保証範囲。出力はTに収める。
 * @throws std::overflow_error 出力座標がTに収まらない場合。加減算は変換前に128bitで行う。
 * @par Complexity O(N+M)時間・入力コピー/出力空間。ソートは行わない。
 * @code
 * #include "library/geometry/minkowski_sum.hpp"
 * #include <cassert>
 * int main(){using P=cpl::geometry::Point<long long>;auto p=cpl::geometry::minkowski_sum(std::vector<P>{{0,0},{1,0}},std::vector<P>{{0,0},{0,1}});assert(p.size()==4);}
 * @endcode
 */
template <class T> std::vector<Point<T>> minkowski_sum(std::vector<Point<T>> a, std::vector<Point<T>> b) {
    using namespace minkowski_detail;
    a = normalize(std::move(a));
    b = normalize(std::move(b));
    if (a.empty() || b.empty())
        return {};
    auto rotate = [](auto &p) {
        auto it = std::min_element(p.begin(), p.end(),
                                   [](auto u, auto v) { return u.y != v.y ? u.y < v.y : u.x < v.x; });
        std::rotate(p.begin(), it, p.end());
    };
    rotate(a);
    rotate(b);
    struct Edge {
        Wide x, y;
    };
    auto edge = [](const auto &p, std::size_t i) {
        return Edge{Wide(p[(i + 1) % p.size()].x) - p[i].x, Wide(p[(i + 1) % p.size()].y) - p[i].y};
    };
    auto half = [](Edge e) { return e.y > 0 || (e.y == 0 && e.x > 0) ? 0 : 1; };
    std::size_t n = a.size() > 1 ? a.size() : 0, m = b.size() > 1 ? b.size() : 0, i = 0, j = 0;
    Wide x = Wide(a[0].x) + b[0].x, y = Wide(a[0].y) + b[0].y;
    std::vector<Point<T>> result{point<T>(x, y)};
    while (i < n || j < m) {
        bool take_a = i < n, take_b = j < m;
        Edge u{}, v{};
        if (take_a)
            u = edge(a, i);
        if (take_b)
            v = edge(b, j);
        if (take_a && take_b) {
            int first = half(u), second = half(v);
            Wide turn = u.x * v.y - u.y * v.x;
            if (first != second) {
                take_a = first < second;
                take_b = !take_a;
            } else if (turn) {
                take_a = turn > 0;
                take_b = !take_a;
            }
        }
        if (take_a) {
            x += u.x;
            y += u.y;
            ++i;
        }
        if (take_b) {
            x += v.x;
            y += v.y;
            ++j;
        }
        if (i < n || j < m)
            result.push_back(point<T>(x, y));
    }
    result = normalize(std::move(result));
    auto first = std::min_element(result.begin(), result.end());
    std::rotate(result.begin(), first, result.end());
    return result;
}
} // namespace cpl::geometry
