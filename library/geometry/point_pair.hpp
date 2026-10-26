#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>
#include "convex_hull.hpp"

namespace cpl::geometry {
/**
 * @brief 点対の元配列インデックスと距離の2乗。first < second。同距離ならインデックス対の辞書順最小を選ぶ。
 * @note 構築例は closest_pair を参照。距離は整数 Wide で、平方根は取らない。
 */
struct PointPair { std::size_t first, second; Wide distance2; };
namespace pair_detail {
inline bool less_ids(const PointPair& a,const PointPair& b) {
    return std::pair{a.first,a.second}<std::pair{b.first,b.second};
}
template<class T> PointPair make(const std::vector<Point<T>>& p,std::size_t i,std::size_t j) {
    if(i>j) std::swap(i,j);
    return {i,j,dist2(p[i],p[j])};
}
}
/**
 * @brief 最近点対を分割統治＋y順マージで求める。
 * @tparam T Point の整数型。@param points 元の点列（変更しない）。
 * @return 2点未満は nullopt。重複点の異なるインデックスも有効な距離0の点対。
 * @pre 全座標の絶対値 <= 2^62-1 は十分条件。範囲外では差・積・平方和がすべて Wide に収まること。
 * @par Complexity
 * O(N log N) 時間、O(N) 追加空間、再帰深さ O(log N)。
 * @code
 * #include "library/geometry/point_pair.hpp"
 * #include <cassert>
 * int main() {
 *   using namespace cpl::geometry;
 *   std::vector<Point<>> p{{0,0},{2,0},{1,0},{0,0}};
 *   auto near=closest_pair(p), far=farthest_pair(p);
 *   assert(near && near->first==0 && near->second==3 && near->distance2==0);
 *   assert(far && far->first==0 && far->second==1 && far->distance2==4);
 * }
 * @endcode
 */
template<class T>
std::optional<PointPair> closest_pair(const std::vector<Point<T>>& points) {
    const std::size_t n=points.size();
    if(n<2) return std::nullopt;
    std::vector<std::size_t> ids(n),temp(n);
    std::iota(ids.begin(),ids.end(),0);
    std::sort(ids.begin(),ids.end(),[&](auto i,auto j){return points[i]!=points[j]?points[i]<points[j]:i<j;});
    PointPair best=pair_detail::make(points,ids[0],ids[1]);
    auto update=[&](auto i,auto j){auto p=pair_detail::make(points,i,j);
        if(p.distance2<best.distance2 || (p.distance2==best.distance2 && pair_detail::less_ids(p,best))) best=p;};
    for(std::size_t i=1;i<n;++i) if(points[ids[i-1]]==points[ids[i]]) update(ids[i-1],ids[i]);
    if(best.distance2==0) return best;
    auto by_y=[&](auto i,auto j){return points[i].y!=points[j].y ? points[i].y<points[j].y : i<j;};
    std::function<void(std::size_t,std::size_t)> solve=[&](auto l,auto r) {
        if(r-l<=3) {
            for(auto i=l;i<r;++i) for(auto j=i+1;j<r;++j) update(ids[i],ids[j]);
            std::sort(ids.begin()+l,ids.begin()+r,by_y); return;
        }
        const auto m=l+(r-l)/2;
        const T mid_x=points[ids[m]].x;
        solve(l,m); solve(m,r);
        std::merge(ids.begin()+l,ids.begin()+m,ids.begin()+m,ids.begin()+r,temp.begin()+l,by_y);
        std::copy(temp.begin()+l,temp.begin()+r,ids.begin()+l);
        std::size_t count=0;
        for(auto i=l;i<r;++i) {
            const auto id=ids[i]; const Wide dx=Wide(points[id].x)-mid_x;
            if(dx*dx>best.distance2) continue;
            for(auto j=count;j>0;) {
                const auto other=temp[l+--j]; const Wide dy=Wide(points[id].y)-points[other].y;
                if(dy*dy>best.distance2) break;
                update(id,other);
            }
            temp[l+count++]=id;
        }
    };
    solve(0,n); return best;
}

/**
 * @brief 凸包と Rotating Calipers で最遠点対を求める。同距離時の規約は PointPair と同じ。
 * @param points 元配列。2点未満は nullopt。全点一致なら (0,1,0)。
 * @pre closest_pair と cross の数値条件を満たすこと。
 * @par Complexity
 * O(N log N) 時間・O(N) 空間。凸包構築後の calipers は O(H)。使用例は closest_pair。
 */
template<class T>
std::optional<PointPair> farthest_pair(const std::vector<Point<T>>& points) {
    if(points.size()<2) return std::nullopt;
    const auto h=convex_hull(points);
    if(h.size()==1) return PointPair{0,1,0};
    std::vector<std::size_t> ids(points.size()); std::iota(ids.begin(),ids.end(),0);
    std::sort(ids.begin(),ids.end(),[&](auto i,auto j){return points[i]!=points[j]?points[i]<points[j]:i<j;});
    std::vector<std::size_t> original;
    for(auto p:h) {
        auto it=std::lower_bound(ids.begin(),ids.end(),p,[&](auto id,auto value){return points[id]<value;});
        original.push_back(*it);
    }
    PointPair best=pair_detail::make(points,original[0],original[1]);
    auto update=[&](auto i,auto j){if(i==j) return; auto p=pair_detail::make(points,original[i],original[j]);
        if(p.distance2>best.distance2 || (p.distance2==best.distance2 && pair_detail::less_ids(p,best))) best=p;};
    std::size_t j=1;
    for(std::size_t i=0;i<h.size();++i) {
        const auto k=(i+1)%h.size();
        while(cross(h[i],h[k],h[(j+1)%h.size()])>cross(h[i],h[k],h[j])) j=(j+1)%h.size();
        update(i,j); update(k,j);
        if(cross(h[i],h[k],h[(j+1)%h.size()])==cross(h[i],h[k],h[j])) {
            update(i,(j+1)%h.size()); update(k,(j+1)%h.size());
        }
    }
    return best;
}
} // namespace cpl::geometry
