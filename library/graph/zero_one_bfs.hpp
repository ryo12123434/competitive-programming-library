#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 重み 0 または 1 の有向グラフの最短路を 0-1 BFS で求める。
 * @note 頂点は 0-indexed。無向辺は両方向に追加する。
 */
struct ZeroOneBFS {
    struct Edge { int to, cost; };
    /// @brief 到達不能を表す距離。
    static constexpr int INF = std::numeric_limits<int>::max();
    int V;
    std::vector<std::vector<Edge>> G;
    std::vector<int> dis, prev;

    /// @brief V 頂点のグラフを O(V) で構築する。@pre V >= 0。
    explicit ZeroOneBFS(int V) : V(V) {
        assert(V >= 0);
        G.resize(V);
    }

    /// @brief 有向辺を償却 O(1) で追加する。@pre 端点は [0, V)、cost は 0 または 1。
    void add_edge(int from, int to, int cost) {
        assert(0 <= from && from < V && 0 <= to && to < V);
        assert(cost == 0 || cost == 1);
        G[from].push_back({to, cost});
    }

    /**
     * @brief s からの距離と直前の頂点を毎回初期化して求める。
     * @pre 0 <= s < V。
     * @par 計算量 O(V + E) 時間、グラフを除く追加空間 O(V + E)。
     */
    void solve(int s) {
        assert(0 <= s && s < V);
        dis.assign(V, INF);
        prev.assign(V, -1);
        std::deque<std::pair<int, int>> queue;
        dis[s] = 0;
        queue.emplace_back(0, s);
        while (!queue.empty()) {
            auto [d, v] = queue.front();
            queue.pop_front();
            if (d != dis[v]) continue;
            for (const auto& e : G[v]) {
                int next = d + e.cost;
                if (next >= dis[e.to]) continue;
                dis[e.to] = next;
                prev[e.to] = v;
                if (e.cost == 0) queue.emplace_front(next, e.to);
                else queue.emplace_back(next, e.to);
            }
        }
    }

    /// @brief t までの距離を O(1) で返す。到達不能なら INF。@pre 最後の辺追加後に solve 実行済み。
    int get_dist(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        return dis[t];
    }

    /// @brief 最短路の頂点列を O(V) で返す。到達不能なら空。@pre 最後の辺追加後に solve 実行済み。
    std::vector<int> get_path(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        if (dis[t] == INF) return {};
        std::vector<int> path;
        for (int v = t; v != -1; v = prev[v]) path.push_back(v);
        std::reverse(path.begin(), path.end());
        return path;
    }
};
