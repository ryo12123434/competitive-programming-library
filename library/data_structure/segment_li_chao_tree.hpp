#pragma once
#include "li_chao_tree.hpp"

/**
 * @brief 区間限定の直線追加に対応する座標圧縮Li Chao Tree。実装はLiChaoTreeと共有する型別名。
 * @note 全域追加add_line(a,b)、半開座標域追加add_segment(l,r,a,b)、最小値query(x)を提供する。
 * add_segment_indices(l,r,a,b)はソート済み異なる座標の添字域で、最大座標も区間に含められる。
 * @pre query座標は事前登録。a,b,xはlong long全域、返り値はoptional<__int128>。空ならnullopt。
 * @par Complexity M座標に保持O(M)、構築O(M log(M+1))、全域追加・取得O(log(M+1))、区間追加O(log^2(M+1))。
 * @code
 * #include "library/data_structure/segment_li_chao_tree.hpp"
 * #include <cassert>
 * int main(){SegmentLiChaoTree h({-1,0,1});h.add_segment(0,2,3,4);assert(!h.query(-1)&&h.query(1)==7);h.add_line(0,2);assert(h.query(-1)==2);}
 * @endcode
 */
using SegmentLiChaoTree = LiChaoTree;
