#pragma once
#include "real_linear.hpp"

namespace cpl::geometry {
/**
 * @brief 円の中心と半径。半径0は点円、負半径は Invalid。全操作 O(1) 時間・空間。
 * @note RealPoint の数値条件に加え、半径は有限であること。
 * @code
 * #include "library/geometry/real_circle.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   RealCircle c{{0,0},1};
 *   auto hit=intersections(RealLine::through({-2,0},{2,0}),c);
 *   assert(hit.kind==IntersectionKind::Finite && hit.points.size()==2);
 *   assert(intersections(c,RealCircle{{2,0},1}).points.size()==1);
 *   assert(tangents_from(c,{2,0}).lines.size()==2);
 *   assert(common_tangents(c,RealCircle{{4,0},1}).lines.size()==4);
 * }
 * @endcode
 */
struct RealCircle { RealPoint center{}; long double radius{}; };
/**
 * @brief 接線集合。Finite の lines は有限本、Infinite は無数。各直線は接点を p とする。
 * @note 点円の点と同じ位置からの接線は無数。使用例は RealCircle。
 */
struct Tangents { IntersectionKind kind=IntersectionKind::Finite; std::vector<RealLine> lines; };

/// @brief 直線と円の交点。接点は1点、同じ点の重複は返さない。退化直線・負半径は Invalid。
inline Intersections intersections(RealLine line,RealCircle circle,RelativeTolerance t={}) {
    if(circle.radius<0 || length(line.d)==0) return {IntersectionKind::Invalid,{}};
    const auto u=line.d/length(line.d);
    const auto offset=cross(u,circle.center-line.p);
    const auto dist=std::abs(offset), r=circle.radius;
    const auto side=real_detail::sign(dist-r,std::max(dist,r),t);
    if(!side) return {IntersectionKind::Uncertain,{}};
    if(*side>0) return {};
    const auto foot=circle.center-perpendicular(u)*offset;
    if(*side==0) return {IntersectionKind::Finite,{foot}};
    const auto shift=u*std::sqrt((r-dist)*(r+dist));
    return {IntersectionKind::Finite,{foot-shift,foot+shift}};
}
/// @brief 円同士の交点。正半径の同一円は Infinite、同一点円は1点、同心異円は0点。
inline Intersections intersections(RealCircle a,RealCircle b,RelativeTolerance t={}) {
    if(a.radius<0 || b.radius<0) return {IntersectionKind::Invalid,{}};
    const auto d=distance(a.center,b.center);
    if(d==0) {
        if(a.radius!=b.radius) return {};
        return a.radius==0 ? Intersections{IntersectionKind::Finite,{a.center}} : Intersections{IntersectionKind::Infinite,{}};
    }
    const auto scale=std::max({d,a.radius,b.radius});
    const auto outer=real_detail::sign(d-(a.radius+b.radius),scale,t);
    const auto inner=real_detail::sign(d-std::abs(a.radius-b.radius),scale,t);
    if(!outer || !inner) return {IntersectionKind::Uncertain,{}};
    if(*outer>0 || *inner<0) return {};
    const auto u=(b.center-a.center)/d;
    const auto along=(d+(a.radius-b.radius)*((a.radius+b.radius)/d))/2;
    const auto foot=a.center+u*along;
    if(*outer==0 || *inner==0) return {IntersectionKind::Finite,{foot}};
    const auto h2=(a.radius-along)*(a.radius+along);
    if(h2<0) return {IntersectionKind::Uncertain,{}};
    const auto shift=perpendicular(u)*std::sqrt(h2);
    return {IntersectionKind::Finite,{foot-shift,foot+shift}};
}
/// @brief 点 p から円への接線。円内0本、円上1本、円外2本。点円と異なる点なら1本。
inline Tangents tangents_from(RealCircle circle,RealPoint p,RelativeTolerance t={}) {
    if(circle.radius<0) return {IntersectionKind::Invalid,{}};
    const auto d=distance(circle.center,p),r=circle.radius;
    if(r==0) return d==0 ? Tangents{IntersectionKind::Infinite,{}} : Tangents{IntersectionKind::Finite,{{circle.center,p-circle.center}}};
    const auto side=real_detail::sign(d-r,std::max(d,r),t);
    if(!side) return {IntersectionKind::Uncertain,{}};
    if(*side<0) return {};
    const auto u=(p-circle.center)/d;
    if(*side==0) return {IntersectionKind::Finite,{{p,perpendicular(u)}}};
    const auto ratio=r/d;
    const auto base=circle.center+u*(r*ratio);
    const auto shift=perpendicular(u)*(r*std::sqrt((1-ratio)*(1+ratio)));
    const auto a=base-shift,b=base+shift;
    return {IntersectionKind::Finite,{{a,p-a},{b,p-b}}};
}
/// @brief 2円の共通接線（0～4本）。同一円は点円も含め Infinite。各 p は第1円の接点。
inline Tangents common_tangents(RealCircle a,RealCircle b,RelativeTolerance t={}) {
    if(a.radius<0 || b.radius<0) return {IntersectionKind::Invalid,{}};
    const auto d=distance(a.center,b.center);
    if(d==0) return {a.radius==b.radius ? IntersectionKind::Infinite : IntersectionKind::Finite,{}};
    const auto u=(b.center-a.center)/d;
    Tangents result;
    for(int sign:{1,-1}) {
        if(sign==-1 && (a.radius==0 || b.radius==0)) continue;
        const auto difference=a.radius-sign*b.radius;
        const auto side=real_detail::sign(d-std::abs(difference),std::max({d,a.radius,b.radius}),t);
        if(!side) return {IntersectionKind::Uncertain,{}};
        if(*side<0) continue;
        const auto h=difference/d;
        const auto height=*side==0 ? 0.0L : std::sqrt(std::max(0.0L,(1-h)*(1+h)));
        for(int direction:{1,-1}) {
            if(direction==-1 && (height==0 || (a.radius==0 && b.radius==0))) continue;
            const auto normal=u*h+perpendicular(u)*(height*direction);
            result.lines.push_back({a.center+normal*a.radius,perpendicular(normal)});
        }
    }
    return result;
}
} // namespace cpl::geometry
