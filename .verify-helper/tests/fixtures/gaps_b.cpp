#include "library/algorithm/slope_trick.hpp"
#include "library/algorithm/monotone_minima.hpp"
#include "library/algorithm/divide_and_conquer_dp.hpp"
#include "library/math/gauss_elimination_gf2.hpp"
#include "library/algorithm/knapsack.hpp"
#include <climits>
#include <random>
#include <set>
#include <string>
using Wide = __int128_t;
void slope() {
    std::mt19937 rng(725);
    constexpr int B = 500;
    const Wide INF = Wide(1) << 100;
    for (int trial = 0; trial < 70; ++trial) {
        SlopeTrick f;
        std::vector<Wide> values(B * 2 + 1);
        for (int step = 0; step < 50; ++step) {
            int op = rng() % 9, a = static_cast<int>(rng() % 13) - 6;
            auto next = values;
            if (op <= 3) {
                if (op == 0)
                    f.add_constant(a);
                if (op == 1)
                    f.add_x_minus_a(a);
                if (op == 2)
                    f.add_a_minus_x(a);
                if (op == 3)
                    f.add_abs(a);
                for (int x = -B; x <= B; ++x)
                    next[x + B] += op == 0   ? a
                                   : op == 1 ? std::max(0, x - a)
                                   : op == 2 ? std::max(0, a - x)
                                             : std::abs(x - a);
            }
            if (op == 4) {
                f.prefix_min();
                for (int i = 1; i <= 2 * B; ++i)
                    next[i] = std::min(next[i], next[i - 1]);
            }
            if (op == 5) {
                f.suffix_min();
                for (int i = 2 * B; i-- > 0;)
                    next[i] = std::min(next[i], next[i + 1]);
            }
            if (op >= 6) {
                int l = static_cast<int>(rng() % 5) - 2, r = static_cast<int>(rng() % 5) - 2;
                if (l > r)
                    std::swap(l, r);
                if (op == 6) {
                    r = l;
                    f.shift(l);
                } else
                    f.slide_min(l, r);
                for (int x = -B; x <= B; ++x) {
                    next[x + B] = INF;
                    for (int y = x - r; y <= x - l; ++y)
                        if (-B <= y && y <= B)
                            next[x + B] = std::min(next[x + B], values[y + B]);
                }
            }
            values = next;
            assert(f.minimum() == *std::min_element(values.begin(), values.end()));
            for (int x = -12; x <= 12; ++x)
                assert(f.eval(x) == values[x + B]);
            auto [l, r] = f.argmin();
            if (l && r)
                assert(*l <= *r);
            if (l)
                assert(f.eval(*l) == f.minimum());
            if (r)
                assert(f.eval(*r) == f.minimum());
        }
    }
    SlopeTrick huge;
    huge.add_abs(LLONG_MIN);
    huge.add_abs(LLONG_MAX);
    assert(huge.minimum() == Wide(LLONG_MAX) - LLONG_MIN);
    huge.shift(LLONG_MAX);
    assert(huge.eval(0) == huge.minimum());
}
void optimizations() {
    std::mt19937 rng(7224);
    for (int h = 0; h < 45; ++h)
        for (int w = 0; w < 35; ++w) {
            std::vector<int> target(h);
            for (int &i : target)
                i = w ? rng() % w : 0;
            std::sort(target.begin(), target.end());
            std::size_t calls = 0;
            auto cost = [&](int i, int j) {
                ++calls;
                long long d = j - target[i];
                return d * d;
            };
            auto r = monotone_minima(h, w, cost);
            assert(calls == r.evaluations);
            for (int i = 0; i < h; ++i)
                assert(r.argmin[i] == (w ? target[i] : -1));
            auto ties = monotone_minima(h, w, [](int, int) { return 0; });
            for (int i : ties.argmin)
                assert(i == (w ? 0 : -1));
            auto maximum = monotone_minima(
                h, w, [&](int i, int j) { return -(j - target[i]) * (j - target[i]); }, std::greater<>{});
            assert(maximum.argmin == r.argmin);
        }
    for (int trial = 0; trial < 700; ++trial) {
        int n = trial % 60;
        std::vector<long long> prefix(n + 1);
        std::vector<std::optional<long long>> prev(n + 1);
        for (int i = 0; i <= n; ++i) {
            if (rng() % 3)
                prev[i] = static_cast<int>(rng() % 31) - 10;
            if (i)
                prefix[i] = prefix[i - 1] + rng() % 4;
        }
        std::size_t calls = 0;
        auto cost = [&](int k, int j) {
            ++calls;
            auto d = prefix[j] - prefix[k];
            return d * d;
        };
        auto r = divide_and_conquer_dp(prev, cost);
        assert(calls == r.evaluations);
        for (int j = 0; j <= n; ++j) {
            std::optional<long long> best;
            int arg = -1;
            for (int k = 0; k < j; ++k)
                if (prev[k]) {
                    auto value = *prev[k] + cost(k, j);
                    if (!best || value < *best) {
                        best = value;
                        arg = k;
                    }
                }
            assert(r.value[j] == best && r.argmin[j] == arg);
        }
    }
}
void gf2() {
    std::mt19937 rng(832);
    for (int trial = 0; trial < 700; ++trial) {
        int n = trial % 10, m = rng() % 14;
        GF2LinearSystem system(n);
        std::vector<unsigned> a(m);
        std::vector<int> b(m);
        for (int i = 0; i < m; ++i) {
            a[i] = rng() & ((1u << n) - 1);
            b[i] = rng() % 2;
            std::vector<int> ones;
            for (int j = 0; j < n; ++j)
                if (a[i] >> j & 1)
                    ones.push_back(j);
            system.add_equation(ones, b[i]);
        }
        auto r = system.solve(true);
        std::set<unsigned> solutions;
        int homogeneous = 0;
        for (unsigned x = 0; x < (1u << n); ++x) {
            bool valid = true, zero = true;
            for (int i = 0; i < m; ++i) {
                int parity = __builtin_popcount(a[i] & x) % 2;
                if (parity != b[i])
                    valid = false;
                if (parity)
                    zero = false;
            }
            if (valid)
                solutions.insert(x);
            homogeneous += zero;
        }
        assert(r.solvable == !solutions.empty() &&
               (1u << r.free_variables) == static_cast<unsigned>(homogeneous) &&
               r.rank + r.free_variables == n);
        if (r.solvable) {
            unsigned x = 0;
            for (int i = 0; i < n; ++i)
                x |= r.solution[i] << i;
            assert(solutions.contains(x) && r.nullspace.size() == static_cast<std::size_t>(r.free_variables));
            std::set<unsigned> span{x};
            for (auto &v : r.nullspace) {
                unsigned mask = 0;
                for (int i = 0; i < n; ++i)
                    mask |= v[i] << i;
                for (auto row : a)
                    assert(__builtin_popcount(row & mask) % 2 == 0);
                auto old = span;
                for (unsigned y : old)
                    span.insert(y ^ mask);
            }
            assert(span == solutions);
        } else
            assert(r.solution.empty() && r.nullspace.empty());
    }
    for (int n : {0, 1, 63, 64, 65, 127, 128, 129}) {
        GF2LinearSystem s(n);
        for (int i = 0; i < n; ++i) {
            std::vector<std::uint64_t> row((n + 63) / 64);
            row[i / 64] |= std::uint64_t(1) << (i % 64);
            if (n % 64)
                row.back() |= ~((std::uint64_t(1) << (n % 64)) - 1);
            s.add_row(row, i % 2);
        }
        auto r = s.solve(true);
        assert(r.solvable && r.rank == n && r.nullspace.empty());
        for (int i = 0; i < n; ++i)
            assert(r.solution[i] == i % 2);
    }
    GF2LinearSystem repeated(1);
    repeated.add_equation({0, 0, 0}, true);
    assert(repeated.solve().solution[0] == 1);
}
void knapsack() {
    std::mt19937 rng(5174);
    for (int trial = 0; trial < 500; ++trial) {
        int n = trial % 13, cap = rng() % 35;
        std::vector<KnapsackItem> items;
        std::vector<int> weights;
        std::vector<long long> values;
        for (int i = 0; i < n; ++i) {
            int w = rng() % 10, v = static_cast<int>(rng() % 24) - 8;
            items.push_back({w, v});
            weights.push_back(w);
            values.push_back(v);
        }
        SubsetSum ss(weights, cap);
        Wide best = 0;
        std::vector<bool> reachable(cap + 1);
        std::uint64_t count = 0;
        int lower = static_cast<int>(rng() % 41) - 20, upper = lower + rng() % 20;
        for (unsigned mask = 0; mask < (1u << n); ++mask) {
            int weight = 0;
            Wide value = 0;
            for (int i = 0; i < n; ++i)
                if (mask >> i & 1) {
                    weight += items[i].weight;
                    value += items[i].value;
                }
            if (weight <= cap) {
                reachable[weight] = true;
                best = std::max(best, value);
            }
            if (lower <= value && value < upper)
                ++count;
        }
        assert(knapsack_by_weight(items, cap) == best &&
               subset_sum_mitm_count(values, lower, upper) == count);
        for (int w = 0; w <= cap; ++w)
            assert(ss.reachable(w) == reachable[w]);
        assert(!ss.reachable(-1) && !ss.reachable(cap + 1));
        for (auto &item : items)
            item.value = std::max(0LL, item.value);
        assert(knapsack_by_value(items, cap) == knapsack_by_weight(items, cap));
    }
    assert(knapsack_by_weight({{0, LLONG_MAX}, {0, LLONG_MAX}}, 0) == Wide(LLONG_MAX) * 2);
    assert(knapsack_by_value({{LLONG_MAX, 1}, {LLONG_MAX, 1}}, LLONG_MAX) == 1);
    assert(subset_sum_mitm_count({LLONG_MIN, LLONG_MAX}, -1, 0) == 1);
    for (int n : {0, 1, 63, 64, 65, 127, 128}) {
        SubsetSum s({0, 1, 63, 64, 65}, n);
        std::set<int> expected{0};
        for (int w : {0, 1, 63, 64, 65}) {
            auto old = expected;
            for (int x : old)
                if (x + w <= n)
                    expected.insert(x + w);
        }
        for (int i = 0; i <= n; ++i)
            assert(s.reachable(i) == expected.contains(i));
    }
}
void large() {
    SlopeTrick f;
    int n = 200000;
    for (int i = 0; i < n; ++i)
        f.add_abs(i);
    assert(f.minimum() == Wide(n) * n / 4);
    auto r = monotone_minima(n, n, [](int i, int j) {
        long long d = i - j;
        return d * d;
    });
    assert(r.argmin.back() == n - 1);
    std::vector<std::optional<long long>> prev(n + 1, 0);
    auto dp = divide_and_conquer_dp(prev, [](int k, int j) { return 1LL * (j - k) * (j - k); });
    assert(dp.value[n] == 1);
    GF2LinearSystem system(512);
    for (int i = 0; i < 512; ++i)
        system.add_equation({i}, i % 2);
    assert(system.solve().rank == 512);
    SubsetSum s(std::vector<int>(1000, 201), 200000);
    assert(s.reachable(20100));
    assert(subset_sum_mitm_count(std::vector<long long>(40), 0, 1) == (1ULL << 40));
}
int main(int argc, char **argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (mode == "all" || mode == "slope")
        slope();
    if (mode == "all" || mode == "optimizations")
        optimizations();
    if (mode == "all" || mode == "gf2")
        gf2();
    if (mode == "all" || mode == "knapsack")
        knapsack();
    if (mode == "large")
        large();
}
