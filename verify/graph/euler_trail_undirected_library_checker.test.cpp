#define PROBLEM "https://judge.yosupo.jp/problem/eulerian_trail_undirected"
#include "../../library/graph/euler_trail.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int t;
    std::cin >> t;
    while (t--) {
        int n, m;
        std::cin >> n >> m;
        std::vector<std::pair<int, int>> edges(m);
        for (auto &[u, v] : edges)
            std::cin >> u >> v;
        auto r = euler_trail(n, edges, false);
        if (!r) {
            std::cout << "No\n";
            continue;
        }
        std::cout << "Yes\n";
        for (int v : r->vertices)
            std::cout << v << ' ';
        std::cout << '\n';
        for (int id : r->edge_ids)
            std::cout << id << ' ';
        std::cout << '\n';
    }
}
