#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_C"
#include "../../library/graph/warshall_floyd.hpp"
#include <iostream>
int main() {
    int n, m;
    std::cin >> n >> m;
    WarshallFloyd graph(n);
    while (m--) {
        int u, v;
        long long w;
        std::cin >> u >> v >> w;
        graph.add_edge(u, v, w);
    }
    graph.solve();
    if (graph.has_negative_cycle()) {
        std::cout << "NEGATIVE CYCLE\n";
        return 0;
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (j)
                std::cout << ' ';
            auto d = graph.distance(i, j);
            if (d.state == WarshallFloyd::State::Unreachable)
                std::cout << "INF";
            else
                std::cout << static_cast<long long>(d.value);
        }
        std::cout << '\n';
    }
}
