#include "library/math/linear_recurrence.hpp"
#include "library/math/berlekamp_massey.hpp"
#include "library/math/bostan_mori.hpp"
#include "library/algorithm/knuth_optimization.hpp"
#include "library/algorithm/smawk.hpp"
#include "library/data_structure/monotone_cht.hpp"
#include "library/math/subset_convolution.hpp"

#include "library/math/formal_power_series.hpp"
#include <atcoder/modint>
#include <array>
#include <climits>
#include <numeric>
#include <random>
using M = atcoder::modint998244353;
using P = FormalPowerSeries<M>;
using W = __int128_t;
void recurrences() {
    std::mt19937 rng(357);
    assert(linear_recurrence(std::vector<M>{}, {}, ~std::uint64_t(0)) == 0);
    for (int t = 0; t < 200; ++t) {
        int k = rng() % 13;
        std::vector<M> initial(k), c(k), sequence(120);
        for (auto &x : initial)
            x = int(rng() % 11) - 5;
        for (auto &x : c)
            x = int(rng() % 11) - 5;
        for (int i = 0; i < 120; ++i) {
            if (i < k)
                sequence[i] = initial[i];
            else
                for (int j = 0; j < k; ++j)
                    sequence[i] += c[j] * sequence[i - 1 - j];
        }
        auto bm = berlekamp_massey(std::vector<M>(sequence.begin(), sequence.begin() + 2 * k + 10));
        assert(int(bm.size()) <= k);
        std::vector<M> first(sequence.begin(), sequence.begin() + bm.size());
        P q{1};
        for (auto x : c)
            q.push_back(-x);
        P numerator = P(initial) * q;
        numerator.resize(k);
        for (int i = 0; i < 120; ++i) {
            assert(linear_recurrence(initial, c, i) == sequence[i]);
            assert(linear_recurrence(first, bm, i) == sequence[i]);
            assert(bostan_mori(numerator, q, i) == sequence[i]);
        }
        auto n = rng();
        assert(bostan_mori(numerator, q, n) == linear_recurrence(initial, c, n));
    }
    for (int t = 0; t < 100; ++t) {
        P p(rng() % 14), q(1 + rng() % 10);
        for (auto &x : p)
            x = rng() % 30;
        for (auto &x : q)
            x = rng() % 30;
        q[0] = 1 + rng() % 30;
        std::vector<M> a(80);
        for (int i = 0; i < 80; ++i) {
            if (i < int(p.size()))
                a[i] = p[i];
            for (int j = 1; j < int(q.size()) && j <= i; ++j)
                a[i] -= q[j] * a[i - j];
            a[i] /= q[0];
            assert(bostan_mori(p, q, i) == a[i]);
        }
    }
    // GF(2)上で短い全列の最小次数を係数の全探索と比較する。
    using B = atcoder::static_modint<2>;
    for (int n = 0; n <= 9; ++n)
        for (int mask = 0; mask < (1 << n); ++mask) {
            std::vector<B> s(n);
            for (int i = 0; i < n; ++i)
                s[i] = mask >> i & 1;
            int degree = 0;
            bool found = false;
            for (; degree <= n && !found; ++degree)
                for (int coefficient = 0; coefficient < (1 << degree); ++coefficient) {
                    bool valid = true;
                    for (int i = degree; i < n; ++i) {
                        int x = 0;
                        for (int j = 0; j < degree; ++j)
                            x ^= ((coefficient >> j) & (mask >> (i - 1 - j))) & 1;
                        valid &= x == int(s[i].val());
                    }
                    if (valid) {
                        found = true;
                        break;
                    }
                }
            assert(int(berlekamp_massey(s).size()) == degree - 1);
        }
    bool caught = false;
    try {
        bostan_mori(P{1}, P{0, 1}, 1);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    caught = false;
    try {
        berlekamp_massey(std::vector<atcoder::static_modint<8>>{1, 1});
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    using Matrix = std::array<M, 4>;
    auto mult = [](Matrix a, Matrix b) {
        return Matrix{a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2],
                      a[2] * b[1] + a[3] * b[3]};
    };
    Matrix a{1, 0, 0, 1}, b{1, 1, 1, 0};
    std::uint64_t n = ~std::uint64_t(0);
    for (auto x = n; x; x >>= 1) {
        if (x & 1)
            a = mult(a, b);
        b = mult(b, b);
    }
    assert(linear_recurrence(std::vector<M>{0, 1}, {1, 1}, n) == a[1]);
    P q(4097);
    q[0] = 1;
    q[1] = -1;
    q.back() = -1;
    P p{1};
    std::vector<M> ref(20001);
    ref[0] = 1;
    for (int i = 1; i <= 20000; ++i) {
        ref[i] = ref[i - 1];
        if (i >= 4096)
            ref[i] += ref[i - 4096];
    }
    assert(bostan_mori(p, q, 20000) == ref.back());
}
void knuth() {
    std::mt19937 rng(33);
    for (int n = 0; n <= 35; ++n)
        for (int t = 0; t < 10; ++t) {
            std::vector<long long> prefix(n + 1);
            for (int i = 0; i < n; ++i)
                prefix[i + 1] = prefix[i] + rng() % 10;
            auto cost = [&](int l, int r) { return prefix[r] - prefix[l]; };
            auto result = knuth_optimization<long long>(n, cost);
            std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(n + 1));
            for (int len = 2; len <= n; ++len)
                for (int l = 0; l + len <= n; ++l) {
                    int r = l + len, split = -1;
                    long long best = LLONG_MAX;
                    for (int k = l + 1; k < r; ++k) {
                        long long x = dp[l][k] + dp[k][r];
                        if (x <= best) {
                            best = x;
                            split = k;
                        }
                    }
                    dp[l][r] = best + cost(l, r);
                    assert(result.dp[l][r] == dp[l][r] && result.split[l][r] == split);
                }
        }
    auto large = knuth_optimization<long long>(1200, [](int l, int r) { return r - l; });
    assert(large.dp[0][1200] > 0);
}
void smawk_test() {
    std::mt19937 rng(443);
    for (int h = 0; h <= 45; ++h)
        for (int w = 1; w <= 45; w += 4) {
            std::vector<int> rows(h), cols(w);
            for (int i = 0; i < h; ++i)
                rows[i] = 2 * i + 1;
            for (int j = 0; j < w; ++j)
                cols[j] = 3 * j;
            std::vector<long long> bias(w * 3 + 1);
            for (auto &x : bias)
                x = int(rng() % 31) - 15;
            std::size_t calls = 0;
            auto f = [&](int i, int j) {
                ++calls;
                return 1LL * (i - j) * (i - j) + bias[j];
            };
            auto answer = smawk(rows, cols, f);
            assert(calls <= 30 * (rows.size() + cols.size()));
            for (int i = 0; i < h; ++i) {
                int best = cols[0];
                for (int j : cols)
                    if (f(rows[i], j) < f(rows[i], best))
                        best = j;
                assert(answer[i] == best);
            }
            auto tied = smawk(rows, cols, [](int, int) { return 0; });
            for (int x : tied)
                assert(x == cols[0]);
        }
    bool caught = false;
    try {
        smawk(std::vector<int>{1}, std::vector<int>{}, [](int, int) { return 0; });
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    std::vector<int> rows(100000), cols(200000);
    std::iota(rows.begin(), rows.end(), 0);
    std::iota(cols.begin(), cols.end(), 0);
    std::size_t calls = 0;
    auto a = smawk(rows, cols, [&](int i, int j) {
        ++calls;
        return 1LL * (i - j) * (i - j);
    });
    assert(calls < 30 * (rows.size() + cols.size()));
    for (int i = 0; i < int(rows.size()); ++i)
        assert(a[i] == i);
}
void cht() {
    std::mt19937_64 rng(643);
    for (int t = 0; t < 150; ++t) {
        MonotoneCHT<> lo;
        MonotoneCHT<true> hi;
        assert(!lo.query(0) && !hi.query_monotone(-100));
        std::vector<std::pair<long long, long long>> lines;
        long long slope = 1000, x = -100;
        for (int i = 0; i < 200; ++i) {
            if (rng() % 2 || lines.empty()) {
                slope -= rng() % 3;
                long long b = int(rng() % 20001) - 10000;
                lo.add_line(slope, b);
                hi.add_line(-slope, -b);
                lines.emplace_back(slope, b);
            } else {
                x += rng() % 5;
                W best = W(1) << 126;
                for (auto [a, b] : lines)
                    best = std::min(best, W(a) * x + b);
                assert(lo.query(x) == best && lo.query_monotone(x) == best && hi.query_monotone(x) == -best);
                long long arbitrary = static_cast<long long>(rng());
                W ref = W(1) << 126;
                for (auto [a, b] : lines)
                    ref = std::min(ref, W(a) * arbitrary + b);
                assert(lo.query(arbitrary) == ref && hi.query(arbitrary) == -ref);
            }
        }
    }
    const long long bound = (1LL << 62) - 1;
    MonotoneCHT<> h;
    h.add_line(bound, -bound);
    h.add_line(0, bound);
    h.add_line(-bound, -bound);
    for (long long x : {LLONG_MIN, 0LL, LLONG_MAX}) {
        W expected = std::min({W(bound) * x - bound, W(bound), -W(bound) * x - bound});
        assert(h.query(x) == expected);
    }
}
void subset() {
    std::mt19937 rng(73);
    for (int k = 0; k <= 9; ++k)
        for (int t = 0; t < 8; ++t) {
            int n = 1 << k;
            std::vector<M> a(n), b(n), expected(n);
            for (auto &x : a)
                x = int(rng() % 11) - 5;
            for (auto &x : b)
                x = int(rng() % 11) - 5;
            for (int s = 0; s < n; ++s)
                for (int v = s;; v = (v - 1) & s) {
                    expected[s] += a[v] * b[s ^ v];
                    if (!v)
                        break;
                }
            assert(subset_convolution(a, b) == expected);
        }
    assert(subset_convolution(std::vector<M>{}, {}).empty());
    std::vector<M> a(1 << 15, 1);
    auto c = subset_convolution(a, a);
    for (std::size_t i = 0; i < c.size(); ++i)
        assert(c[i] == M(2).pow(std::popcount(i)));
}
int main() {
    recurrences();
    knuth();
    smawk_test();
    cht();
    subset();
}
