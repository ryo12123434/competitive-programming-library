#pragma once
#include "lowlink.hpp"
#include <cassert>
#include <vector>

/**
 * @brief Lowlinkのord/lowを再利用する二重頂点連結成分分解。橋を除く二重辺連結成分とは異なる。
 * @note componentsは頂点列、component_edgesは辺ID列。順序不定。孤立点も1頂点の成分を作る。
 * 自己ループは分解・関節点に影響させず、その頂点の最初の成分へ所属させる（独立したブロックにしない）。
 * 多重辺は辺IDで区別し、各辺はちょうど1成分に属する。関節点は複数成分に現れる。
 * @par Complexity 構築O(V)、追加償却O(1)、build O(V+E)時間・空間。反復DFSを使用。
 * @code
 * #include "library/graph/biconnected_components.hpp"
 * #include <cassert>
 * int main(){BiconnectedComponents b(3);b.add_edge(0,1);b.add_edge(1,2);b.build();assert(b.components.size()==2&&b.articulation_points==std::vector<int>{1});}
 * @endcode
 */
struct BiconnectedComponents {
    Lowlink lowlink;
    std::vector<std::vector<int>> components, component_edges;
    std::vector<int> articulation_points;
    /// @brief n頂点で構築。@pre n>=0。
    explicit BiconnectedComponents(int n) : lowlink(n) {}
    /// @brief 無向辺を追加して辺IDを返す。build結果を更新するには再buildする。頂点は[0,n)。
    int add_edge(int u, int v) { return lowlink.add_edge(u, v); }
    /// @brief 全成分を再計算する。lowlinkも再buildし、その公開結果を利用する。
    void build() {
        lowlink.build();
        articulation_points = lowlink.articulation_points;
        components.clear();
        component_edges.clear();
        int n = lowlink.V;
        std::vector<bool> seen(n);
        std::vector<int> parent(n, -1), parent_edge(n, -1), next(n), edge_stack, mark(n, -1);
        auto emit = [&](int stop) {
            int id = static_cast<int>(components.size());
            components.emplace_back();
            component_edges.emplace_back();
            while (true) {
                int e = edge_stack.back();
                edge_stack.pop_back();
                component_edges.back().push_back(e);
                auto [u, v] = lowlink.edges[e];
                for (int x : {u, v})
                    if (mark[x] != id) {
                        mark[x] = id;
                        components.back().push_back(x);
                    }
                if (e == stop)
                    break;
            }
        };
        for (int root = 0; root < n; ++root)
            if (!seen[root]) {
                std::vector<int> stack{root};
                seen[root] = true;
                bool isolated = true;
                while (!stack.empty()) {
                    int v = stack.back();
                    if (next[v] == static_cast<int>(lowlink.G[v].size())) {
                        stack.pop_back();
                        int p = parent[v];
                        if (p >= 0 && lowlink.low[v] >= lowlink.ord[p])
                            emit(parent_edge[v]);
                        continue;
                    }
                    auto [to, id] = lowlink.G[v][next[v]++];
                    if (to == v || id == parent_edge[v])
                        continue;
                    isolated = false;
                    if (!seen[to]) {
                        seen[to] = true;
                        parent[to] = v;
                        parent_edge[to] = id;
                        edge_stack.push_back(id);
                        stack.push_back(to);
                    } else if (lowlink.ord[to] < lowlink.ord[v])
                        edge_stack.push_back(id);
                }
                if (isolated) {
                    components.push_back({root});
                    component_edges.emplace_back();
                }
            }
        std::vector<int> first(n, -1);
        for (int i = 0; i < static_cast<int>(components.size()); ++i)
            for (int v : components[i])
                if (first[v] < 0)
                    first[v] = i;
        for (int id = 0; id < static_cast<int>(lowlink.edges.size()); ++id) {
            auto [u, v] = lowlink.edges[id];
            if (u == v)
                component_edges[first[u]].push_back(id);
        }
    }
};
