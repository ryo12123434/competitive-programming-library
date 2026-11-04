#include "library/data_structure/fenwick_tree_2d.hpp"
#include "library/data_structure/range_tree_2d.hpp"
#include "library/algorithm/parallel_binary_search.hpp"
#include "library/algorithm/mo_with_updates.hpp"
#include "library/data_structure/range_add_fenwick_tree.hpp"
#include <atcoder/modint>
#include "library/data_structure/union_find.hpp"
#include <climits>
#include <map>
#include <random>
#include <string>
void fenwick() {
    std::mt19937 rng(5571);
    for (int h = 0; h < 12; ++h)
        for (int w = 0; w < 12; ++w) {
            std::vector<std::vector<long long>> a(h, std::vector<long long>(w));
            for (auto &row : a)
                for (auto &v : row)
                    v = static_cast<int>(rng() % 41) - 20;
            FenwickTree2D<long long> from(a), zero(h, w);
            for (int i = 0; i < h; ++i)
                for (int j = 0; j < w; ++j)
                    zero.add(i, j, a[i][j]);
            for (int step = 0; step < 300; ++step) {
                if (h && w && step % 2 == 0) {
                    int y = rng() % h, x = rng() % w;
                    long long value = static_cast<int>(rng() % 61) - 30;
                    a[y][x] += value;
                    zero.add(y, x, value);
                    from.add(y, x, value);
                }
                int u = rng() % (h + 1), d = rng() % (h + 1), l = rng() % (w + 1), r = rng() % (w + 1);
                if (u > d)
                    std::swap(u, d);
                if (l > r)
                    std::swap(l, r);
                long long sum = 0;
                for (int y = u; y < d; ++y)
                    for (int x = l; x < r; ++x)
                        sum += a[y][x];
                assert(zero.sum(u, d, l, r) == sum);
                if (h)
                    assert(from.sum(u, d, l, r) == sum);
            }
        }
    for (int n = 0; n < 60; ++n) {
        RangeAddFenwickTree<long long> bit(n);
        std::vector<long long> a(n);
        for (int step = 0; step < 700; ++step) {
            int l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r)
                std::swap(l, r);
            long long value = static_cast<int>(rng() % 41) - 20;
            if (step % 2) {
                bit.add(l, r, value);
                for (int i = l; i < r; ++i)
                    a[i] += value;
            }
            long long sum = 0;
            for (int i = l; i < r; ++i)
                sum += a[i];
            assert(bit.sum(l, r) == sum && bit.sum(n) == std::accumulate(a.begin(), a.end(), 0LL));
        }
    }
    using Mint = atcoder::modint998244353;
    FenwickTree2D<Mint> two(2, 3);
    two.add(1, 2, -5);
    assert(two.sum(0, 2, 0, 3) == Mint(-5));
    RangeAddFenwickTree<Mint> one(5);
    one.add(0, 5, -3);
    one.add(1, 4, 2);
    assert(one.sum(0, 5) == Mint(-9));
}
void range_tree() {
    std::mt19937 rng(8423);
    for (int trial = 0; trial < 500; ++trial) {
        int n = trial % 70;
        std::vector<std::pair<long long, long long>> points;
        for (int i = 0; i < n; ++i)
            points.emplace_back(static_cast<int>(rng() % 13) - 6, static_cast<int>(rng() % 13) - 6);
        RangeTree2D<> tree(points);
        assert(tree.size() == n);
        for (int q = 0; q < 200; ++q) {
            int l = static_cast<int>(rng() % 21) - 10, r = static_cast<int>(rng() % 21) - 10,
                d = static_cast<int>(rng() % 21) - 10, u = static_cast<int>(rng() % 21) - 10;
            if (l > r)
                std::swap(l, r);
            if (d > u)
                std::swap(d, u);
            int expected = 0;
            for (auto [x, y] : points)
                expected += l <= x && x < r && d <= y && y < u;
            assert(tree.count(l, r, d, u) == expected);
        }
    }
    RangeTree2D<> extreme({{LLONG_MIN, LLONG_MAX}, {LLONG_MAX, LLONG_MIN}, {0, 0}, {0, 0}});
    assert(extreme.count(LLONG_MIN, LLONG_MAX, LLONG_MIN, LLONG_MAX) == 2);
    using W = __int128_t;
    RangeTree2D<W> wide({{LLONG_MAX, LLONG_MAX}});
    assert(wide.count(LLONG_MIN, W(LLONG_MAX) + 1, LLONG_MIN, W(LLONG_MAX) + 1) == 1);
}
void parallel() {
    std::mt19937 rng(457);
    for (int trial = 0; trial < 400; ++trial) {
        int m = trial % 35, q = rng() % 55;
        std::vector<int> need(q);
        for (int &x : need)
            x = rng() % (m + 2);
        int state = 0, resets = 0;
        auto r = parallel_binary_search(
            m, q,
            [&] {
                state = 0;
                ++resets;
            },
            [&](int id) {
                assert(id == state);
                ++state;
            },
            [&](int id) { return state >= need[id]; });
        assert(r == need);
        if (!q)
            assert(resets == 0);
    }
    for (int trial = 0; trial < 120; ++trial) {
        int n = 1 + trial % 15, m = rng() % 45, q = 30;
        std::vector<std::pair<int, int>> edges, queries;
        for (int i = 0; i < m; ++i)
            edges.emplace_back(rng() % n, rng() % n);
        for (int i = 0; i < q; ++i)
            queries.emplace_back(rng() % n, rng() % n);
        UnionFind uf(n);
        auto r = parallel_binary_search(
            m, q, [&] { uf = UnionFind(n); },
            [&](int i) {
                auto [u, v] = edges[i];
                uf.unite(u, v);
            },
            [&](int id) {
                auto [u, v] = queries[id];
                return uf.issame(u, v);
            });
        for (int id = 0; id < q; ++id) {
            int expected = m + 1;
            std::vector<std::vector<int>> g(n);
            auto [u, v] = queries[id];
            for (int t = 0; t <= m; ++t) {
                std::vector<int> seen(n);
                std::vector<int> queue{u};
                seen[u] = 1;
                for (std::size_t i = 0; i < queue.size(); ++i)
                    for (int to : g[queue[i]])
                        if (!seen[to]) {
                            seen[to] = 1;
                            queue.push_back(to);
                        }
                if (seen[v]) {
                    expected = t;
                    break;
                }
                if (t < m) {
                    auto [a, b] = edges[t];
                    g[a].push_back(b);
                    g[b].push_back(a);
                }
            }
            assert(r[id] == expected);
        }
    }
}
void mo() {
    std::mt19937 rng(2411);
    for (int trial = 0; trial < 180; ++trial) {
        int n = trial % 40;
        std::vector<int> initial(n);
        for (int &x : initial)
            x = static_cast<int>(rng() % 31) - 15;
        auto values = initial;
        MoWithUpdates<int> mo(initial);
        std::vector<long long> sums;
        std::vector<int> distinct;
        for (int step = 0; step < 240; ++step) {
            if (n && rng() % 2) {
                int p = rng() % n, value = static_cast<int>(rng() % 31) - 15;
                mo.add_update(p, value);
                values[p] = value;
            } else {
                int l = rng() % (n + 1), r = rng() % (n + 1);
                if (l > r)
                    std::swap(l, r);
                int id = mo.add_query(l, r);
                assert(id == static_cast<int>(sums.size()));
                long long sum = 0;
                std::set<int> s;
                for (int i = l; i < r; ++i) {
                    sum += values[i];
                    s.insert(values[i]);
                }
                sums.push_back(sum);
                distinct.push_back(s.size());
            }
        }
        for (int block : {0, 1, 3, 100}) {
            std::map<int, int> counts;
            long long sum = 0;
            int count = 0;
            std::vector<long long> result(sums.size(), LLONG_MAX);
            std::vector<int> actual(distinct.size(), -1);
            mo.run(
                [&](int, int x) {
                    sum += x;
                    if (counts[x]++ == 0)
                        ++count;
                },
                [&](int, int x) {
                    assert(counts[x] > 0);
                    sum -= x;
                    if (--counts[x] == 0)
                        --count;
                },
                [&](int id) {
                    result[id] = sum;
                    actual[id] = count;
                },
                block);
            assert(result == sums && actual == distinct);
        }
        assert(initial.size() == values.size());
    }
    MoWithUpdates<int> empty({1});
    empty.add_update(0, 2);
    int callbacks = 0;
    empty.run([&](int, int) { ++callbacks; }, [&](int, int) { ++callbacks; }, [&](int) { ++callbacks; });
    assert(callbacks == 0);
}
void large() {
    const int n = 200000;
    RangeAddFenwickTree<long long> bit(n);
    for (int i = 0; i < n; ++i)
        bit.add(i, n, 1);
    assert(bit.sum(n) == 1LL * n * (n + 1) / 2);
    std::vector<std::pair<long long, long long>> points;
    for (int i = 0; i < n; ++i)
        points.emplace_back(i, i % 10);
    RangeTree2D<> tree(points);
    assert(tree.count(0, n, 0, 5) == n / 2);
    FenwickTree2D<long long> two(std::vector<std::vector<long long>>(400, std::vector<long long>(500, 1)));
    assert(two.sum(0, 400, 0, 500) == n);
    int state = 0;
    auto p = parallel_binary_search(
        n, n, [&] { state = 0; }, [&](int) { ++state; }, [&](int q) { return state >= q; });
    assert(p.back() == n - 1);
    MoWithUpdates<int> mo(std::vector<int>(20000));
    for (int i = 0; i < 5000; ++i) {
        mo.add_update(i, i);
        mo.add_query(0, i + 1);
    }
    long long sum = 0;
    mo.run([&](int, int x) { sum += x; }, [&](int, int x) { sum -= x; },
           [&](int id) { assert(sum == 1LL * id * (id + 1) / 2); });
}
int main(int argc, char **argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (mode == "all" || mode == "fenwick")
        fenwick();
    if (mode == "all" || mode == "range_tree")
        range_tree();
    if (mode == "all" || mode == "parallel")
        parallel();
    if (mode == "all" || mode == "mo")
        mo();
    if (mode == "large")
        large();
}
