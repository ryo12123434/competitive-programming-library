#pragma once
#include "cycle_detection.hpp"
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

/// @brief なもり分解。root[v] は所属する閉路頂点、cycle_index[v] は閉路内の位置（閉路外は-1）。
struct NamoriResult {
    std::vector<int> cycle_vertices, cycle_edge_ids, root, cycle_index;
};
/**
 * @brief 連結かつ E=V の単一閉路無向グラフを、閉路と付属する木に分解する。
 * @param n 頂点数。@param edges 無向辺を各IDにつき一度だけ格納。
 * @return 条件を満たさない（空・非連結・E!=V）なら nullopt。端点範囲は find_cycle と同じ前提。
 * @note 自己ループ1辺・平行辺2辺の閉路も可。閉路の始点は再掲しない。一般のFunctional Graphとは別。
 * @par Complexity O(V+E) 時間・追加空間。
 * @code
 * #include "library/graph/namori.hpp"
 * #include <cassert>
 * int main(){auto r=decompose_namori(3,{{0,1},{0,1},{1,2}});assert(r&&r->root[2]==1&&r->cycle_index[2]==-1);}
 * @endcode
 */
inline std::optional<NamoriResult> decompose_namori(int n, const std::vector<std::pair<int, int>> &edges) {
    assert(n >= 0);
    if (n == 0 || edges.size() != static_cast<std::size_t>(n))
        return std::nullopt;
    auto cycle = find_cycle(n, edges, false);
    if (!cycle)
        return std::nullopt;
    NamoriResult result;
    result.cycle_vertices = std::move(cycle->vertices);
    result.cycle_edge_ids = std::move(cycle->edge_ids);
    result.root.assign(n, -1);
    result.cycle_index.assign(n, -1);
    std::vector<std::vector<int>> graph(n);
    for (auto [u, v] : edges) {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> queue = result.cycle_vertices;
    for (std::size_t i = 0; i < queue.size(); ++i) {
        result.root[queue[i]] = queue[i];
        result.cycle_index[queue[i]] = static_cast<int>(i);
    }
    for (std::size_t i = 0; i < queue.size(); ++i)
        for (int to : graph[queue[i]])
            if (result.root[to] == -1) {
                result.root[to] = result.root[queue[i]];
                queue.push_back(to);
            }
    if (queue.size() != static_cast<std::size_t>(n))
        return std::nullopt;
    return result;
}
