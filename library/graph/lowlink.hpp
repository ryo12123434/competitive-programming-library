#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 無向グラフの Lowlink で関節点と橋を求める。
 * @note 頂点・辺 ID は 0-indexed。非連結グラフ・多重辺・自己ループに対応する。
 * @note DFS は反復処理で行い、親への辺は辺 ID で区別する。
 */
struct Lowlink {
    struct Edge { int to, id; };
    int V;
    std::vector<std::vector<Edge>> G;
    /// @brief add_edge の登録順に格納する無向辺の端点。
    std::vector<std::pair<int, int>> edges;
    /// @brief DFS の訪問順序と、部分木から後退辺を高々 1 本通って到達できる最小の訪問順序。
    std::vector<int> ord, low;
    /// @brief 関節点の頂点番号（昇順）。@pre 最後の辺追加後に build 実行済み。
    std::vector<int> articulation_points;
    /// @brief 橋の辺 ID（順序は不定）。@pre 最後の辺追加後に build 実行済み。
    std::vector<int> bridges;

    /// @brief V 頂点のグラフを O(V) で構築する。@pre V >= 0。
    explicit Lowlink(int V) : V(V) {
        assert(V >= 0);
        G.resize(V);
    }

    /// @brief 無向辺を償却 O(1) で追加し、辺 ID を返す。@pre 端点は [0, V)、辺数は int に収まること。
    int add_edge(int u, int v) {
        assert(0 <= u && u < V && 0 <= v && v < V);
        assert(edges.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(edges.size());
        edges.emplace_back(u, v);
        G[u].push_back({v, id});
        G[v].push_back({u, id});
        return id;
    }

    /// @brief 全連結成分を調べて結果を毎回再構築する。O(V + E) 時間、追加空間 O(V)。
    void build() {
        ord.assign(V, -1);
        low.assign(V, -1);
        articulation_points.clear();
        bridges.clear();
        std::vector<int> parent(V, -1), parent_edge(V, -1), children(V, 0), stack;
        std::vector<std::size_t> next(V, 0);
        std::vector<bool> articulation(V, false);
        int timer = 0;
        for (int root = 0; root < V; ++root) {
            if (ord[root] != -1) continue;
            ord[root] = low[root] = timer++;
            stack.push_back(root);
            while (!stack.empty()) {
                int v = stack.back();
                if (next[v] == G[v].size()) {
                    stack.pop_back();
                    int p = parent[v];
                    if (p == -1) {
                        articulation[v] = children[v] >= 2;
                    } else {
                        low[p] = std::min(low[p], low[v]);
                        if (ord[p] < low[v]) bridges.push_back(parent_edge[v]);
                        if (parent[p] != -1 && ord[p] <= low[v]) articulation[p] = true;
                    }
                    continue;
                }
                auto [to, id] = G[v][next[v]++];
                if (id == parent_edge[v]) continue;
                if (ord[to] != -1) {
                    low[v] = std::min(low[v], ord[to]);
                    continue;
                }
                parent[to] = v;
                parent_edge[to] = id;
                ++children[v];
                ord[to] = low[to] = timer++;
                stack.push_back(to);
            }
        }
        for (int v = 0; v < V; ++v) {
            if (articulation[v]) articulation_points.push_back(v);
        }
    }
};
