#pragma once

#include <algorithm>
#include <vector>
#include "point.hpp"

namespace cpl::geometry {
/**
 * @brief 原点を基準とする偏角の比較関数。正の x 軸から反時計回り [0,2pi)。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b 比較するベクトル。
 * @return a を先に置く場合 true。同じ点同士は false。
 * @note ゼロベクトルはすべての非ゼロベクトルより前。同一方向は norm2 の小さい順。
 * 前提の範囲内で狭義弱順序を満たす。atan2 や浮動小数点は使用しない。
 * @pre cross(a,b) と norm2(a), norm2(b) の全中間値が Wide に収まること。
 * 各座標の絶対値 <= 2^62-1 は十分条件。Point の数値保証を参照。
 * @par Complexity
 * O(1) 時間・空間。
 * @code
 * #include "library/geometry/polar_angle_sort.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     using P = Point<long long>;
 *     assert(polar_angle_less(P{0,0}, P{1,0}));
 *     assert(polar_angle_less(P{1,0}, P{2,0}));
 *     std::vector<P> p{{0,-1}, {0,1}, {1,0}};
 *     std::sort(p.begin(), p.end(), polar_angle_less<long long>);
 *     assert((p.front() == P{1,0}));
 * }
 * @endcode
 */
template <class T>
constexpr bool polar_angle_less(const Point<T>& a, const Point<T>& b) {
    const bool az = a.x == 0 && a.y == 0, bz = b.x == 0 && b.y == 0;
    if (az || bz) return az && !bz;
    const bool upper_a = a.y > 0 || (a.y == 0 && a.x > 0);
    const bool upper_b = b.y > 0 || (b.y == 0 && b.x > 0);
    if (upper_a != upper_b) return upper_a;
    const Wide turn = cross(a, b);
    if (turn != 0) return turn > 0;
    return norm2(a) < norm2(b);
}

/**
 * @brief ベクトル列を polar_angle_less の規約でその場でソートする。
 * @tparam T Point が対応する符号付き整数型。
 * @param points 入出力の点列。空列・重複点も指定できる。
 * @pre polar_angle_less の数値条件を満たすこと。
 * @par Complexity
 * O(N log N) 時間、ソートのスタック O(log N)。
 * @code
 * #include "library/geometry/polar_angle_sort.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     std::vector<Point<int>> p{{0,-1}, {1,0}, {0,0}, {0,1}};
 *     polar_angle_sort(p);
 *     assert((p[0] == Point<int>{0,0}) && (p[1] == Point<int>{1,0}));
 * }
 * @endcode
 */
template <class T>
void polar_angle_sort(std::vector<Point<T>>& points) {
    std::sort(points.begin(), points.end(), polar_angle_less<T>);
}
} // namespace cpl::geometry
