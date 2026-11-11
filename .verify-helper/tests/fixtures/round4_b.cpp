#include "library/graph/bipartite_graph.hpp"
#include "library/graph/bipartite_matching.hpp"
#include "library/graph/centroid_decomposition.hpp"
#include "library/graph/biconnected_components.hpp"
#include "library/graph/block_cut_tree.hpp"
#include "library/graph/tree_isomorphism.hpp"

#include <functional>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <tuple>
int count_components(int n, const std::vector<std::pair<int, int>> &edges, int mask) {
    std::vector<bool> seen(n);
    int count = 0;
    for (int root = 0; root < n; ++root)
        if ((mask >> root & 1) && !seen[root]) {
            ++count;
            std::vector<int> q{root};
            seen[root] = true;
            for (std::size_t i = 0; i < q.size(); ++i)
                for (auto [a, b] : edges) {
                    int to = -1;
                    if (a == q[i])
                        to = b;
                    if (b == q[i])
                        to = a;
                    if (to >= 0 && (mask >> to & 1) && !seen[to]) {
                        seen[to] = true;
                        q.push_back(to);
                    }
                }
        }
    return count;
}
void coloring() {
    std::mt19937 rng(736);
    for (int t = 0; t < 2000; ++t) {
        int n = rng() % 9;
        std::vector<std::pair<int, int>> e;
        if (n)
            for (int k = rng() % 25; k--;)
                e.emplace_back(rng() % n, rng() % n);
        auto result = bipartite_coloring(n, e);
        bool expected = false;
        for (int mask = 0; mask < (1 << n); ++mask) {
            bool ok = true;
            for (auto [u, v] : e)
                ok &= ((mask >> u) ^ (mask >> v)) & 1;
            expected |= ok;
        }
        assert(result.bipartite == expected);
        if (expected) {
            for (auto [u, v] : e)
                assert(result.color[u] != result.color[v]);
        } else {
            int len = result.odd_cycle.size();
            assert(len % 2 == 1 && int(result.odd_cycle_edges.size()) == len);
            std::set<int> vs(result.odd_cycle.begin(), result.odd_cycle.end());
            assert(int(vs.size()) == len);
            for (int i = 0; i < len; ++i) {
                auto [u, v] = e[result.odd_cycle_edges[i]];
                int a = result.odd_cycle[i], b = result.odd_cycle[(i + 1) % len];
                assert((a == u && b == v) || (a == v && b == u));
            }
        }
    }
}
int matching_brute(int l, int r, const std::vector<std::pair<int, int>> &e) {
    std::vector<int> dp(1 << r, -100);
    dp[0] = 0;
    for (int v = 0; v < l; ++v) {
        auto next = dp;
        for (int mask = 0; mask < (1 << r); ++mask)
            for (auto [a, b] : e)
                if (a == v && !(mask >> b & 1))
                    next[mask | 1 << b] = std::max(next[mask | 1 << b], dp[mask] + 1);
        dp = next;
    }
    return *std::max_element(dp.begin(), dp.end());
}
void matching() {
    auto check = [](int l, int r, const std::vector<std::pair<int, int>> &e) {
        BipartiteMatching g(l, r);
        for (auto [u, v] : e)
            g.add_edge(u, v);
        int ans = g.solve();
        assert(ans == matching_brute(l, r, e));
        assert(int(g.matching().size()) == ans);
        std::set<int> used;
        for (auto [u, v] : g.matching()) {
            assert(g.match_right[v] == u && g.match_left[u] == v && used.insert(v).second);
            assert(std::find(e.begin(), e.end(), std::pair{u, v}) != e.end());
        }
        auto [a, b] = g.minimum_vertex_cover();
        assert(int(a.size() + b.size()) == ans);
        for (auto [u, v] : e)
            assert(std::find(a.begin(), a.end(), u) != a.end() ||
                   std::find(b.begin(), b.end(), v) != b.end());
        assert(g.solve() == ans);
    };
    for (int l = 0; l <= 3; ++l)
        for (int r = 0; r <= 3; ++r)
            for (int mask = 0; mask < (1 << (l * r)); ++mask) {
                std::vector<std::pair<int, int>> e;
                for (int i = 0; i < l; ++i)
                    for (int j = 0; j < r; ++j)
                        if (mask >> (i * r + j) & 1)
                            e.emplace_back(i, j);
                check(l, r, e);
            }
    std::mt19937 rng(545);
    for (int t = 0; t < 500; ++t) {
        int l = rng() % 8, r = rng() % 8;
        std::vector<std::pair<int, int>> e;
        if (l && r)
            for (int k = rng() % 30; k--;)
                e.emplace_back(rng() % l, rng() % r);
        check(l, r, e);
    }
    int n = 100000;
    BipartiteMatching g(n, n);
    for (int i = 0; i < n - 1; ++i) {
        g.add_edge(i, i);
        g.add_edge(i, i + 1);
    }
    g.add_edge(n - 1, 0);
    assert(g.solve() == n);
}
std::vector<std::vector<int>> distances(const std::vector<std::vector<int>> &g) {
    int n = g.size();
    std::vector<std::vector<int>> d(n, std::vector<int>(n, -1));
    for (int s = 0; s < n; ++s) {
        std::vector<int> q{s};
        d[s][s] = 0;
        for (std::size_t i = 0; i < q.size(); ++i)
            for (int v : g[q[i]])
                if (d[s][v] < 0) {
                    d[s][v] = d[s][q[i]] + 1;
                    q.push_back(v);
                }
    }
    return d;
}
void centroids() {
    assert(CentroidDecomposition({}).parent.empty());
    std::mt19937 rng(442);
    for (int t = 0; t < 200; ++t) {
        int n = 1 + rng() % 50;
        std::vector<std::vector<int>> g(n);
        for (int v = 1; v < n; ++v) {
            int p = rng() % v;
            g[p].push_back(v);
            g[v].push_back(p);
        }
        CentroidDecomposition c(g);
        auto d = distances(g);
        std::vector<int> best(n, 1000000);
        std::vector<bool> marked(n);
        for (int v = 0; v < n; ++v) {
            assert(c.ancestors[v].back() == std::pair(v, 0));
            int prev = -1;
            for (auto [a, dist] : c.ancestors[v]) {
                assert(dist == d[a][v] && c.parent[a] == prev);
                prev = a;
            }
            assert(c.level[v] + 1 == int(c.ancestors[v].size()));
        }
        for (int k = 0; k < 100; ++k) {
            int v = rng() % n;
            if (rng() % 2) {
                marked[v] = true;
                for (auto [a, x] : c.ancestors[v])
                    best[a] = std::min(best[a], x);
            } else {
                int ans = 1000000, expected = 1000000;
                for (auto [a, x] : c.ancestors[v])
                    ans = std::min(ans, best[a] + x);
                for (int i = 0; i < n; ++i)
                    if (marked[i])
                        expected = std::min(expected, d[i][v]);
                assert(ans == expected);
            }
        }
    }
    int n = 200000;
    std::vector<std::vector<int>> g(n);
    for (int v = 1; v < n; ++v) {
        g[v].push_back(v - 1);
        g[v - 1].push_back(v);
    }
    CentroidDecomposition c(g);
    for (int v = 0; v < n; ++v) {
        assert(c.ancestors[v].size() < 20);
        for (auto [a, d] : c.ancestors[v])
            assert(d == std::abs(a - v));
    }
}
void blocks() {
    std::mt19937 rng(233);
    for (int t = 0; t < 500; ++t) {
        int n = rng() % 8;
        BiconnectedComponents b(n);
        std::vector<std::pair<int, int>> edges;
        if (n)
            for (int k = rng() % 24; k--;) {
                int u = rng() % n, v = rng() % n;
                edges.emplace_back(u, v);
                b.add_edge(u, v);
            }
        b.build();
        BlockCutTree cut(b);
        std::set<std::vector<int>> expected, actual;
        int full = (1 << n) - 1;
        std::vector<int> valid;
        for (int mask = 1; mask <= full; ++mask) {
            if (__builtin_popcount(unsigned(mask)) < 2 || count_components(n, edges, mask) != 1)
                continue;
            bool ok = true;
            for (int v = 0; v < n; ++v)
                if (mask >> v & 1)
                    ok &= count_components(n, edges, mask ^ (1 << v)) == 1;
            if (ok)
                valid.push_back(mask);
        }
        for (int mask : valid) {
            bool maximal = true;
            for (int other : valid)
                if (mask != other && (mask & other) == mask)
                    maximal = false;
            if (maximal) {
                std::vector<int> vs;
                for (int v = 0; v < n; ++v)
                    if (mask >> v & 1)
                        vs.push_back(v);
                expected.insert(vs);
            }
        }
        for (int v = 0; v < n; ++v) {
            bool isolated = true;
            for (auto [a, c] : edges)
                if (a != c && (a == v || c == v))
                    isolated = false;
            if (isolated)
                expected.insert({v});
        }
        std::vector<int> counts(edges.size());
        for (std::size_t i = 0; i < b.components.size(); ++i) {
            auto vs = b.components[i];
            std::sort(vs.begin(), vs.end());
            assert(std::unique(vs.begin(), vs.end()) == vs.end());
            actual.insert(vs);
            for (int id : b.component_edges[i]) {
                assert(++counts[id] == 1);
                auto [u, v] = edges[id];
                assert(std::binary_search(vs.begin(), vs.end(), u) &&
                       std::binary_search(vs.begin(), vs.end(), v));
            }
        }
        assert(actual == expected);
        for (int count : counts)
            assert(count == 1);
        int base = count_components(n, edges, full);
        std::vector<int> art;
        for (int v = 0; v < n; ++v)
            if (count_components(n, edges, full ^ (1 << v)) > base)
                art.push_back(v);
        assert(art == b.articulation_points);
        int sum = 0;
        for (auto &list : cut.tree)
            sum += list.size();
        assert(sum / 2 == int(cut.tree.size()) - base);
        auto reach = distances(cut.tree);
        for (int v = 0; v < n; ++v) {
            assert(cut.vertex_node[v] >= 0);
            for (int u = 0; u < n; ++u) {
                int pairmask = full;
                std::vector<std::vector<int>> g(n);
                for (auto [a, c] : edges) {
                    g[a].push_back(c);
                    g[c].push_back(a);
                }
                auto d = distances(g);
                assert((reach[cut.vertex_node[v]][cut.vertex_node[u]] >= 0) == (d[v][u] >= 0));
                (void)pairmask;
            }
        }
        b.build();
        assert(b.components.size() == actual.size());
    }
    int n = 100000;
    BiconnectedComponents b(n);
    for (int i = 1; i < n; ++i)
        b.add_edge(i - 1, i);
    b.build();
    BlockCutTree cut(b);
    assert(int(b.components.size()) == n - 1 && int(cut.tree.size()) == 2 * n - 3);
}
std::string rooted_string(const std::vector<std::vector<int>> &g, int v, int p) {
    std::vector<std::string> children;
    for (int to : g[v])
        if (to != p)
            children.push_back(rooted_string(g, to, v));
    std::sort(children.begin(), children.end());
    std::string s = "(";
    for (auto &x : children)
        s += x;
    return s + ")";
}
void isomorphism() {
    TreeIsomorphism ids;
    std::map<int, std::string> seen;
    std::map<std::string, int> reverse;
    std::mt19937 rng(43);
    assert(ids.rooted_id({}) == 0 && ids.unrooted_id({}) == 0);
    assert(ids.unrooted_id({{1}, {0}}) != ids.unrooted_id({{1}, {0, 2}, {1}}));
    for (int t = 0; t < 1500; ++t) {
        int n = 1 + rng() % 10;
        std::vector<std::vector<int>> g(n), h(n);
        for (int v = 1; v < n; ++v) {
            int p = rng() % v;
            g[p].push_back(v);
            g[v].push_back(p);
        }
        std::vector<int> perm(n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), rng);
        for (int v = 0; v < n; ++v)
            for (int to : g[v])
                h[perm[v]].push_back(perm[to]);
        std::string canonical;
        for (int v = 0; v < n; ++v) {
            auto s = rooted_string(g, v, -1);
            if (canonical.empty() || s < canonical)
                canonical = s;
            assert(ids.rooted_id(g, v) == ids.rooted_id(h, perm[v]));
        }
        int id = ids.unrooted_id(g);
        assert(id == ids.unrooted_id(h));
        if (seen.count(id))
            assert(seen[id] == canonical);
        if (reverse.count(canonical))
            assert(reverse[canonical] == id);
        seen[id] = canonical;
        reverse[canonical] = id;
    }
    int n = 100000;
    std::vector<std::vector<int>> g(n);
    for (int v = 1; v < n; ++v) {
        g[v].push_back(v - 1);
        g[v - 1].push_back(v);
    }
    assert(ids.rooted_id(g, 0) == ids.rooted_id(g, n - 1));
}
int main() {
    coloring();
    matching();
    centroids();
    blocks();
    isomorphism();
}
