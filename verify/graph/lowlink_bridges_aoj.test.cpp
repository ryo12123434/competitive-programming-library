#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_3_B"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include "../../library/graph/lowlink.hpp"

int main() {
    int n, m;
    std::cin >> n >> m;
    Lowlink graph(n);
    while (m--) {
        int u, v;
        std::cin >> u >> v;
        graph.add_edge(u, v);
    }
    graph.build();
    std::vector<std::pair<int, int>> bridges;
    for (int id : graph.bridges) {
        auto [u, v] = graph.edges[id];
        if (u > v) std::swap(u, v);
        bridges.emplace_back(u, v);
    }
    std::sort(bridges.begin(), bridges.end());
    for (auto [u, v] : bridges) std::cout << u << ' ' << v << '\n';
}
