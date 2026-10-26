#pragma once
#include <algorithm>
#include <vector>
#include "real_point.hpp"

namespace cpl::geometry {
/**
 * @brief 実数直線 p+t*d。d==0 は無効な退化直線。
 * @note 2点指定は through(a,b)。方向を直接保存し、巨大な点に微小な方向を足す情報損失を避ける。
 * @code
 * #include "library/geometry/real_linear.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   auto l=RealLine::through({0,0},{2,0});
 *   auto p=projection(l,{1,3}); assert(p && p->y==0);
 *   auto q=reflection(l,{1,3}); assert(q && q->y==-3);
 *   auto hit=intersections(l,RealLine::through({1,-1},{1,1}));
 *   assert(hit.kind==IntersectionKind::Finite && hit.points.size()==1);
 *   assert(distance(RealPoint{0,2},l)==2);
 * }
 * @endcode
 */
struct RealLine {
    RealPoint p{}, d{};
    /// @brief 2点を通る直線を O(1) で作る。同一点なら無効な直線。
    static RealLine through(RealPoint a,RealPoint b) { return {a,b-a}; }
};
/**
 * @brief 端点を含む実数線分。長さ0も有効。
 * @code
 * #include "library/geometry/real_linear.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   RealSegment s{{0,0},{2,0}},t{{1,-1},{1,1}};
 *   assert(distance(s,t)==0 && distance(RealPoint{3,0},s)==1);
 *   assert(intersections(s,t).points.size()==1);
 * }
 * @endcode
 */
struct RealSegment { RealPoint a{}, b{}; };
/// @brief Finite は points/lines の件数、Infinite は無数、Invalid は退化直線・負半径、Uncertain は相対誤差帯。
enum class IntersectionKind { Finite, Infinite, Invalid, Uncertain };
/**
 * @brief 交点集合。有限の場合は points.size() が0/1/2。重なる線分・直線は Infinite。
 * @note 構築・使用例は RealLine を参照。Infinite の points は空（重なりの端点は返さない）。
 */
struct Intersections { IntersectionKind kind=IntersectionKind::Finite; std::vector<RealPoint> points; };
/// @brief 射影点。退化直線は nullopt。O(1)。@param line 直線。@param p 点。
inline std::optional<RealPoint> projection(RealLine line,RealPoint p) {
    const auto norm=length(line.d);if(norm==0) return std::nullopt;
    const auto u=line.d/norm;return line.p+u*dot(p-line.p,u);
}
/// @brief 鏡映点。退化直線は nullopt。O(1)。@param line 鏡とする直線。@param p 点。
inline std::optional<RealPoint> reflection(RealLine line,RealPoint p) {
    const auto foot=projection(line,p);if(!foot) return std::nullopt;
    return *foot+(*foot-p);
}
/// @brief 点と直線の距離。退化直線は nullopt。O(1)。
inline std::optional<long double> distance(RealPoint p,RealLine line) {
    const auto norm=length(line.d);if(norm==0) return std::nullopt;
    return std::abs(cross(line.d/norm,p-line.p));
}
/// @brief 点と閉線分の距離。長さ0なら端点まで。O(1)。
inline long double distance(RealPoint p,RealSegment s) {
    const auto len=length(s.b-s.a);if(len==0) return distance(p,s.a);
    const auto u=(s.b-s.a)/len;
    const auto along=std::clamp(dot(p-s.a,u),0.0L,len);
    return distance(p,s.a+u*along);
}
/**
 * @brief 直線同士の交点。平行異線は0件、同一直線は Infinite、退化は Invalid。
 * @param a,b 直線。@param tolerance 無次元の相対許容幅。
 * @pre RealPoint の有限値・中間演算の条件。全交点 API は O(1) 時間・空間。
 * @note ほぼ平行で判定が不安定なら Uncertain。計算値が0の外積は平行と扱う。使用例は RealLine。
 */
inline Intersections intersections(RealLine a,RealLine b,RelativeTolerance tolerance={}) {
    if(length(a.d)==0 || length(b.d)==0) return {IntersectionKind::Invalid,{}};
    const auto determinant=real_detail::turn(a.d,b.d,tolerance);
    if(!determinant) return {IntersectionKind::Uncertain,{}};
    if(*determinant==0) {
        const auto side=real_detail::turn(a.d,b.p-a.p,tolerance);
        if(!side) return {IntersectionKind::Uncertain,{}};
        return {*side==0 ? IntersectionKind::Infinite : IntersectionKind::Finite,{}};
    }
    return {IntersectionKind::Finite,{a.p+a.d*(cross(b.p-a.p,b.d)/cross(a.d,b.d))}};
}
namespace real_detail {
inline std::optional<bool> on_segment(RealPoint p,RealSegment s,RelativeTolerance t) {
    if(s.a==s.b) return p==s.a;
    const auto side=turn(s.b-s.a,p-s.a,t);
    if(!side) return std::nullopt;
    if(*side!=0) return false;
    return std::min(s.a.x,s.b.x)<=p.x && p.x<=std::max(s.a.x,s.b.x) &&
           std::min(s.a.y,s.b.y)<=p.y && p.y<=std::max(s.a.y,s.b.y);
}
}
/// @brief 直線と閉線分の交点。線分全体が重なるなら Infinite、長さ0なら最大1点。O(1)。
inline Intersections intersections(RealLine line,RealSegment s,RelativeTolerance t={}) {
    if(length(line.d)==0) return {IntersectionKind::Invalid,{}};
    const auto a=real_detail::turn(line.d,s.a-line.p,t), b=real_detail::turn(line.d,s.b-line.p,t);
    if(!a || !b) return {IntersectionKind::Uncertain,{}};
    if(*a==0 && *b==0) return s.a==s.b ? Intersections{IntersectionKind::Finite,{s.a}} : Intersections{IntersectionKind::Infinite,{}};
    if(*a==0) return {IntersectionKind::Finite,{s.a}};
    if(*b==0) return {IntersectionKind::Finite,{s.b}};
    if(*a==*b) return {};
    return intersections(line,RealLine::through(s.a,s.b),t);
}
/// @brief 閉線分同士の交点。共線の正の長さの重なりは Infinite、端点接触は1点。O(1)。
inline Intersections intersections(RealSegment a,RealSegment b,RelativeTolerance t={}) {
    if(a.a==a.b || b.a==b.b) {
        if(b.a==b.b) std::swap(a,b);
        const auto on=real_detail::on_segment(a.a,b,t);
        if(!on) return {IntersectionKind::Uncertain,{}};
        return *on ? Intersections{IntersectionKind::Finite,{a.a}} : Intersections{};
    }
    auto hit=intersections(RealLine::through(a.a,a.b),RealLine::through(b.a,b.b),t);
    if(hit.kind==IntersectionKind::Infinite) {
        if(a.b<a.a) std::swap(a.a,a.b);
        if(b.b<b.a) std::swap(b.a,b.b);
        const auto l=std::max(a.a,b.a),r=std::min(a.b,b.b);
        if(r<l) return {};
        if(l==r) return {IntersectionKind::Finite,{l}};
        return {IntersectionKind::Infinite,{}};
    }
    if(hit.kind!=IntersectionKind::Finite || hit.points.empty()) return hit;
    // 交点を再び外積に代入して丸め誤差を増幅せず、各端点の符号で区間内を判定する。
    const auto a0=real_detail::turn(a.b-a.a,b.a-a.a,t),a1=real_detail::turn(a.b-a.a,b.b-a.a,t);
    const auto b0=real_detail::turn(b.b-b.a,a.a-b.a,t),b1=real_detail::turn(b.b-b.a,a.b-b.a,t);
    if(!a0 || !a1 || !b0 || !b1) return {IntersectionKind::Uncertain,{}};
    return *a0 * *a1<=0 && *b0 * *b1<=0 ? hit : Intersections{};
}
/// @brief 線分間距離。判定が Uncertain なら nullopt、交差・接触・重なりは0。O(1)。
inline std::optional<long double> distance(RealSegment a,RealSegment b,RelativeTolerance t={}) {
    const auto hit=intersections(a,b,t);
    if(hit.kind==IntersectionKind::Uncertain) return std::nullopt;
    if(hit.kind==IntersectionKind::Infinite || !hit.points.empty()) return 0;
    return std::min({distance(a.a,b),distance(a.b,b),distance(b.a,a),distance(b.b,a)});
}
} // namespace cpl::geometry
