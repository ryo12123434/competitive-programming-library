#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 非負の重みを持つ有向グラフの最短路を Dijkstra 法で求める。
 * @pre 頂点番号は [0, V)。有限の最短距離は INF 未満であること。
 * @note 無向辺は両方向に追加する。solve_dijk は毎回結果を初期化する。
 */
struct Dijkstra {
    /// @brief 行き先と非負の重みを格納する有向辺。
    struct Edge { int to; long long cost; };
    using P = std::pair<long long, int>;
    /// @brief 到達不能を表す距離。有効な有限距離にはこの値を使わない。
    static constexpr long long INF = std::numeric_limits<long long>::max();
    int V;
    std::vector<std::vector<Edge>> G;
    std::vector<long long> dis;
    std::vector<int> prev;

    /// @brief V 頂点のグラフを O(V) で構築する。@pre V >= 0。
    explicit Dijkstra(int V) : V(V), G(V) { assert(V >= 0); }

    /// @brief 有向辺を償却 O(1) で追加する。@pre cost >= 0。
    void add_edge(int from, int to, long long cost) {
        assert(0 <= from && from < V && 0 <= to && to < V && cost >= 0);
        G[from].push_back({to, cost});
    }

    /// @brief s からの距離と直前の頂点を O(V + E log(E + 1)) で求める。
    void solve_dijk(int s) {
        assert(0 <= s && s < V);
        dis.assign(V, INF);
        prev.assign(V, -1);
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        dis[s] = 0;
        pq.emplace(0, s);
        while (!pq.empty()) {
            auto [d, v] = pq.top();
            pq.pop();
            if (d != dis[v]) continue;
            for (const auto& e : G[v]) {
                if (e.cost >= INF - d) continue;
                if (d + e.cost < dis[e.to]) {
                    dis[e.to] = d + e.cost;
                    prev[e.to] = v;
                    pq.emplace(dis[e.to], e.to);
                }
            }
        }
    }

    /// @brief t までの距離を O(1) で返す。到達不能なら INF。@pre 最後の辺追加後に solve_dijk 実行済み。
    long long get_dist(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        return dis[t];
    }

    /// @brief 最短路の頂点列を O(V) で返す。到達不能なら空配列。@pre 最後の辺追加後に solve_dijk 実行済み。
    std::vector<int> get_path(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        if (dis[t] == INF) return {};
        std::vector<int> path;
        for (int v = t; v != -1; v = prev[v]) path.push_back(v);
        std::reverse(path.begin(), path.end());
        return path;
    }
};
