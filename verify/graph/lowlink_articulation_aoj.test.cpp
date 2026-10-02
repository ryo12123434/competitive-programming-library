#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_3_A"

#include <iostream>
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
    for (int v : graph.articulation_points) std::cout << v << '\n';
}
