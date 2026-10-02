#define PROBLEM "https://judge.yosupo.jp/problem/two_edge_connected_components"

#include <iostream>
#include <vector>
#include "../../library/graph/lowlink.hpp"
#include "../../library/data_structure/union_find.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    Lowlink graph(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        std::cin >> u >> v;
        graph.add_edge(u, v);
    }
    graph.build();
    std::vector<bool> bridge(m, false);
    for (int id : graph.bridges) bridge[id] = true;
    UnionFind uf(n);
    for (int id = 0; id < m; ++id) {
        if (!bridge[id]) uf.unite(graph.edges[id].first, graph.edges[id].second);
    }
    std::vector<std::vector<int>> components(n);
    for (int v = 0; v < n; ++v) components[uf.root(v)].push_back(v);
    int count = 0;
    for (const auto& component : components) count += !component.empty();
    std::cout << count << '\n';
    for (const auto& component : components) {
        if (component.empty()) continue;
        std::cout << component.size();
        for (int v : component) std::cout << ' ' << v;
        std::cout << '\n';
    }
}
