#pragma once

#include <compare>
#include <type_traits>

namespace cpl::geometry {

/// @brief 積・面積・距離の2乗の計算型。GNU C++ の符号付き128bit整数。
using Wide = __int128;

/**
 * @brief 整数の2次元点・ベクトル。比較は (x,y) の辞書順。
 * @tparam T long long、int など64bit以下の符号付き整数型。浮動小数点・unsigned は不可。
 * @note 座標は公開メンバ x, y。初期値は (0,0)。算術演算の結果も Point<T>。
 * @pre 加算・減算・符号反転の結果の各座標が T に収まること。
 * @note dot / cross / norm2 / dist2 は Wide で計算する。
 * 各入力座標の絶対値が M <= 2^62-1 なら、これらと ccw の全中間値が収まる。
 * 範囲外は各演算の128bit中間値が収まる場合だけ有効。全64bit座標を一律には保証しない。
 * @note 3点の外積・距離は T の減算をせず、拡張してから差を取る。
 * Point 同士の差を明示的に作る場合は、差が T に収まるという別の条件が必要。
 * @par Complexity
 * 構築・比較・算術演算は O(1) 時間・空間。
 * @code
 * #include "library/geometry/point.hpp"
 * #include <cassert>
 * int main() {
 *     using namespace cpl::geometry;
 *     using P = Point<long long>;
 *     P a{1, 2}, b{3, 4};
 *     assert((a + b == P{4, 6}) && (b - a == P{2, 2}));
 *     assert((-a == P{-1, -2}) && a < b);
 *     a += b; a -= b;
 *     assert(dot(a, b) == 11 && cross(a, b) == -2);
 *     assert(cross(P{0, 0}, a, b) == -2);
 *     assert(norm2(a) == 5 && dist2(a, b) == 8);
 * }
 * @endcode
 */
template <class T = long long>
struct Point {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8,
                  "Point requires a signed integer type of at most 64 bits");
    T x{}, y{};

    /// @brief 座標の完全一致と辞書順比較。O(1)。
    constexpr auto operator<=>(const Point&) const = default;

    /// @brief ベクトルを加算する。結果が T に収まること。O(1)。
    constexpr Point& operator+=(const Point& other) {
        x = static_cast<T>(Wide(x) + other.x);
        y = static_cast<T>(Wide(y) + other.y);
        return *this;
    }

    /// @brief ベクトルを減算する。結果が T に収まること。O(1)。
    constexpr Point& operator-=(const Point& other) {
        x = static_cast<T>(Wide(x) - other.x);
        y = static_cast<T>(Wide(y) - other.y);
        return *this;
    }

    /// @brief 座標の符号を反転する。結果が T に収まること。O(1)。
    constexpr Point operator-() const {
        return {static_cast<T>(-Wide(x)), static_cast<T>(-Wide(y))};
    }

    /// @brief ベクトルの和。結果が T に収まること。O(1)。
    constexpr Point operator+(const Point& other) const {
        Point result = *this;
        return result += other;
    }

    /// @brief ベクトルの差。結果が T に収まること。O(1)。
    constexpr Point operator-(const Point& other) const {
        Point result = *this;
        return result -= other;
    }
};

/**
 * @brief 内積 a.x*b.x + a.y*b.y を返す。使用例・座標の保証範囲は Point を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b 同じ座標型のベクトル。
 * @return 符号付き128bitの内積。
 * @pre 各積と和が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr Wide dot(const Point<T>& a, const Point<T>& b) {
    return Wide(a.x) * b.x + Wide(a.y) * b.y;
}

/**
 * @brief 原点からの2ベクトルの外積を返す。使用例・座標の保証範囲は Point を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b 同じ座標型のベクトル。
 * @return a.x*b.y - a.y*b.x。正なら a から b へ反時計回り。
 * @pre 各積と差が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr Wide cross(const Point<T>& a, const Point<T>& b) {
    return Wide(a.x) * b.y - Wide(a.y) * b.x;
}

/**
 * @brief (b-a) と (c-a) の外積。減算前に128bitへ拡張する。使用例は Point を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b,c 向きを調べる3点。
 * @return 三角形 abc の符号付き面積の2倍。一直線上・重複点は 0。
 * @pre 差・積・最終的な差が Wide に収まること。保証範囲は Point と同じ。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr Wide cross(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    const Wide bx = Wide(b.x) - a.x, by = Wide(b.y) - a.y;
    const Wide cx = Wide(c.x) - a.x, cy = Wide(c.y) - a.y;
    return bx * cy - by * cx;
}

/**
 * @brief 原点からの距離の2乗。使用例・保証範囲は Point を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param p ベクトル。
 * @return p.x^2 + p.y^2 を Wide で返す。
 * @pre 積と和が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr Wide norm2(const Point<T>& p) {
    return dot(p, p);
}

/**
 * @brief 2点間の距離の2乗。減算前に128bitへ拡張する。使用例は Point を参照。
 * @tparam T Point が対応する符号付き整数型。
 * @param a,b 距離を調べる2点。
 * @return (a.x-b.x)^2 + (a.y-b.y)^2 を Wide で返す。
 * @pre 差・積・和が Wide に収まること。保証範囲は Point と同じ。
 * @par Complexity
 * O(1) 時間・空間。
 */
template <class T>
constexpr Wide dist2(const Point<T>& a, const Point<T>& b) {
    const Wide x = Wide(a.x) - b.x, y = Wide(a.y) - b.y;
    return x * x + y * y;
}
} // namespace cpl::geometry
