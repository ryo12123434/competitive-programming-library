#pragma once

#include <cstddef>
#include <vector>
#include "point.hpp"

namespace cpl::geometry {
/**
 * @brief Shoelace formula による符号付き面積の2倍。
 * @tparam T Point が対応する符号付き整数型。
 * @param polygon 境界に沿って並んだ頂点列。末尾の始点の繰り返しはあってもなくてもよい。
 * @return 反時計回りなら正、時計回りなら負。3点未満・面積0は 0。除算・丸めを行わない。
 * @pre 幾何的な面積として使う場合は単純多角形。自己交差では代数的面積を返す。
 * @pre 各外積と逐次加算の全中間値が Wide に収まること。最終値だけが収まればよいわけではない。
 * @note N頂点、各座標の絶対値 <= M なら 2*N*M^2 <= 2^127-1 は十分条件。
 * 点演算の範囲内でも、頂点数によって累積があふれうる。Wide の標準 ostream 出力はない。
 * @par Complexity
 * O(N) 時間、O(1) 追加空間。
 * @code
 * #include "library/geometry/polygon.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     std::vector<Point<long long>> p{{0,0}, {3,0}, {0,1}};
 *     Wide twice = signed_area2(p); // 実面積は twice / 2（ここでは 1.5）。
 *     assert(twice == 3 && polygon_orientation(p) == 1);
 * }
 * @endcode
 */
template <class T>
Wide signed_area2(const std::vector<Point<T>>& polygon) {
    if (polygon.size() < 3) return 0;
    Wide result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        result += cross(polygon[i], polygon[(i + 1) % polygon.size()]);
    }
    return result;
}

/**
 * @brief 多角形の向きを返す。使用例は signed_area2 を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param polygon 境界に沿った頂点列。単純多角形を想定する。
 * @return 反時計回り +1、面積0（3点未満を含む）0、時計回り -1。
 * @pre signed_area2 の数値条件を満たすこと。
 * @par Complexity
 * O(N) 時間、O(1) 追加空間。
 */
template <class T>
int polygon_orientation(const std::vector<Point<T>>& polygon) {
    const Wide value = signed_area2(polygon);
    return (value > 0) - (value < 0);
}
} // namespace cpl::geometry
