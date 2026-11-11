#pragma once
#include <cassert>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

/// @brief 有向辺。辺IDは入力配列の添字。自己ループ・多重辺・負重みを許す。
struct DirectedMSTEdge {
    int from, to;
    long long cost;
};
/**
 * @brief 根付き有向最小全域木。parent[root]=parent_edge[root]=-1、辺IDは元の入力に対応する。
 * @note costはsigned 128bit。全頂点数がintに収まればlong long辺重みの総和・縮約計算も収まる。
 * @code
 * #include "library/graph/directed_mst.hpp"
 * #include <cassert>
 * int main(){auto r=directed_mst(3,0,{{0,1,4},{0,2,5},{1,2,-2}});assert(r&&r->cost==2&&r->parent[2]==1);}
 * @endcode
 */
struct DirectedMSTResult {
    __int128_t cost = 0;
    std::vector<int> parent, parent_edge, edge_ids;
};
/**
 * @brief Chu–Liu/Edmonds法。根から全頂点へ到達する有向木がなければnullopt。
 * @param n 頂点数（int）。0の場合はroot=-1、辺なしを指定し、空の木を返す。
 * @param root 根。n>0なら[0,n)。辺ID・辺数はintに収める。
 * @param edges 有向辺。根への辺と自己ループは木に採用しない。
 * @par Complexity O(VE+V)時間。復元用の縮約対応を保持するため最悪O(VE+V)空間。
 * 各縮約段階でO(V+E)、非自明な閉路を縮めるごとに頂点数が減る。
 */
inline std::optional<DirectedMSTResult> directed_mst(int n, int root,
                                                     const std::vector<DirectedMSTEdge> &edges) {
    assert(n >= 0);
    if (!n) {
        assert(root == -1 && edges.empty());
        return DirectedMSTResult{};
    }
    assert(0 <= root && root < n);
    struct Edge {
        int from, to;
        __int128_t cost;
    };
    struct Layer {
        std::vector<int> incoming, old_edge, old_target;
    };
    std::vector<Edge> graph;
    for (auto e : edges) {
        assert(0 <= e.from && e.from < n && 0 <= e.to && e.to < n);
        graph.push_back({e.from, e.to, e.cost});
    }
    std::vector<Layer> history;
    int count = n, current_root = root;
    std::vector<int> selected;
    while (true) {
        std::vector<int> in(count, -1);
        for (int id = 0; id < static_cast<int>(graph.size()); ++id) {
            auto e = graph[id];
            if (e.from != e.to && e.to != current_root && (in[e.to] < 0 || e.cost < graph[in[e.to]].cost))
                in[e.to] = id;
        }
        for (int v = 0; v < count; ++v)
            if (v != current_root && in[v] < 0)
                return std::nullopt;
        std::vector<int> component(count, -1), seen(count, -1);
        int cycles = 0;
        for (int start = 0; start < count; ++start) {
            int v = start;
            while (v != current_root && seen[v] < 0) {
                seen[v] = start;
                v = graph[in[v]].from;
            }
            if (v != current_root && seen[v] == start && component[v] < 0) {
                int u = v;
                do {
                    component[u] = cycles;
                    u = graph[in[u]].from;
                } while (u != v);
                ++cycles;
            }
        }
        if (!cycles) {
            selected = std::move(in);
            break;
        }
        int next_count = cycles;
        for (auto &c : component)
            if (c < 0)
                c = next_count++;
        Layer layer;
        layer.incoming = std::move(in);
        std::vector<Edge> next;
        for (int id = 0; id < static_cast<int>(graph.size()); ++id) {
            auto e = graph[id];
            int u = component[e.from], v = component[e.to];
            if (u == v || v == component[current_root])
                continue;
            next.push_back({u, v, e.cost - graph[layer.incoming[e.to]].cost});
            layer.old_edge.push_back(id);
            layer.old_target.push_back(e.to);
        }
        current_root = component[current_root];
        count = next_count;
        history.push_back(std::move(layer));
        graph = std::move(next);
    }
    while (!history.empty()) {
        auto layer = std::move(history.back());
        history.pop_back();
        auto restored = std::move(layer.incoming);
        for (int id : selected)
            if (id >= 0)
                restored[layer.old_target[id]] = layer.old_edge[id];
        selected = std::move(restored);
    }
    DirectedMSTResult result;
    result.parent.assign(n, -1);
    result.parent_edge = selected;
    for (int v = 0; v < n; ++v)
        if (v != root) {
            int id = selected[v];
            result.edge_ids.push_back(id);
            result.parent[v] = edges[id].from;
            result.cost += edges[id].cost;
        }
    return result;
}
