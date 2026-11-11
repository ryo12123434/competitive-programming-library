#define PROBLEM "https://judge.yosupo.jp/problem/biconnected_components"
#include "../../library/graph/biconnected_components.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    BiconnectedComponents g(n);
    while (m--) {
        int u, v;
        std::cin >> u >> v;
        g.add_edge(u, v);
    }
    g.build();
    std::cout << g.components.size() << "\n";
    for (auto &c : g.components) {
        std::cout << c.size();
        for (int v : c)
            std::cout << " " << v;
        std::cout << "\n";
    }
}
