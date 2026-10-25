#pragma once

#include "point.hpp"

namespace cpl::geometry {
/**
 * @brief 3点の向きを浮動小数点を使わずに判定する。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b,c この順に向きを調べる点。
 * @return 反時計回り +1、一直線上 0、時計回り -1。重複点を含む場合も 0。
 * @pre cross(a,b,c) の全中間値が Wide に収まること。Point の数値保証を参照。
 * @note 線上にある点の前後関係は区別しない。
 * @par Complexity
 * O(1) 時間・空間。
 * @code
 * #include "library/geometry/ccw.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     using P = Point<long long>;
 *     assert(ccw(P{0,0}, P{2,0}, P{0,1}) == 1);
 *     assert(ccw(P{0,0}, P{2,0}, P{3,0}) == 0);
 * }
 * @endcode
 */
template <class T>
constexpr int ccw(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    const Wide value = cross(a, b, c);
    return (value > 0) - (value < 0);
}
} // namespace cpl::geometry
