#pragma once

#include "lowlink.hpp"
#include <cassert>
#include <vector>

/**
 * @brief Lowlink の橋を除いて2辺連結成分へ分解し、縮約した橋の森を作る。
 * @note component[v] が成分ID、groups[id] が頂点列、forest[id] が隣接成分と元の辺ID。
 * 非連結・多重辺・自己ループ・空グラフに対応。ID は頂点番号の小さい未訪問成分から付ける。
 * @pre Lowlink に最後の辺を追加した後、build() 済みであること。
 * @par Complexity
 * 構築 O(V+E) 時間・作業空間。結果の保持 O(V+橋数)。Lowlink 自体は再計算しない。
 * @code
 * #include "library/graph/two_edge_connected_components.hpp"
 * #include <cassert>
 * int main() {
 *     Lowlink graph(3); graph.add_edge(0,1); graph.add_edge(0,1); graph.add_edge(1,2); graph.build();
 *     TwoEdgeConnectedComponents result(graph);
 *     assert(result.component[0]==result.component[1] && result.component[1]!=result.component[2]);
 * }
 * @endcode
 */
class TwoEdgeConnectedComponents {
public:
    /// @brief 橋の森の隣接辺。to は成分ID、edge_id は Lowlink の元の辺ID。
    struct BridgeEdge { int to, edge_id; };
    std::vector<int> component;
    std::vector<std::vector<int>> groups;
    std::vector<std::vector<BridgeEdge>> forest;
    /// @brief build 済み Lowlink から独立した結果を構築する。
    explicit TwoEdgeConnectedComponents(const Lowlink& graph) : component(graph.V, -1) {
        assert(graph.ord.size() == static_cast<std::size_t>(graph.V));
        std::vector<bool> bridge(graph.edges.size());
        for (int id : graph.bridges) bridge[id] = true;
        for (int start = 0; start < graph.V; ++start) {
            if (component[start] != -1) continue;
            int id = static_cast<int>(groups.size()); groups.emplace_back();
            std::vector<int> stack{start}; component[start] = id;
            while (!stack.empty()) {
                int v = stack.back(); stack.pop_back(); groups[id].push_back(v);
                for (auto edge : graph.G[v]) if (!bridge[edge.id] && component[edge.to] == -1) {
                    component[edge.to] = id; stack.push_back(edge.to);
                }
            }
        }
        forest.resize(groups.size());
        for (int id : graph.bridges) {
            auto [u, v] = graph.edges[id]; int a = component[u], b = component[v];
            forest[a].push_back({b, id}); forest[b].push_back({a, id});
        }
    }
    /// @brief 成分数。O(1)。
    int component_count() const { return static_cast<int>(groups.size()); }
};
