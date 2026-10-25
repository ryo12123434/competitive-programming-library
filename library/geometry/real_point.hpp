#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <optional>

namespace cpl::geometry {
/**
 * @brief long double の実数点・ベクトル。整数専用 Point<T> とは別の型。
 * @pre 入力は有限で、全中間演算がオーバーフロー・アンダーフローしない範囲。
 * @note 比較は丸め後の座標の完全一致・厳密な辞書順。ソートに EPS は使わない。
 * 入力丸めで失われた情報は復元できない。極端な条件数の計算には厳密演算が必要。
 * @par Complexity
 * 各演算 O(1) 時間・空間。
 * @code
 * #include "library/geometry/real_point.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   RealPoint a{3,4}, b{1,0};
 *   assert(length(a)==5 && dot(a,b)==3 && cross(a,b)==-4);
 *   assert(distance(a,b)>4 && angle(a,b)>0);
 * }
 * @endcode
 */
struct RealPoint {
    long double x{}, y{};
    bool operator==(const RealPoint&) const = default;
    bool operator<(RealPoint b) const { return x!=b.x ? x<b.x : y<b.y; }
    RealPoint operator+(RealPoint b) const { return {x+b.x,y+b.y}; }
    RealPoint operator-(RealPoint b) const { return {x-b.x,y-b.y}; }
    RealPoint operator-() const { return {-x,-y}; }
    RealPoint operator*(long double k) const { return {x*k,y*k}; }
    /// @brief スカラー除算。@pre k != 0。
    RealPoint operator/(long double k) const { assert(k!=0);return {x/k,y/k}; }
};
/// @brief 内積。O(1)。@param a,b 実数ベクトル。積和を FMA で計算する。
inline long double dot(RealPoint a,RealPoint b) { return std::fma(a.x,b.x,a.y*b.y); }
/// @brief 外積。O(1)。@param a,b 実数ベクトル。積の丸め残差を FMA で補償する。
inline long double cross(RealPoint a,RealPoint b) {
    const auto product=a.y*b.x;
    return std::fma(a.x,b.y,-product)+std::fma(-a.y,b.x,product);
}
/// @brief 3点外積。O(1)。@param a,b,c 実数点。座標差の丸めには注意。
inline long double cross(RealPoint a,RealPoint b,RealPoint c) { return cross(b-a,c-a); }
/// @brief 長さの2乗。O(1)。@param p ベクトル。
inline long double norm2(RealPoint p) { return dot(p,p); }
/// @brief 距離の2乗。O(1)。@param a,b 点。
inline long double dist2(RealPoint a,RealPoint b) { return norm2(a-b); }
/// @brief hypot による長さ。O(1)。@param p ベクトル。
inline long double length(RealPoint p) { return std::hypot(p.x,p.y); }
/// @brief 2点の距離。O(1)。@param a,b 点。
inline long double distance(RealPoint a,RealPoint b) { return length(a-b); }
/// @brief 反時計回りに90度回転したベクトル。O(1)。
inline RealPoint perpendicular(RealPoint p) { return {-p.y,p.x}; }
/// @brief 2ベクトル間の角度 [0,pi]（ラジアン）。O(1)。@pre 両ベクトルは非零。
inline long double angle(RealPoint a,RealPoint b) {
    const auto u=length(a),v=length(b);assert(u>0 && v>0);
    return std::atan2(std::abs(cross(a/u,b/v)),dot(a/u,b/v));
}
/**
 * @brief 実数判定の相対許容幅。既定値は long double の機械イプシロンの32倍。
 * @note 長さ比較には長さの尺度、外積には積の絶対値和を使う。単一の絶対 EPS は使わない。
 * 非零の比較値が許容幅内なら交点 API は Uncertain を返す。計算値が厳密に0なら接触・平行の規約を使う。
 * 許容幅は数値的不安定性の診断であり、入力丸めを含む厳密な誤差証明ではない。
 * @pre relative は有限かつ非負。0 は丸め後の計算値をそのまま比較する設定。
 * @code
 * #include "library/geometry/real_point.hpp"
 * #include <cassert>
 * int main() { cpl::geometry::RelativeTolerance policy{0}; assert(policy.relative==0); }
 * @endcode
 */
struct RelativeTolerance { long double relative=32*std::numeric_limits<long double>::epsilon(); };
namespace real_detail {
inline std::optional<int> sign(long double value,long double scale,RelativeTolerance tolerance) {
    assert(std::isfinite(tolerance.relative) && tolerance.relative>=0);
    if(value==0) return 0;
    if(std::abs(value)<=tolerance.relative*scale) return std::nullopt;
    return value>0 ? 1 : -1;
}
inline std::optional<int> turn(RealPoint a,RealPoint b,RelativeTolerance t) {
    return sign(cross(a,b),std::abs(a.x*b.y)+std::abs(a.y*b.x),t);
}
}
} // namespace cpl::geometry
