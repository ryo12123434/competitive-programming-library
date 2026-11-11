#define PROBLEM "https://judge.yosupo.jp/problem/bipartitematching"
#include "../../library/graph/bipartite_matching.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int l, r, m;
    std::cin >> l >> r >> m;
    BipartiteMatching g(l, r);
    while (m--) {
        int u, v;
        std::cin >> u >> v;
        g.add_edge(u, v);
    }
    std::cout << g.solve() << "\n";
    for (auto [u, v] : g.matching())
        std::cout << u << " " << v << "\n";
}
