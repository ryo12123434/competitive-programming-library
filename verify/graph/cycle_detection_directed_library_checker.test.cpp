#define PROBLEM "https://judge.yosupo.jp/problem/cycle_detection"
#include "../../library/graph/cycle_detection.hpp"
#include <iostream>
int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
        std::cin >> u >> v;
    auto c = find_cycle(n, edges, true);
    if (!c) {
        std::cout << -1 << '\n';
        return 0;
    }
    std::cout << c->edge_ids.size() << '\n';
    for (int id : c->edge_ids)
        std::cout << id << '\n';
}
