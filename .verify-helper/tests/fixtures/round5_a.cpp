#include "library/graph/directed_mst.hpp"
#include "library/graph/steiner_tree.hpp"
#include "library/graph/complement_graph.hpp"
#include "library/graph/range_edge_graph.hpp"

#include <algorithm>
#include <climits>
#include <functional>
#include <numeric>
#include <random>
#include <set>
using W = __int128_t;
void mst() {
    std::mt19937 rng(583);
    assert(directed_mst(0, -1, {})->cost == 0);
    for (int t = 0; t < 2500; ++t) {
        int n = 1 + rng() % 6, root = rng() % n;
        std::vector<DirectedMSTEdge> edges;
        for (int m = rng() % 16; m--;)
            edges.push_back({int(rng() % n), int(rng() % n), int(rng() % 21) - 10});
        std::vector<std::vector<int>> incoming(n);
        for (int i = 0; i < int(edges.size()); ++i)
            if (edges[i].from != edges[i].to)
                incoming[edges[i].to].push_back(i);
        std::optional<W> expected;
        std::vector<int> choice(n, -1);
        auto search = [&](auto &&self, int v) -> void {
            if (v == n) {
                std::vector<bool> seen(n);
                seen[root] = true;
                for (int k = 0; k < n; ++k)
                    for (int x = 0; x < n; ++x)
                        if (x != root && seen[edges[choice[x]].from])
                            seen[x] = true;
                if (std::count(seen.begin(), seen.end(), true) != n)
                    return;
                W cost = 0;
                for (int x = 0; x < n; ++x)
                    if (x != root)
                        cost += edges[choice[x]].cost;
                if (!expected || cost < *expected)
                    expected = cost;
                return;
            }
            if (v == root) {
                self(self, v + 1);
                return;
            }
            for (int id : incoming[v]) {
                choice[v] = id;
                self(self, v + 1);
            }
        };
        search(search, 0);
        auto got = directed_mst(n, root, edges);
        assert(bool(got) == bool(expected));
        if (got) {
            assert(got->cost == *expected && got->edge_ids.size() == std::size_t(n - 1));
            W cost = 0;
            std::vector<bool> seen(n);
            seen[root] = true;
            assert(got->parent_edge[root] == -1 && got->parent[root] == -1);
            for (int v = 0; v < n; ++v)
                if (v != root) {
                    int id = got->parent_edge[v];
                    assert(edges[id].to == v && edges[id].from == got->parent[v]);
                    cost += edges[id].cost;
                }
            for (int k = 0; k < n; ++k)
                for (int v = 0; v < n; ++v)
                    if (v != root && seen[got->parent[v]])
                        seen[v] = true;
            assert(cost == got->cost && std::count(seen.begin(), seen.end(), true) == n);
        }
    }
    auto extreme = directed_mst(3, 0, {{0, 1, LLONG_MIN}, {1, 2, LLONG_MIN}, {2, 1, LLONG_MAX}});
    assert(extreme && extreme->cost == W(LLONG_MIN) * 2);
    int n = 100000;
    std::vector<DirectedMSTEdge> edges;
    for (int i = 1; i < n; ++i)
        edges.push_back({i - 1, i, -1});
    assert(directed_mst(n, 0, edges)->cost == -(n - 1));
}
void steiner() {
    std::mt19937 rng(53);
    for (int t = 0; t < 1000; ++t) {
        int n = 1 + rng() % 7, m = rng() % 11;
        std::vector<SteinerTreeEdge> edges;
        for (int i = 0; i < m; ++i)
            edges.push_back({int(rng() % n), int(rng() % n), int(rng() % 11)});
        std::vector<int> term;
        for (int i = rng() % (n + 2); i--;)
            term.push_back(rng() % n);
        std::optional<W> expected;
        for (int mask = 0; mask < (1 << m); ++mask) {
            std::vector<int> p(n);
            std::iota(p.begin(), p.end(), 0);
            auto root = [&](int v) {
                while (p[v] != v)
                    v = p[v];
                return v;
            };
            W sum = 0;
            for (int i = 0; i < m; ++i)
                if (mask >> i & 1) {
                    p[root(edges[i].u)] = root(edges[i].v);
                    sum += edges[i].cost;
                }
            bool ok = true;
            for (int v : term)
                ok &= root(v) == root(term[0]);
            if (ok && (!expected || sum < *expected))
                expected = sum;
        }
        assert(steiner_tree(n, edges, term) == expected);
        auto restored = steiner_tree_with_edges(n, edges, term);
        assert(bool(restored) == bool(expected));
        if (restored) {
            W sum = 0;
            std::vector<int> p(n);
            std::iota(p.begin(), p.end(), 0);
            auto root = [&](int x) {
                while (p[x] != x)
                    x = p[x];
                return x;
            };
            std::set<int> ids;
            for (int id : restored->edge_ids) {
                assert(ids.insert(id).second);
                auto e = edges[id];
                p[root(e.u)] = root(e.v);
                sum += e.cost;
            }
            assert(sum == restored->cost && sum == *expected);
            for (int v : term)
                assert(root(v) == root(term[0]));
        }
    }
    assert(steiner_tree(0, {}, {}).value() == 0);
    assert(steiner_tree(4, {{0, 3, LLONG_MAX}, {1, 3, LLONG_MAX}, {2, 3, LLONG_MAX}}, {0, 1, 2}).value() ==
           W(LLONG_MAX) * 3);
    int n = 400;
    std::vector<SteinerTreeEdge> edges;
    for (int i = 1; i < n; ++i)
        edges.push_back({i - 1, i, 1});
    assert(steiner_tree(n, edges, {0, 50, 100, 150, 200, 250, 300, 399}) == 399);
}
void complement() {
    std::mt19937 rng(584);
    assert(ComplementGraph(0, {}).components().empty());
    for (int t = 0; t < 500; ++t) {
        int n = 1 + rng() % 40;
        std::vector<std::pair<int, int>> edges;
        std::vector<std::vector<bool>> a(n, std::vector<bool>(n));
        for (int m = rng() % (n * n + 1); m--;) {
            int u = rng() % n, v = rng() % n;
            edges.emplace_back(u, v);
            a[u][v] = a[v][u] = true;
        }
        ComplementGraph g(n, edges);
        for (int s = 0; s < n; ++s) {
            std::vector<int> d(n, -1), q{s};
            d[s] = 0;
            for (std::size_t i = 0; i < q.size(); ++i)
                for (int v = 0; v < n; ++v)
                    if (v != q[i] && !a[q[i]][v] && d[v] < 0) {
                        d[v] = d[q[i]] + 1;
                        q.push_back(v);
                    }
            auto r = g.bfs(s);
            assert(r.distance == d);
            for (int v = 0; v < n; ++v)
                if (v != s && d[v] >= 0)
                    assert(!a[v][r.parent[v]] && d[r.parent[v]] + 1 == d[v]);
        }
        auto groups = g.components();
        std::vector<int> seen(n);
        for (auto &c : groups) {
            auto d = g.bfs(c[0]).distance;
            for (int v : c) {
                assert(!seen[v]++ && d[v] >= 0);
            }
            assert(int(c.size()) == n - std::count(d.begin(), d.end(), -1));
        }
        assert(std::count(seen.begin(), seen.end(), 1) == n);
    }
    assert(ComplementGraph(200000, {}).components().front().size() == 200000);
    std::vector<std::pair<int, int>> all;
    for (int i = 0; i < 1200; ++i)
        for (int j = 0; j < i; ++j)
            all.emplace_back(i, j);
    assert(ComplementGraph(1200, all).components().size() == 1200);
}
void range_graph() {
    std::mt19937 rng(639);
    assert(RangeEdgeGraph(0).expanded_graph().V == 0);
    for (int t = 0; t < 300; ++t) {
        int n = 1 + rng() % 20;
        RangeEdgeGraph g(n);
        const long long inf = 1LL << 50;
        std::vector<std::vector<long long>> d(n, std::vector<long long>(n, inf));
        for (int i = 0; i < n; ++i)
            d[i][i] = 0;
        for (int q = 0; q < 80; ++q) {
            int type = rng() % 4, a = rng() % n, b = rng() % n, l = rng() % (n + 1), r = rng() % (n + 1),
                l2 = rng() % (n + 1), r2 = rng() % (n + 1);
            if (l > r)
                std::swap(l, r);
            if (l2 > r2)
                std::swap(l2, r2);
            long long w = rng() % 20;
            if (type == 0) {
                g.add_edge(a, b, w);
                d[a][b] = std::min(d[a][b], w);
            }
            if (type == 1) {
                g.add_point_to_range(a, l, r, w);
                for (int i = l; i < r; ++i)
                    d[a][i] = std::min(d[a][i], w);
            }
            if (type == 2) {
                g.add_range_to_point(l, r, b, w);
                for (int i = l; i < r; ++i)
                    d[i][b] = std::min(d[i][b], w);
            }
            if (type == 3) {
                g.add_range_to_range(l, r, l2, r2, w);
                for (int i = l; i < r; ++i)
                    for (int j = l2; j < r2; ++j)
                        d[i][j] = std::min(d[i][j], w);
            }
        }
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
        for (int s = 0; s < n; ++s) {
            auto got = g.distances(s);
            for (int v = 0; v < n; ++v)
                assert(got[v] == (d[s][v] == inf ? Dijkstra::INF : d[s][v]));
        }
    }
    RangeEdgeGraph large(20000);
    for (int i = 0; i < 2000; ++i)
        large.add_range_to_range(0, 20000, 0, 20000, 1);
    auto d = large.distances(0);
    assert(d.size() == 20000 && d.back() == 1 && large.expanded_graph().V == 3 * 20000 - 2 + 2000);
}
int main() {
    mst();
    steiner();
    complement();
    range_graph();
}
