#pragma once
#include <algorithm>
#include <cassert>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 二部判定の結果。成功時colorは0/1、失敗時odd_cycleとodd_cycle_edgesが奇閉路を表す。
 * @note 閉路の始点は末尾に重ねない。edges[i]がvertices[i]とvertices[(i+1)%k]を結ぶ。
 * 自己ループは頂点1個・辺1個。失敗時colorは部分的で、未探索頂点は-1。
 * @code
 * #include "library/graph/bipartite_graph.hpp"
 * #include <cassert>
 * int main(){auto r=bipartite_coloring(3,{{0,1},{1,2},{2,0}});assert(!r.bipartite&&r.odd_cycle.size()==3);}
 * @endcode
 */
struct BipartiteResult {
    bool bipartite = true;
    std::vector<int> color, odd_cycle, odd_cycle_edges;
};
/**
 * @brief 無向グラフをBFSで二部判定し、塗り分けまたは奇閉路を返す。
 * @param n 頂点数。0も可。頂点は[0,n)。辺IDは入力配列の添字。
 * @param edges 無向辺。多重辺・自己ループ・非連結グラフを許す。
 * @pre n>=0、頂点数と辺数はintに収まる。
 * @par Complexity O(V+E) 時間・空間。
 */
inline BipartiteResult bipartite_coloring(int n, const std::vector<std::pair<int, int>> &edges) {
    assert(n >= 0);
    std::vector<std::vector<std::pair<int, int>>> graph(n);
    for (int id = 0; id < static_cast<int>(edges.size()); ++id) {
        auto [u, v] = edges[id];
        assert(0 <= u && u < n && 0 <= v && v < n);
        graph[u].emplace_back(v, id);
        graph[v].emplace_back(u, id);
    }
    BipartiteResult result;
    result.color.assign(n, -1);
    std::vector<int> parent(n, -1), parent_edge(n, -1), depth(n);
    std::queue<int> queue;
    for (int root = 0; root < n; ++root)
        if (result.color[root] < 0) {
            result.color[root] = 0;
            queue.push(root);
            while (!queue.empty()) {
                int v = queue.front();
                queue.pop();
                for (auto [to, id] : graph[v]) {
                    if (result.color[to] < 0) {
                        result.color[to] = result.color[v] ^ 1;
                        parent[to] = v;
                        parent_edge[to] = id;
                        depth[to] = depth[v] + 1;
                        queue.push(to);
                    } else if (result.color[to] == result.color[v]) {
                        result.bipartite = false;
                        int a = v, b = to;
                        std::vector<int> left{a}, right{b}, le, re;
                        while (a != b) {
                            if (depth[a] >= depth[b]) {
                                le.push_back(parent_edge[a]);
                                a = parent[a];
                                left.push_back(a);
                            } else {
                                re.push_back(parent_edge[b]);
                                b = parent[b];
                                right.push_back(b);
                            }
                        }
                        result.odd_cycle = left;
                        result.odd_cycle_edges = le;
                        for (int i = static_cast<int>(right.size()) - 2; i >= 0; --i)
                            result.odd_cycle.push_back(right[i]);
                        for (auto it = re.rbegin(); it != re.rend(); ++it)
                            result.odd_cycle_edges.push_back(*it);
                        result.odd_cycle_edges.push_back(id);
                        return result;
                    }
                }
            }
        }
    return result;
}
