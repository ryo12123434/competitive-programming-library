#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"

#include <iostream>
#include "../../library/graph/dijkstra.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m, s, t;
    std::cin >> n >> m >> s >> t;
    Dijkstra graph(n);
    while (m--) {
        int u, v;
        long long w;
        std::cin >> u >> v >> w;
        graph.add_edge(u, v, w);
    }
    graph.solve_dijk(s);
    if (graph.get_dist(t) == Dijkstra::INF) {
        std::cout << -1 << '\n';
        return 0;
    }
    auto path = graph.get_path(t);
    std::cout << graph.get_dist(t) << ' ' << path.size() - 1 << '\n';
    for (int i = 1; i < static_cast<int>(path.size()); ++i)
        std::cout << path[i - 1] << ' ' << path[i] << '\n';
}
