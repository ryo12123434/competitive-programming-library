#pragma once

#include <algorithm>
#include <vector>
#include "ccw.hpp"

namespace cpl::geometry {
/**
 * @brief Andrew 法で凸包を求める。入力の重複点を除去し、元の配列は変更しない。
 * @tparam T Point が対応する符号付き整数型。
 * @param points 入力点列。値渡しでコピーして処理する。
 * @param keep_collinear true なら凸包の辺上の点も残す。false なら辺の中間点を除く。
 * @return (x,y) 辞書順最小点から反時計回りの頂点列。始点を末尾に重ねない。
 * 0/1/2点は重複除去後の辞書順。全点共線なら false では両端、true では全点を辞書順で返す。
 * @pre ccw の全中間値が Wide に収まること。Point の数値保証を参照。
 * @note 共線・2点以下では幾何的な回転方向は定義しない。境界上以外の点は残さない。
 * @par Complexity
 * O(N log N) 時間、入力コピー・出力を含む O(N) 空間。
 * @code
 * #include "library/geometry/convex_hull.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     using P = Point<long long>;
 *     std::vector<P> p{{0,0}, {1,0}, {2,0}, {2,2}, {0,2}, {1,1}, {0,0}};
 *     auto corners = convex_hull(p);
 *     auto boundary = convex_hull(p, true);
 *     assert(corners.size() == 4 && boundary.size() == 5);
 *     assert((corners.front() == P{0,0}) && p.size() == 7);
 * }
 * @endcode
 */
template <class T>
std::vector<Point<T>> convex_hull(std::vector<Point<T>> points, bool keep_collinear = false) {
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() <= 2) return points;
    if (keep_collinear && std::all_of(points.begin(), points.end(), [&](const Point<T>& p) {
            return ccw(points.front(), points.back(), p) == 0;
        })) return points;

    std::vector<Point<T>> lower, upper;
    for (const auto& p : points) {
        while (lower.size() >= 2) {
            const int turn = ccw(lower[lower.size() - 2], lower.back(), p);
            if (turn > 0 || (keep_collinear && turn == 0)) break;
            lower.pop_back();
        }
        lower.push_back(p);
    }
    for (auto it = points.rbegin(); it != points.rend(); ++it) {
        while (upper.size() >= 2) {
            const int turn = ccw(upper[upper.size() - 2], upper.back(), *it);
            if (turn > 0 || (keep_collinear && turn == 0)) break;
            upper.pop_back();
        }
        upper.push_back(*it);
    }
    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}
} // namespace cpl::geometry
