#define PROBLEM "https://judge.yosupo.jp/problem/minimum_spanning_tree"
#include "../../library/graph/minimum_spanning_tree.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    std::vector<MSTEdge> e(m);
    for (auto &edge : e)
        std::cin >> edge.from >> edge.to >> edge.cost;
    auto r = kruskal(n, e);
    std::cout << static_cast<long long>(r.total_weight) << '\n';
    for (int id : r.edge_ids)
        std::cout << id << ' ';
    std::cout << '\n';
}
