#pragma once

#include <cassert>
#include <limits>
#include <vector>

/// @brief Bellman-Ford 法で使う有向重み付き辺。頂点番号は 0-indexed。
struct Edge { long long from, to, cost; };
using Edges = std::vector<Edge>;
/// @brief 到達不能を表す距離。有効な有限距離にはこの値を使わない。
inline constexpr long long BELLMAN_FORD_INF = std::numeric_limits<long long>::max();

/**
 * @brief s からの最短距離を dis に格納し、到達可能な負閉路があれば true を返す。
 * @pre V > 0、0 <= s < V、各辺の端点は [0, V)。
 * @pre 辺数は int に収まること。負閉路がない場合、有限の最短距離は long long に収まり、INF 未満であること。
 * @note 中間距離は __int128 で計算する。正のオーバーフローや負閉路でのアンダーフローを避ける。
 * @note dis は毎回初期化する。到達不能な頂点の距離は BELLMAN_FORD_INF。
 * @note true の場合、dis は最短距離を表さない。
 * @par 計算量 O(VE + V) 時間、O(V) 空間。
 */
inline bool bellman_ford(const Edges& Es, int V, int s, std::vector<long long>& dis) {
    assert(V > 0 && 0 <= s && s < V);
    assert(Es.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    for (const auto& e : Es) assert(0 <= e.from && e.from < V && 0 <= e.to && e.to < V);
    dis.assign(V, BELLMAN_FORD_INF);
    dis[s] = 0;
    const __int128 wide_inf = static_cast<__int128>(1) << 126;
    std::vector<__int128> distance(V, wide_inf);
    distance[s] = 0;
    for (int i = 0; i < V - 1; ++i) {
        bool changed = false;
        for (const auto& e : Es) {
            if (distance[e.from] != wide_inf && distance[e.from] + e.cost < distance[e.to]) {
                distance[e.to] = distance[e.from] + e.cost;
                changed = true;
            }
        }
        if (!changed) break;
    }
    // V 回目にも緩和できれば、始点から到達可能な負閉路が存在する。
    for (const auto& e : Es) {
        if (distance[e.from] != wide_inf && distance[e.from] + e.cost < distance[e.to]) return true;
    }
    for (int v = 0; v < V; ++v) {
        if (distance[v] == wide_inf) continue;
        assert(std::numeric_limits<long long>::min() <= distance[v] && distance[v] < BELLMAN_FORD_INF);
        dis[v] = static_cast<long long>(distance[v]);
    }
    return false;
}
