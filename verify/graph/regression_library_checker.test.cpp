#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// A + B を入出力用に使い、各 API の境界条件とランダムケースを assert で検証する。
#include <cassert>
#include <iostream>
#include <random>
#include "../../library/graph/dijkstra.hpp"
#include "../../library/graph/bellman_ford.hpp"
#include "../../library/graph/topological_sort.hpp"

int main() {
    std::vector<long long> dis(4, -100);
    Edges edges{{0, 1, 5}, {1, 2, -2}};
    assert(!bellman_ford(edges, 4, 0, dis));
    assert((dis == std::vector<long long>{0, 5, 3, BELLMAN_FORD_INF}));
    assert(!bellman_ford({}, 4, 3, dis));
    assert((dis == std::vector<long long>{BELLMAN_FORD_INF, BELLMAN_FORD_INF, BELLMAN_FORD_INF, 0}));
    assert(!bellman_ford({{1, 1, -1}}, 2, 0, dis));
    assert(bellman_ford({{0, 0, -1}}, 1, 0, dis));
    assert(bellman_ford({{0, 1, 2}, {1, 2, -3}, {2, 1, 1}}, 3, 0, dis));
    assert(!bellman_ford({}, 1, 0, dis) && dis[0] == 0);

    Dijkstra large(3);
    large.add_edge(0, 1, Dijkstra::INF - 10);
    large.add_edge(1, 2, 20);
    large.add_edge(0, 2, 7);
    large.solve_dijk(0);
    assert(large.get_dist(1) == Dijkstra::INF - 10 && large.get_dist(2) == 7);
    large.solve_dijk(2);
    assert(large.get_path(0).empty());
    assert((large.get_path(2) == std::vector<int>{2}));

    std::mt19937 rng(1701);
    for (int trial = 0; trial < 300; ++trial) {
        int n = 1 + rng() % 10;
        Dijkstra graph(n);
        Edges es;
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) {
            if (rng() % 4 == 0) {
                long long w = rng() % 20;
                graph.add_edge(u, v, w);
                es.push_back({u, v, w});
            }
        }
        for (int s = 0; s < n; ++s) {
            graph.solve_dijk(s);
            assert(!bellman_ford(es, n, s, dis));
            assert(graph.dis == dis);
            for (int t = 0; t < n; ++t) {
                auto path = graph.get_path(t);
                if (dis[t] == Dijkstra::INF) { assert(path.empty()); continue; }
                assert(path.front() == s && path.back() == t && path.size() <= static_cast<std::size_t>(n));
                for (int i = 1; i < static_cast<int>(path.size()); ++i) {
                    bool valid = false;
                    for (const auto& e : graph.G[path[i - 1]])
                        if (e.to == path[i] && dis[path[i - 1]] + e.cost == dis[path[i]]) valid = true;
                    assert(valid);
                }
            }
        }
        std::vector<std::vector<int>> dag(n);
        std::vector<int> indegree(n, 0);
        for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
            if (rng() % 3 == 0) { dag[u].push_back(v); ++indegree[v]; }
        auto order = topological_sort(dag, indegree, n);
        assert(static_cast<int>(order.size()) == n);
        std::vector<int> pos(n);
        for (int i = 0; i < n; ++i) pos[order[i]] = i;
        for (int u = 0; u < n; ++u) for (int v : dag[u]) assert(pos[u] < pos[v]);
    }
    std::vector<int> degree{1, 1, 0};
    assert((topological_sort({{1}, {0}, {}}, degree, 3) == std::vector<int>{2}));
    degree.clear();
    assert(topological_sort({}, degree, 0).empty());
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
