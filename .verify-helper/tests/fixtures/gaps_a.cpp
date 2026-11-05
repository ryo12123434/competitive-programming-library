#include "library/graph/minimum_spanning_tree.hpp"
#include "library/graph/cycle_detection.hpp"
#include "library/graph/euler_trail.hpp"
#include "library/graph/warshall_floyd.hpp"
#include "library/graph/namori.hpp"
#include "library/algorithm/cartesian_tree.hpp"
#include "library/algorithm/sliding_window_extrema.hpp"
#include <climits>
#include <random>
#include <string>
using Wide = __int128_t;
std::vector<int> labels(int n, const std::vector<std::pair<int, int>> &edges) {
    std::vector<int> c(n, -1);
    std::vector<std::vector<int>> g(n);
    for (auto [u, v] : edges) {
        g[u].push_back(v);
        g[v].push_back(u);
    }
    for (int v = 0; v < n; ++v)
        if (c[v] < 0) {
            std::vector<int> q{v};
            c[v] = v;
            for (std::size_t i = 0; i < q.size(); ++i)
                for (int to : g[q[i]])
                    if (c[to] < 0) {
                        c[to] = v;
                        q.push_back(to);
                    }
        }
    return c;
}
void mst() {
    std::mt19937 rng(224);
    for (int trial = 0; trial < 700; ++trial) {
        int n = trial % 6, m = n ? rng() % 9 : 0;
        std::vector<MSTEdge> edges;
        std::vector<std::pair<int, int>> uv;
        for (int i = 0; i < m; ++i) {
            int u = rng() % n, v = rng() % n;
            edges.push_back({u, v, static_cast<int>(rng() % 21) - 10});
            uv.emplace_back(u, v);
        }
        auto base = labels(n, uv);
        int components = std::set<int>(base.begin(), base.end()).size();
        std::optional<Wide> best;
        for (int mask = 0; mask < (1 << m); ++mask) {
            if (__builtin_popcount(static_cast<unsigned>(mask)) != n - components)
                continue;
            std::vector<std::pair<int, int>> selected;
            Wide cost = 0;
            for (int i = 0; i < m; ++i)
                if (mask >> i & 1) {
                    selected.push_back(uv[i]);
                    cost += edges[i].cost;
                }
            auto c = labels(n, selected);
            bool valid = true;
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    if ((c[u] == c[v]) != (base[u] == base[v]))
                        valid = false;
            if (valid && (!best || cost < *best))
                best = cost;
        }
        auto r = kruskal(n, edges);
        assert(best && r.total_weight == *best && r.components == components);
        Wide cost = 0;
        std::set<int> seen;
        std::vector<std::pair<int, int>> selected;
        for (int id : r.edge_ids) {
            assert(seen.insert(id).second);
            cost += edges[id].cost;
            selected.push_back(uv[id]);
        }
        assert(cost == r.total_weight && labels(n, selected) == base);
    }
    assert(kruskal(3, {{0, 1, LLONG_MAX}, {1, 2, LLONG_MAX}}).total_weight == Wide(LLONG_MAX) * 2);
}
bool naive_euler(int n, const std::vector<std::pair<int, int>> &e, bool directed, std::optional<int> start) {
    if (e.empty())
        return true;
    auto dfs = [&](auto &&self, int v, unsigned mask) -> bool {
        if (mask == (1u << e.size()) - 1)
            return true;
        for (std::size_t i = 0; i < e.size(); ++i)
            if (!(mask >> i & 1)) {
                auto [u, w] = e[i];
                if (u == v && self(self, w, mask | (1u << i)))
                    return true;
                if (!directed && w == v && self(self, u, mask | (1u << i)))
                    return true;
            }
        return false;
    };
    if (start)
        return dfs(dfs, *start, 0);
    for (int v = 0; v < n; ++v)
        if (dfs(dfs, v, 0))
            return true;
    return false;
}
void check_walk(const std::vector<int> &vertices, const std::vector<int> &ids,
                const std::vector<std::pair<int, int>> &e, bool directed, bool closed) {
    assert(vertices.size() == ids.size() + (closed ? 0 : 1));
    std::set<int> seen;
    for (std::size_t i = 0; i < ids.size(); ++i) {
        int id = ids[i];
        assert(0 <= id && id < static_cast<int>(e.size()) && seen.insert(id).second);
        auto [u, v] = e[id];
        int a = vertices[i], b = vertices[(i + 1) % vertices.size()];
        assert((u == a && v == b) || (!directed && u == b && v == a));
    }
}
void walks() {
    std::mt19937 rng(2233);
    for (int trial = 0; trial < 1200; ++trial) {
        int n = trial % 6, m = n ? rng() % 8 : 0;
        std::vector<std::pair<int, int>> e;
        for (int i = 0; i < m; ++i)
            e.emplace_back(rng() % n, rng() % n);
        for (bool directed : {false, true}) {
            bool cycle = false;
            if (directed) {
                std::vector<int> in(n);
                for (auto [u, v] : e) {
                    (void)u;
                    ++in[v];
                }
                std::vector<int> q;
                for (int i = 0; i < n; ++i)
                    if (!in[i])
                        q.push_back(i);
                for (std::size_t i = 0; i < q.size(); ++i)
                    for (auto [u, v] : e)
                        if (u == q[i] && !--in[v])
                            q.push_back(v);
                cycle = q.size() != static_cast<std::size_t>(n);
            } else {
                std::vector<std::pair<int, int>> past;
                for (auto edge : e) {
                    auto c = labels(n, past);
                    if (c[edge.first] == c[edge.second])
                        cycle = true;
                    past.push_back(edge);
                }
            }
            auto c = find_cycle(n, e, directed);
            assert(bool(c) == cycle);
            if (c)
                check_walk(c->vertices, c->edge_ids, e, directed, true);
            for (int start = -1; start < n; ++start) {
                std::optional<int> s;
                if (start >= 0)
                    s = start;
                auto r = euler_trail(n, e, directed, s);
                assert(bool(r) == naive_euler(n, e, directed, s));
                if (r && !e.empty()) {
                    check_walk(r->vertices, r->edge_ids, e, directed, false);
                    assert(r->edge_ids.size() == e.size());
                    if (s)
                        assert(r->vertices.front() == *s);
                } else if (r)
                    assert(r->vertices.size() == static_cast<std::size_t>(n ? 1 : 0));
            }
        }
        auto namori = decompose_namori(n, e);
        auto c = labels(n, e);
        bool valid = n > 0 && m == n && std::set<int>(c.begin(), c.end()).size() == 1;
        assert(bool(namori) == valid);
        if (namori) {
            check_walk(namori->cycle_vertices, namori->cycle_edge_ids, e, false, true);
            std::set<int> ids(namori->cycle_edge_ids.begin(), namori->cycle_edge_ids.end());
            std::vector<std::pair<int, int>> remaining;
            for (int i = 0; i < m; ++i)
                if (!ids.contains(i))
                    remaining.push_back(e[i]);
            auto groups = labels(n, remaining);
            for (int v = 0; v < n; ++v) {
                int root = namori->root[v];
                assert(groups[v] == groups[root] && namori->cycle_index[root] >= 0);
                assert((namori->cycle_index[v] >= 0) == (root == v));
            }
        }
    }
}
void floyd() {
    std::mt19937 rng(84922);
    for (int trial = 0; trial < 400; ++trial) {
        int n = trial % 8;
        WarshallFloyd g(n);
        std::vector<std::tuple<int, int, long long>> edges;
        for (int i = 0; i < (n ? 20 : 0); ++i) {
            int u = rng() % n, v = rng() % n;
            long long w = static_cast<int>(rng() % 19) - 7;
            if (trial % 3 == 0) {
                if (u > v)
                    std::swap(u, v);
                if (u == v)
                    w = std::abs(w);
            }
            edges.emplace_back(u, v, w);
            g.add_edge(u, v, w);
        }
        g.solve();
        g.solve();
        bool any_negative = false;
        for (int source = 0; source < n; ++source) {
            std::vector<std::optional<Wide>> d(n);
            d[source] = 0;
            std::vector<bool> bad(n);
            for (int t = 0; t < n; ++t)
                for (auto [u, v, w] : edges)
                    if (d[u] && (!d[v] || *d[u] + w < *d[v])) {
                        d[v] = *d[u] + w;
                        if (t == n - 1)
                            bad[v] = true;
                    }
            for (int t = 0; t < n; ++t)
                for (auto [u, v, w] : edges) {
                    (void)w;
                    if (bad[u])
                        bad[v] = true;
                }
            for (int v = 0; v < n; ++v) {
                auto r = g.distance(source, v);
                auto p = g.path(source, v);
                if (bad[v]) {
                    any_negative = true;
                    assert(r.state == WarshallFloyd::State::NegativeInfinity && !p);
                } else if (!d[v])
                    assert(r.state == WarshallFloyd::State::Unreachable && !p);
                else {
                    assert(r.state == WarshallFloyd::State::Finite && r.value == *d[v] && p &&
                           p->front() == source && p->back() == v &&
                           p->size() <= static_cast<std::size_t>(n));
                    Wide sum = 0;
                    for (std::size_t i = 1; i < p->size(); ++i) {
                        std::optional<long long> cost;
                        for (auto [u, w, c] : edges)
                            if (u == (*p)[i - 1] && w == (*p)[i])
                                cost = cost ? std::min(*cost, c) : c;
                        assert(cost);
                        sum += *cost;
                    }
                    assert(sum == r.value);
                }
            }
        }
        assert(g.has_negative_cycle() == any_negative);
    }
    WarshallFloyd huge(3);
    huge.add_edge(0, 1, LLONG_MAX);
    huge.add_edge(1, 2, LLONG_MAX);
    huge.solve();
    assert(huge.distance(0, 2).value == Wide(LLONG_MAX) * 2);
    huge.add_edge(2, 2, LLONG_MIN);
    huge.solve();
    assert(huge.distance(0, 2).state == WarshallFloyd::State::NegativeInfinity);
    WarshallFloyd saturation(150);
    for (int i = 0; i < 150; ++i) {
        saturation.add_edge(i, i, LLONG_MIN);
        if (i)
            saturation.add_edge(i - 1, i, LLONG_MAX);
    }
    saturation.solve();
    assert(saturation.distance(0, 149).state == WarshallFloyd::State::NegativeInfinity &&
           saturation.distance(149, 0).state == WarshallFloyd::State::Unreachable);
}
void sequences() {
    for (int n = 0; n <= 8; ++n) {
        int total = 1;
        for (int i = 0; i < n; ++i)
            total *= 3;
        for (int mask = 0; mask < total; ++mask) {
            int tmp = mask;
            std::vector<int> a(n);
            for (int &x : a) {
                x = tmp % 3 - 1;
                tmp /= 3;
            }
            for (bool maximum : {false, true}) {
                auto t = maximum ? cartesian_tree(a, std::greater<int>{}) : cartesian_tree(a);
                auto visit = [&](auto &&self, int l, int r, int parent) -> void {
                    if (l == r)
                        return;
                    int root = l;
                    for (int i = l + 1; i < r; ++i)
                        if (maximum ? a[root] < a[i] : a[i] < a[root])
                            root = i;
                    assert(t.parent[root] == parent);
                    if (parent < 0)
                        assert(t.root == root);
                    self(self, l, root, root);
                    self(self, root + 1, r, root);
                    if (l == root)
                        assert(t.left[root] == -1);
                    else
                        assert(t.left[root] >= l && t.left[root] < root);
                    if (root + 1 == r)
                        assert(t.right[root] == -1);
                    else
                        assert(root < t.right[root] && t.right[root] < r);
                };
                visit(visit, 0, n, -1);
                if (!n)
                    assert(t.root == -1);
                for (int width = 0; width <= n + 1; ++width) {
                    auto s = maximum ? sliding_window_max(a, width) : sliding_window_min(a, width);
                    assert(s.indices.size() ==
                           static_cast<std::size_t>(width && width <= n ? n - width + 1 : 0));
                    for (std::size_t i = 0; i < s.indices.size(); ++i) {
                        int best = i;
                        for (int j = i + 1; j < static_cast<int>(i) + width; ++j)
                            if (maximum ? a[best] < a[j] : a[j] < a[best])
                                best = j;
                        assert(s.indices[i] == best && s.values[i] == a[best]);
                    }
                }
            }
        }
    }
}
void large() {
    int n = 200000;
    std::vector<std::pair<int, int>> e;
    std::vector<MSTEdge> w;
    for (int i = 1; i < n; ++i) {
        e.emplace_back(i - 1, i);
        w.push_back({i - 1, i, 1});
    }
    assert(!find_cycle(n, e, true));
    auto p = euler_trail(n, e, true);
    assert(p && p->edge_ids.size() == e.size());
    e.emplace_back(n - 1, 0);
    assert(find_cycle(n, e, false)->vertices.size() == static_cast<std::size_t>(n));
    assert(decompose_namori(n, e)->root.size() == static_cast<std::size_t>(n));
    assert(kruskal(n, w).total_weight == n - 1);
    e.back()={0,0};
    auto attached=decompose_namori(n,e);
    assert(attached&&attached->cycle_vertices.size()==1&&attached->root.back()==0);
    WarshallFloyd dense(300);
    for(int i=0;i<300;++i)for(int j=0;j<300;++j)dense.add_edge(i,j,1);
    dense.solve();assert(!dense.has_negative_cycle()&&dense.distance(0,299).value==1);
    std::vector<int> a(n);
    std::iota(a.begin(), a.end(), 0);
    assert(cartesian_tree(a).root == 0 &&
           sliding_window_min(a, 100).indices.size() == static_cast<std::size_t>(n - 99));
}
int main(int argc, char **argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (mode == "all" || mode == "mst")
        mst();
    if (mode == "all" || mode == "walks")
        walks();
    if (mode == "all" || mode == "floyd")
        floyd();
    if (mode == "all" || mode == "sequences")
        sequences();
    if (mode == "large")
        large();
}
