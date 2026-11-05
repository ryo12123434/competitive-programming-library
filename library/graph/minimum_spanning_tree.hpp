#pragma once
#include "../data_structure/union_find.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

/// @brief 無向辺。ID は入力配列の添字。自己ループ・平行辺・負の cost も可。
struct MSTEdge {
    int from, to;
    long long cost;
};
/// @brief Kruskal の結果。edge_ids は採用順、components は孤立点も含む成分数。
struct MinimumSpanningForest {
    __int128_t total_weight = 0;
    std::vector<int> edge_ids;
    int components = 0;
};
/**
 * @brief Kruskal 法で最小全域森を求める。連結なら最小全域木。
 * @param n 頂点数。0 なら重み0・辺なし・成分数0。
 * @param edges 無向辺の配列。重み同値は辺IDの小さい順に採用を試す。
 * @return 総重み（128bit）・採用辺ID・成分数。自己ループは採用しない。
 * @pre n>=0、端点は [0,n)、辺数は INT_MAX 以下。
 * @par Complexity
 * O(N+E log(E+1)) 時間、O(N+E) 追加空間。既存 UnionFind を利用する。
 * @code
 * #include "library/graph/minimum_spanning_tree.hpp"
 * #include <cassert>
 * int main(){auto r=kruskal(3,{{0,1,-2},{1,2,4},{0,2,5}});assert(r.components==1&&r.total_weight==2);}
 * @endcode
 */
inline MinimumSpanningForest kruskal(int n, const std::vector<MSTEdge> &edges) {
    assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    std::vector<int> order(edges.size());
    std::iota(order.begin(), order.end(), 0);
    for (const auto &e : edges) {
        (void)e;
        assert(0 <= e.from && e.from < n && 0 <= e.to && e.to < n);
    }
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return edges[a].cost != edges[b].cost ? edges[a].cost < edges[b].cost : a < b;
    });
    UnionFind uf(n);
    MinimumSpanningForest result;
    result.components = n;
    for (int id : order) {
        const auto &e = edges[id];
        if (uf.unite(e.from, e.to)) {
            result.total_weight += e.cost;
            result.edge_ids.push_back(id);
            --result.components;
        }
    }
    return result;
}
