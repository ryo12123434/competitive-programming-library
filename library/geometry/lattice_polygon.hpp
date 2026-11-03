#pragma once
#include <algorithm>
#include <optional>
#include "polygon.hpp"

namespace cpl::geometry {
/**
 * @brief 格子多角形の境界格子点数。各辺の gcd(|dx|,|dy|) の和。
 * @param polygon 境界順の整数頂点。末尾の始点・連続重複も可。
 * @return 異なる境界格子点数。空は0、一点は1、全共線は両端を含む線分の格子点数。
 * @pre 単純多角形または全共線。cross・座標差・絶対値・累積和が Wide に収まること。
 * 各座標 |x|,|y|<=2^62-1 かつ N*2^63<=2^127-1 は十分条件。
 * @par Complexity
 * O(N log D) 時間、O(1) 空間。D は最大座標差。
 * @code
 * #include "library/geometry/lattice_polygon.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   std::vector<Point<>> p{{0,0},{3,0},{3,3},{0,3}};
 *   assert(boundary_lattice_points(p)==12);
 *   assert(interior_lattice_points(p)==4);
 * }
 * @endcode
 */
template<class T>
Wide boundary_lattice_points(const std::vector<Point<T>>& polygon) {
    if(polygon.empty()) return 0;
    const auto [lo,hi]=std::minmax_element(polygon.begin(),polygon.end());
    if(std::all_of(polygon.begin(),polygon.end(),[&](auto p){return cross(*lo,*hi,p)==0;})) {
        Wide x=Wide(hi->x)-lo->x,y=Wide(hi->y)-lo->y;
        if(y<0) y=-y;
        while(y!=0) {Wide r=x%y;x=y;y=r;}
        return x+1;
    }
    Wide answer=0;
    for(std::size_t i=0;i<polygon.size();++i) {
        auto a=polygon[i],b=polygon[(i+1)%polygon.size()];
        Wide x=Wide(a.x)-b.x,y=Wide(a.y)-b.y;
        if(x<0) x=-x;
        if(y<0) y=-y;
        while(y!=0) { Wide r=x%y; x=y; y=r; }
        answer+=x;
    }
    return answer;
}
/**
 * @brief Pick の定理で内部格子点数を返す。退化・3点未満なら nullopt。
 * @param polygon 単純格子多角形。自己交差・穴は対象外で、検出もしない。
 * @pre signed_area2 と境界数の中間値が Wide に収まり、面積の絶対値も Wide に収まること。
 * @return (|面積の2倍|-境界数)/2+1。単純性を満たさない入力の結果は保証しない。
 * @par Complexity
 * O(N log D) 時間・O(1) 空間。使用例は boundary_lattice_points。
 */
template<class T>
std::optional<Wide> interior_lattice_points(const std::vector<Point<T>>& polygon) {
    if(polygon.size()<3) return std::nullopt;
    Wide a=signed_area2(polygon); if(a==0) return std::nullopt; if(a<0) a=-a;
    return (a-boundary_lattice_points(polygon))/2+1;
}
} // namespace cpl::geometry
