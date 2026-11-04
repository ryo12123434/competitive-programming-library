#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

/// @brief 閉路。edge_ids[i] は vertices[i] から vertices[(i+1)%size] への辺。始点の再掲なし。
struct CycleResult {
    std::vector<int> vertices, edge_ids;
};
/**
 * @brief 有向・無向グラフの閉路を1つ検出・復元する。反復DFSでスタック安全。
 * @param n 頂点数。@param edges 辺ID順の端点配列。有向時は first -> second。
 * @param directed true は有向、false は無向。無向辺は入力に一度だけ置く。
 * @return 閉路なしなら nullopt。自己ループは長さ1、無向平行辺は長さ2の閉路として扱う。
 * @pre n>=0、端点は [0,n)、辺数は INT_MAX 以下。非連結・空グラフも可。
 * @par Complexity O(V+E) 時間・追加空間。
 * @code
 * #include "library/graph/cycle_detection.hpp"
 * #include <cassert>
 * int main(){auto c=find_cycle(2,{{0,1},{0,1}},false);assert(c&&c->edge_ids.size()==2);}
 * @endcode
 */
inline std::optional<CycleResult> find_cycle(int n, const std::vector<std::pair<int, int>> &edges,
                                             bool directed = true) {
    assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    std::vector<std::vector<std::pair<int, int>>> graph(n);
    for (std::size_t i = 0; i < edges.size(); ++i) {
        auto [u, v] = edges[i];
        assert(0 <= u && u < n && 0 <= v && v < n);
        graph[u].push_back({v, static_cast<int>(i)});
        if (!directed)
            graph[v].push_back({u, static_cast<int>(i)});
    }
    std::vector<int> color(n), parent(n, -1), parent_edge(n, -1), stack;
    std::vector<std::size_t> next(n);
    for (int root = 0; root < n; ++root) {
        if (color[root])
            continue;
        color[root] = 1;
        stack.push_back(root);
        while (!stack.empty()) {
            int v = stack.back();
            if (next[v] == graph[v].size()) {
                color[v] = 2;
                stack.pop_back();
                continue;
            }
            auto [to, id] = graph[v][next[v]++];
            if (!directed && id == parent_edge[v])
                continue;
            if (color[to] == 0) {
                parent[to] = v;
                parent_edge[to] = id;
                color[to] = 1;
                stack.push_back(to);
            } else if (color[to] == 1) {
                CycleResult result;
                for (int u = v;; u = parent[u]) {
                    result.vertices.push_back(u);
                    if (u == to)
                        break;
                }
                std::reverse(result.vertices.begin(), result.vertices.end());
                for (std::size_t i = 1; i < result.vertices.size(); ++i)
                    result.edge_ids.push_back(parent_edge[result.vertices[i]]);
                result.edge_ids.push_back(id);
                return result;
            }
        }
    }
    return std::nullopt;
}
