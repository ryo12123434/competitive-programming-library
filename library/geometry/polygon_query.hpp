#pragma once
#include <algorithm>
#include <utility>
#include <vector>
#include "segment.hpp"
#include "convex_hull.hpp"

namespace cpl::geometry {
/// @brief 点の位置。Boundary は辺・端点を含む。
enum class Location { Outside, Boundary, Inside };

/**
 * @brief 単純多角形に対する点の包含判定。自己交差は対象外。
 * @param polygon 境界順の頂点列。CW/CCW・末尾の始点・連続重複を許す。
 * @param p 判定点。@return 内部・境界・外部。0点は外部、面積0では辺上のみ境界。
 * @pre 既存 cross の数値条件（全座標の絶対値 <= 2^62-1 は十分条件）。
 * @par Complexity
 * O(N) 時間、O(1) 追加空間。整数の交差偶奇判定を用いる。
 * @code
 * #include "library/geometry/polygon_query.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   std::vector<Point<>> p{{0,0},{4,0},{4,4},{0,4}};
 *   assert(contains_polygon(p, Point<>{2,2}) == Location::Inside);
 *   assert(contains_triangle(p[0],p[1],p[2],Point<>{2,0}) == Location::Boundary);
 *   assert(is_convex(p));
 *   ConvexPolygon<> convex(p);
 *   assert(convex.contains({4,2}) == Location::Boundary);
 * }
 * @endcode
 */
template<class T>
Location contains_polygon(const std::vector<Point<T>>& polygon, Point<T> p) {
    bool inside = false;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        auto a = polygon[i], b = polygon[(i+1)%polygon.size()];
        if (on_segment(p, Segment<T>{a,b})) return Location::Boundary;
        if (a.y > b.y) std::swap(a,b);
        if (a.y <= p.y && p.y < b.y && cross(a,b,p) > 0) inside = !inside;
    }
    return inside ? Location::Inside : Location::Outside;
}

/**
 * @brief 三角形 abc の包含判定。退化時は3辺の和集合を境界とし、内部はない。
 * @param a,b,c 頂点（順序任意）。@param p 判定点。@return 点の位置。
 * @pre cross の全中間値が Wide に収まること。
 * @par Complexity
 * O(1) 時間・空間。使用例は contains_polygon を参照。
 */
template<class T>
Location contains_triangle(Point<T> a, Point<T> b, Point<T> c, Point<T> p) {
    if (on_segment(p, Segment<T>{a,b}) || on_segment(p, Segment<T>{b,c}) ||
        on_segment(p, Segment<T>{c,a})) return Location::Boundary;
    const int x=ccw(a,b,p), y=ccw(b,c,p), z=ccw(c,a,p);
    return (x>0 && y>0 && z>0) || (x<0 && y<0 && z<0) ? Location::Inside : Location::Outside;
}

/**
 * @brief 単純多角形が凸か判定する。面積0・3頂点未満は false。
 * @param polygon 境界順の単純多角形。末尾の始点と連続重複を除いて判定する。
 * @param strict true は全内角が180度未満であることも要求する。
 * @return 凸なら true。自己交差の検出は行わない。
 * @pre 単純多角形で、cross の数値条件を満たすこと。
 * @par Complexity
 * O(N) 時間・追加空間。使用例は contains_polygon を参照。
 */
template<class T>
bool is_convex(const std::vector<Point<T>>& polygon, bool strict=false) {
    std::vector<Point<T>> p;
    for (auto v:polygon) if (p.empty() || p.back()!=v) p.push_back(v);
    if (p.size()>1 && p.front()==p.back()) p.pop_back();
    if (p.size()<3) return false;
    int direction=0;
    for(std::size_t i=0;i<p.size();++i) {
        const int turn=ccw(p[i],p[(i+1)%p.size()],p[(i+2)%p.size()]);
        if (strict && turn==0) return false;
        if (turn!=0) { if(direction!=0 && turn!=direction) return false; direction=turn; }
    }
    return direction!=0;
}

/**
 * @brief 入力点の凸包を構築し、点の包含を二分探索で判定する。
 * @tparam T 既存 Point の符号付き整数型。
 * @note 凸多角形の頂点列を渡す場合も順序・重複・共線点を正規化する。元配列は変更しない。
 * @pre convex_hull / cross の数値条件を満たすこと。
 * @par Complexity
 * 構築 O(N log N) 時間・O(N) 空間、contains O(log(H+1)) 時間・O(1) 追加空間。
 * @note 構築例は contains_polygon を参照。空・点・線分の凸包にも対応する。
 */
template<class T=long long>
class ConvexPolygon {
    std::vector<Point<T>> hull_;
public:
    /// @brief 点列の凸包を所有する。@param points 凸多角形の頂点または任意の点群。
    explicit ConvexPolygon(std::vector<Point<T>> points):hull_(convex_hull(std::move(points))) {}
    /// @brief 正規化済み凸包を O(1) で参照する。
    const std::vector<Point<T>>& vertices() const { return hull_; }
    /// @brief 点の位置を O(log(H+1)) で返す。@param p 判定点。数値条件はクラスと同じ。
    Location contains(Point<T> p) const {
        const auto& h=hull_;
        if(h.empty()) return Location::Outside;
        if(h.size()==1) return h[0]==p ? Location::Boundary : Location::Outside;
        if(h.size()==2) return on_segment(p,Segment<T>{h[0],h[1]}) ? Location::Boundary : Location::Outside;
        const int a=ccw(h[0],h[1],p), b=ccw(h[0],h.back(),p);
        if(a<0 || b>0) return Location::Outside;
        if(a==0) return on_segment(p,Segment<T>{h[0],h[1]}) ? Location::Boundary : Location::Outside;
        if(b==0) return on_segment(p,Segment<T>{h[0],h.back()}) ? Location::Boundary : Location::Outside;
        std::size_t l=1,r=h.size()-1;
        while(r-l>1) { auto m=l+(r-l)/2; if(ccw(h[0],h[m],p)>=0) l=m; else r=m; }
        const int turn=ccw(h[l],h[r],p);
        return turn<0 ? Location::Outside : turn==0 ? Location::Boundary : Location::Inside;
    }
};
} // namespace cpl::geometry
