#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// LCA を親の逐次走査、0-1 BFS を Floyd-Warshall、Lowlink を削除後の連結成分数と比較する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <utility>
#include <vector>
#include "../../library/graph/lca.hpp"
#include "../../library/graph/zero_one_bfs.hpp"
#include "../../library/graph/lowlink.hpp"

void check_lca(const std::vector<std::vector<int>>& tree, int root) {
    int n = static_cast<int>(tree.size());
    LCA lca(tree, root);
    std::vector<int> parent(n, -1), depth(n, -1);
    std::queue<int> queue;
    queue.push(root);
    depth[root] = 0;
    while (!queue.empty()) {
        int v = queue.front();
        queue.pop();
        for (int to : tree[v]) if (depth[to] == -1) {
            depth[to] = depth[v] + 1;
            parent[to] = v;
            queue.push(to);
        }
    }
    for (int v = 0; v < n; ++v) {
        assert(lca.parent(v) == parent[v] && lca.depth(v) == depth[v]);
        int ancestor = v;
        for (int k = 0; k <= depth[v] + 1; ++k) {
            assert(lca.kth_ancestor(v, k) == ancestor);
            if (ancestor != -1) ancestor = parent[ancestor];
        }
        assert(lca.kth_ancestor(v, std::numeric_limits<unsigned long long>::max()) == -1);
        for (int u = 0; u < n; ++u) {
            int x = u, y = v;
            while (depth[x] > depth[y]) x = parent[x];
            while (depth[y] > depth[x]) y = parent[y];
            while (x != y) { x = parent[x]; y = parent[y]; }
            assert(lca.query(u, v) == x);
            assert(lca.distance(u, v) == depth[u] + depth[v] - 2 * depth[x]);
        }
    }
}

void check_bfs(ZeroOneBFS& graph) {
    int n = graph.V;
    constexpr int INF = 1000000;
    std::vector<std::vector<int>> distance(n, std::vector<int>(n, INF));
    for (int v = 0; v < n; ++v) {
        distance[v][v] = 0;
        for (auto e : graph.G[v]) distance[v][e.to] = std::min(distance[v][e.to], e.cost);
    }
    for (int k = 0; k < n; ++k)
        for (int u = 0; u < n; ++u)
            for (int v = 0; v < n; ++v)
                distance[u][v] = std::min(distance[u][v], distance[u][k] + distance[k][v]);
    for (int s = 0; s < n; ++s) {
        graph.solve(s);
        for (int t = 0; t < n; ++t) {
            auto path = graph.get_path(t);
            if (distance[s][t] == INF) {
                assert(graph.get_dist(t) == ZeroOneBFS::INF && path.empty());
                continue;
            }
            assert(graph.get_dist(t) == distance[s][t]);
            assert(path.front() == s && path.back() == t && static_cast<int>(path.size()) <= n);
            int cost = 0;
            for (int i = 1; i < static_cast<int>(path.size()); ++i) {
                int edge_cost = INF;
                for (auto e : graph.G[path[i - 1]]) if (e.to == path[i]) edge_cost = std::min(edge_cost, e.cost);
                assert(edge_cost != INF);
                cost += edge_cost;
            }
            assert(cost == distance[s][t]);
        }
    }
}

int components(const Lowlink& graph, int removed_vertex = -1, int removed_edge = -1) {
    std::vector<bool> seen(graph.V, false);
    int count = 0;
    for (int root = 0; root < graph.V; ++root) {
        if (root == removed_vertex || seen[root]) continue;
        ++count;
        std::vector<int> stack{root};
        seen[root] = true;
        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            for (auto e : graph.G[v]) {
                if (e.id == removed_edge || e.to == removed_vertex || seen[e.to]) continue;
                seen[e.to] = true;
                stack.push_back(e.to);
            }
        }
    }
    return count;
}

void check_lowlink(Lowlink& graph) {
    int base = components(graph);
    std::vector<int> articulation, bridges;
    for (int v = 0; v < graph.V; ++v) if (components(graph, v) > base) articulation.push_back(v);
    for (int id = 0; id < static_cast<int>(graph.edges.size()); ++id)
        if (components(graph, -1, id) > base) bridges.push_back(id);
    for (int repeat = 0; repeat < 2; ++repeat) {
        graph.build();
        assert(graph.articulation_points == articulation);
        auto actual = graph.bridges;
        std::sort(actual.begin(), actual.end());
        assert(actual == bridges);
        auto order = graph.ord;
        std::sort(order.begin(), order.end());
        for (int v = 0; v < graph.V; ++v) {
            assert(order[v] == v);
            assert(0 <= graph.low[v] && graph.low[v] <= graph.ord[v]);
        }
    }
}

int main() {
    check_lca({{}}, 0);
    std::mt19937 rng(1708);
    for (int trial = 0; trial < 150; ++trial) {
        int n = 1 + rng() % 30;
        std::vector<std::vector<int>> tree(n);
        for (int v = 1; v < n; ++v) {
            int p = rng() % v;
            tree[p].push_back(v);
            tree[v].push_back(p);
        }
        check_lca(tree, rng() % n);

        ZeroOneBFS graph(1 + rng() % 15);
        for (int u = 0; u < graph.V; ++u)
            for (int v = 0; v < graph.V; ++v)
                if (rng() % 4 == 0) graph.add_edge(u, v, rng() % 2);
        check_bfs(graph);
        graph.add_edge(0, graph.V - 1, 0);
        graph.add_edge(0, graph.V - 1, 1);
        check_bfs(graph);
    }
    ZeroOneBFS empty_bfs(0);
    assert(empty_bfs.G.empty());
    ZeroOneBFS cycle(4);
    cycle.add_edge(0, 1, 1);
    cycle.add_edge(0, 2, 0);
    cycle.add_edge(2, 1, 0);
    cycle.add_edge(1, 2, 0);
    check_bfs(cycle);

    Lowlink empty(0);
    check_lowlink(empty);
    Lowlink parallel(4);
    parallel.add_edge(0, 0);
    parallel.add_edge(0, 1);
    parallel.add_edge(0, 1);
    parallel.add_edge(1, 2);
    check_lowlink(parallel);
    for (int trial = 0; trial < 300; ++trial) {
        Lowlink graph(1 + rng() % 9);
        int m = rng() % 25;
        for (int id = 0; id < m; ++id) {
            int u = rng() % graph.V, v = rng() % graph.V;
            assert(graph.add_edge(u, v) == id);
        }
        check_lowlink(graph);
        graph.add_edge(0, graph.V - 1);
        check_lowlink(graph);
    }

    // 長い鎖でも再帰スタックに依存しないことを確認する。
    constexpr int N = 200000;
    std::vector<std::vector<int>> chain(N);
    Lowlink graph(N);
    for (int v = 1; v < N; ++v) {
        chain[v - 1].push_back(v);
        chain[v].push_back(v - 1);
        graph.add_edge(v - 1, v);
    }
    LCA lca(chain, N - 1);
    assert(lca.query(0, N / 2) == N / 2);
    assert(lca.distance(0, N - 1) == N - 1);
    assert(lca.kth_ancestor(0, N - 1) == N - 1);
    assert(lca.kth_ancestor(0, N) == -1);
    graph.build();
    assert(static_cast<int>(graph.bridges.size()) == N - 1);
    assert(static_cast<int>(graph.articulation_points.size()) == N - 2);
    for (int v = 1; v < N - 1; ++v) assert(graph.articulation_points[v - 1] == v);

    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
