#include "library/data_structure/partially_persistent_union_find.hpp"
#include "library/data_structure/sparse_fenwick_tree_2d.hpp"
#include "library/data_structure/dynamic_bitset.hpp"
#include "library/data_structure/dual_segment_tree.hpp"
#include "library/data_structure/segment_li_chao_tree.hpp"

#include <atcoder/modint>
#include <climits>
#include <map>
#include <numeric>
#include <random>
#include <string>
using W = __int128_t;
using M = atcoder::modint998244353;
struct F {
    M a, b;
};
M mapping(F f, M x) { return f.a * x + f.b; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F identity() { return {1, 0}; }
void persistent() {
    std::mt19937 rng(56);
    for (int rep = 0; rep < 100; ++rep) {
        int n = 1 + rng() % 35;
        PartiallyPersistentUnionFind u(n);
        std::vector<std::vector<int>> states(1, std::vector<int>(n));
        std::iota(states[0].begin(), states[0].end(), 0);
        for (int q = 0; q < 500; ++q) {
            int a = rng() % n, b = rng() % n, t = int(u.time()) + 1 + rng() % 4;
            auto next = states.back();
            int x = next[a], y = next[b];
            for (int &v : next)
                if (v == y)
                    v = x;
            while (int(states.size()) <= t)
                states.push_back(states.back());
            states[t] = next;
            assert(u.merge(t, a, b) == (x != y));
            for (int test = 0; test < 5; ++test) {
                int past = rng() % (t + 1), v = rng() % n, w = rng() % n;
                assert(u.same(past, v, w) == (states[past][v] == states[past][w]));
                assert(u.size(past, v) ==
                       std::count(states[past].begin(), states[past].end(), states[past][v]));
            }
        }
    }
    PartiallyPersistentUnionFind u(2);
    assert(u.merge(0, 1) && u.time() == 1 && !u.merge(0, 1) && u.time() == 2 && u.size(0, 0) == 1);
    bool caught = false;
    try {
        u.merge(2, 0, 1);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    u.merge(LLONG_MAX - 1, 0, 1);
    caught = false;
    try {
        u.merge(0, 1);
    } catch (const std::overflow_error &) {
        caught = true;
    }
    assert(caught);
    int n = 200000;
    PartiallyPersistentUnionFind large(n);
    for (int i = 1; i < n; ++i)
        large.merge(i - 1, i);
    assert(large.size(n - 1, 0) == n && large.size(n / 2, 0) == n / 2 + 1);
}
void sparse() {
    std::mt19937_64 rng(92);
    std::vector<std::pair<long long, long long>> points;
    for (int i = 0; i < 500; ++i)
        points.emplace_back(int(rng() % 51) - 25, int(rng() % 61) - 30);
    SparseFenwickTree2D<long long> b(points);
    std::map<std::pair<long long, long long>, long long> ref;
    for (int q = 0; q < 5000; ++q) {
        auto [y, x] = points[rng() % points.size()];
        long long d = int(rng() % 31) - 15;
        b.add(y, x, d);
        ref[{y, x}] += d;
        long long y1 = int(rng() % 81) - 40, y2 = int(rng() % 81) - 40, x1 = int(rng() % 81) - 40,
                  x2 = int(rng() % 81) - 40;
        if (y1 > y2)
            std::swap(y1, y2);
        if (x1 > x2)
            std::swap(x1, x2);
        long long expected = 0;
        for (auto [p, v] : ref)
            if (y1 <= p.first && p.first < y2 && x1 <= p.second && p.second < x2)
                expected += v;
        assert(b.sum(y1, y2, x1, x2) == expected);
    }
    SparseFenwickTree2D<long long> empty({});
    assert(empty.sum(LLONG_MIN, LLONG_MAX, LLONG_MIN, LLONG_MAX) == 0);
    bool caught = false;
    try {
        b.add(100, 100, 1);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    SparseFenwickTree2D<W, W> wide({{W(LLONG_MIN), W(LLONG_MAX)}});
    wide.add(LLONG_MIN, LLONG_MAX, W(LLONG_MAX));
    assert(wide.sum(W(LLONG_MIN), W(LLONG_MAX) + 1, W(LLONG_MIN), W(LLONG_MAX) + 1) == LLONG_MAX);
    points.clear();
    for (int i = 0; i < 100000; ++i)
        points.emplace_back(1000000000LL * i, -1000000000LL * i);
    SparseFenwickTree2D<long long> large(points);
    for (auto [y, x] : points)
        large.add(y, x, 1);
    assert(large.sum(LLONG_MIN, LLONG_MAX, LLONG_MIN, LLONG_MAX) == 100000);
}
void bits() {
    std::mt19937 rng(321);
    for (std::size_t n : {0, 1, 2, 63, 64, 65, 127, 128, 129, 257}) {
        for (int rep = 0; rep < 80; ++rep) {
            DynamicBitset a(n), b(n);
            std::string x(n, '0'), y(n, '0');
            for (std::size_t i = 0; i < n; ++i) {
                if (rng() % 2) {
                    a.set(i);
                    x[i] = '1';
                }
                if (rng() % 2) {
                    b.set(i);
                    y[i] = '1';
                }
            }
            assert(a.to_string() == x && b.to_string() == y);
            assert(a.count() == std::size_t(std::count(x.begin(), x.end(), '1')));
            auto check = [&](const DynamicBitset &v, const std::string &text) {
                assert(v.to_string() == text &&
                       v.count() == std::size_t(std::count(text.begin(), text.end(), '1')));
                std::vector<std::size_t> positions;
                for (auto p = v.find_first(); p; p = v.find_next(*p))
                    positions.push_back(*p);
                std::vector<std::size_t> expected;
                for (std::size_t i = 0; i < n; ++i)
                    if (text[i] == '1')
                        expected.push_back(i);
                assert(positions == expected);
            };
            std::string ands = x, ors = x, xors = x, notx = x;
            for (std::size_t i = 0; i < n; ++i) {
                ands[i] = char('0' + (x[i] == '1' && y[i] == '1'));
                ors[i] = char('0' + (x[i] == '1' || y[i] == '1'));
                xors[i] = char('0' + (x[i] != y[i]));
                notx[i] = x[i] == '1' ? '0' : '1';
            }
            check(a & b, ands);
            check(a | b, ors);
            check(a ^ b, xors);
            check(~a, notx);
            for (std::size_t k : std::vector<std::size_t>{0, 1, 63, 64, 65, n, n + 1, std::size_t(-1)}) {
                std::string left(n, '0'), right(n, '0');
                if (k < n)
                    for (std::size_t i = 0; i < n - k; ++i) {
                        left[i + k] = x[i];
                        right[i] = x[i + k];
                    }
                check(a << k, left);
                check(a >> k, right);
            }
            auto copy = a;
            copy.flip().flip();
            assert(copy == a);
            if (n) {
                auto i = rng() % n;
                copy.flip(i).flip(i);
                assert(copy == a);
                copy.reset(i);
                assert(!copy.test(i));
            }
            DynamicBitset filled(n, true);
            assert(filled.count() == n && (~filled).none());
        }
    }
    bool caught = false;
    try {
        auto b = DynamicBitset(1) | DynamicBitset(2);
        (void)b;
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    // 実用例：bitsetシフトによる部分和を独立なbool DPと比較する。
    DynamicBitset reachable(20001);
    reachable.set(0);
    std::vector<bool> ref(20001);
    ref[0] = true;
    for (int q = 0; q < 1000; ++q) {
        int w = 1 + rng() % 100;
        reachable |= reachable << w;
        for (int x = 20000; x >= w; --x)
            ref[x] = ref[x] || ref[x - w];
    }
    for (int i = 0; i <= 20000; ++i)
        assert(reachable.test(i) == ref[i]);
}
void dual() {
    std::mt19937 rng(98);
    for (int n = 0; n <= 80; ++n) {
        std::vector<M> a(n);
        for (auto &x : a)
            x = rng() % 10;
        DualSegmentTree<M, F, mapping, composition, identity> d(a);
        for (int q = 0; q < 1000; ++q) {
            int l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r)
                std::swap(l, r);
            F f{rng() % 5, rng() % 7};
            d.apply(l, r, f);
            for (int i = l; i < r; ++i)
                a[i] = mapping(f, a[i]);
            if (n) {
                int i = rng() % n;
                if (rng() % 5 == 0) {
                    M x = rng() % 100;
                    d.set(i, x);
                    a[i] = x;
                }
                assert(d.get(i) == a[i]);
            }
        }
        for (int i = 0; i < n; ++i)
            assert(d.get(i) == a[i]);
    }
    std::vector<M> a(200000, 1);
    DualSegmentTree<M, F, mapping, composition, identity> d(a);
    d.apply(0, 200000, {2, 1});
    d.apply(1, 199999, {3, 4});
    assert(d.get(0) == 3 && d.get(100000) == 13 && d.get(199999) == 3);
}
void segment_li() {
    std::mt19937_64 rng(627);
    SegmentLiChaoTree empty({});
    empty.add_line(0, 0);
    empty.add_segment(-1, 1, 0, 0);
    for (int rep = 0; rep < 200; ++rep) {
        std::vector<long long> xs{LLONG_MIN, LLONG_MAX, -1, 0, 1};
        for (int i = 0; i < 30; ++i)
            xs.push_back(int(rng() % 101) - 50);
        SegmentLiChaoTree tree(xs);
        std::sort(xs.begin(), xs.end());
        xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
        struct Line {
            int l, r;
            long long a, b;
        };
        std::vector<Line> lines;
        for (int q = 0; q < 200; ++q) {
            long long a = static_cast<long long>(rng()), b = static_cast<long long>(rng());
            int l = rng() % (xs.size() + 1), r = rng() % (xs.size() + 1);
            if (l > r)
                std::swap(l, r);
            if (q % 3 == 0) {
                tree.add_line(a, b);
                lines.push_back({0, int(xs.size()), a, b});
            } else if (q % 3 == 1) {
                tree.add_segment_indices(l, r, a, b);
                lines.push_back({l, r, a, b});
            } else {
                long long lo = int(rng() % 201) - 100, hi = int(rng() % 201) - 100;
                if (lo > hi)
                    std::swap(lo, hi);
                tree.add_segment(lo, hi, a, b);
                int left = std::lower_bound(xs.begin(), xs.end(), lo) - xs.begin(),
                    right = std::lower_bound(xs.begin(), xs.end(), hi) - xs.begin();
                lines.push_back({left, right, a, b});
            }
            for (int t = 0; t < 4; ++t) {
                int index = rng() % xs.size();
                std::optional<W> best;
                for (auto line : lines)
                    if (line.l <= index && index < line.r) {
                        W value = W(line.a) * xs[index] + line.b;
                        if (!best || value < *best)
                            best = value;
                    }
                assert(tree.query(xs[index]) == best);
            }
        }
    }
    SegmentLiChaoTree sub({0, 1, 2, 3});
    sub.add_segment(2, 3, 0, -7);
    assert(!sub.query(0) && sub.query(2) == -7 && !sub.query(3));
}
int main() {
    persistent();
    sparse();
    bits();
    dual();
    segment_li();
}
