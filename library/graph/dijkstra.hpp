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
 *
 * @code
 * #include "library/graph/dijkstra.hpp"
 * #include <cassert>
 * int main() {
 *     Dijkstra graph(3);
 *     graph.add_edge(0, 1, 1);
 *     graph.solve_dijk(0);
 *     assert(graph.get_dist(1) == 1);
 *     assert(graph.get_dist(2) == Dijkstra::INF);
 *     assert((graph.get_path(1) == std::vector<int>{0, 1}));
 * }
 * @endcode
 */
struct Dijkstra {
    /**
     * @brief 行き先と非負の重みを格納する有向辺。
     * @note to は行き先（0-indexed）、cost は辺の重み。
     * @code
     * #include "library/graph/dijkstra.hpp"
     * #include <cassert>
     * int main() {
     *     Dijkstra::Edge edge{1, 1};
     *     assert(edge.to == 1);
     * }
     * @endcode
     */
    struct Edge { int to; long long cost; };
    using P = std::pair<long long, int>;
    /// @brief 到達不能を表す距離。有効な有限距離にはこの値を使わない。
    static constexpr long long INF = std::numeric_limits<long long>::max();
    int V;
    std::vector<std::vector<Edge>> G;
    std::vector<long long> dis;
    std::vector<int> prev;

    /**
     * @brief V 頂点のグラフを O(V) で構築する。
     * @pre V >= 0。
     * @param V 頂点数。頂点番号は [0, V)。
     */
    explicit Dijkstra(int V) : V(V), G(V) { assert(V >= 0); }

    /**
     * @brief 有向辺を償却 O(1) で追加する。
     * @pre cost >= 0。
     * @param from 始点（0-indexed）。
     * @param to 終点（0-indexed）。
     * @param cost 有向辺の重み。非負の long long。
     */
    void add_edge(int from, int to, long long cost) {
        assert(0 <= from && from < V && 0 <= to && to < V && cost >= 0);
        G[from].push_back({to, cost});
    }

    /**
     * @brief s からの距離と直前の頂点を O(V + E log(E + 1)) で求める。
     * @param s 始点（0-indexed）。
     * @pre 0 <= s < V。
     * @note 結果は dis に距離、prev に直前の頂点を格納する。始点・到達不能の prev は -1。
     *
     * @par Complexity
     * O(V + E log(E + 1)) 時間、追加空間 O(V + E)。
     */
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

    /**
     * @brief t までの距離を O(1) で返す。到達不能なら INF。
     * @pre 最後の辺追加後に solve_dijk 実行済み。
     * @param t 終点（0-indexed）。
     * @return 最短距離。到達不能は INF、始点自身は 0。
     */
    long long get_dist(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        return dis[t];
    }

    /**
     * @brief 最短路の頂点列を O(V) で返す。到達不能なら空配列。
     * @pre 最後の辺追加後に solve_dijk 実行済み。
     * @param t 終点（0-indexed）。
     * @return 始点から t への最短路の頂点列。到達不能は空列、始点自身は始点一つの列。
     * @note 複数の最短路がある場合はその一つを返す。追加空間は経路長に比例。
     */
    std::vector<int> get_path(int t) const {
        assert(0 <= t && t < V && static_cast<int>(dis.size()) == V);
        if (dis[t] == INF) return {};
        std::vector<int> path;
        for (int v = t; v != -1; v = prev[v]) path.push_back(v);
        std::reverse(path.begin(), path.end());
        return path;
    }
};
