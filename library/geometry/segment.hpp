#pragma once

#include <algorithm>
#include "ccw.hpp"

namespace cpl::geometry {
/**
 * @brief 両端点を含む閉線分。a == b の長さ0の線分も扱う。
 * @tparam T Point が対応する符号付き整数型。
 * @note 判定の数値条件は ccw と同じ。構築は O(1) 時間・空間。
 * @code
 * #include "library/geometry/segment.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     Segment<long long> s{{0,0}, {2,2}}, t{{0,2}, {2,0}};
 *     assert(on_segment(Point<long long>{1,1}, s));
 *     assert(segments_intersect(s, t));
 * }
 * @endcode
 */
template <class T = long long>
struct Segment {
    Point<T> a{}, b{};
};

/**
 * @brief 点が閉線分上にあるかを判定する。使用例は Segment を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param p 判定する点。
 * @param segment 両端を含む線分。退化線分上にあるのは、その端点と等しい点のみ。
 * @return 線分上（端点を含む）なら true。
 * @pre ccw の全中間値が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr bool on_segment(const Point<T>& p, const Segment<T>& segment) {
    return ccw(segment.a, segment.b, p) == 0 &&
           std::min(segment.a.x, segment.b.x) <= p.x && p.x <= std::max(segment.a.x, segment.b.x) &&
           std::min(segment.a.y, segment.b.y) <= p.y && p.y <= std::max(segment.a.y, segment.b.y);
}

/**
 * @brief 2つの閉線分が交わるかを判定する。使用例は Segment を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param s,t 両端を含む線分。
 * @return 端点接触・共線での重なり・退化線分との接触も含め、共通点があれば true。
 * @pre 各 ccw の全中間値が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr bool segments_intersect(const Segment<T>& s, const Segment<T>& t) {
    if (on_segment(s.a, t) || on_segment(s.b, t) || on_segment(t.a, s) || on_segment(t.b, s)) return true;
    return ccw(s.a, s.b, t.a) * ccw(s.a, s.b, t.b) < 0 &&
           ccw(t.a, t.b, s.a) * ccw(t.a, t.b, s.b) < 0;
}
} // namespace cpl::geometry
