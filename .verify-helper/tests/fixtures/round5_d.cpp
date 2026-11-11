#include "library/math/discrete_log.hpp"
#include "library/math/segmented_sieve.hpp"
#include "library/math/fps_sqrt.hpp"
#include "library/math/multipoint_evaluation.hpp"
#include "library/math/polynomial_interpolation.hpp"
#include "library/math/gauss_elimination_mod.hpp"
#include "library/math/formal_power_series.hpp"
#include <atcoder/modint>
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <numeric>
#include <random>
#include <set>
#include <vector>
using M = atcoder::modint998244353;
using P = FormalPowerSeries<M>;
std::mt19937_64 rng(5004);
template <class F> void invalid(F f) {
    bool ok = false;
    try {
        f();
    } catch (const std::invalid_argument &) {
        ok = true;
    }
    assert(ok);
}
template <class F> void too_large(F f) {
    bool ok = false;
    try {
        f();
    } catch (const std::length_error &) {
        ok = true;
    }
    assert(ok);
}
void logs() {
    for (std::uint64_t m = 1; m <= 120; ++m)
        for (std::uint64_t a = 0; a < m; ++a) {
            std::vector<std::optional<std::uint64_t>> brute(m);
            std::uint64_t v = 1 % m, e = 0;
            while (!brute[v]) {
                brute[v] = e++;
                v = v * a % m;
            }
            for (std::uint64_t b = 0; b < m; ++b)
                assert(discrete_log_u64(a, b, m) == brute[b]);
        }
    invalid([] { discrete_log_u64(2, 1, 0); });
    too_large([] { discrete_log_u64(2, 3, 1'000'000'007, 3); });
    assert(discrete_log_u64(0, 0, UINT64_MAX) == 1);
    assert(discrete_log_u64(UINT64_MAX - 1, UINT64_MAX - 1, UINT64_MAX) == 1);
    assert(discrete_log_u64(2, 0, 1ULL << 63) == 63);
    assert(!discrete_log_u64(2, 3, 1ULL << 63));
    auto x = discrete_log_u64(2, 123456789, 1'000'000'007);
    assert(x && cpl::number_theory_detail::pow_mod(2, *x, 1'000'000'007) == 123456789);
    // uint64_t全域の逆元を、128bit拡張Euclidと剰余積で照合する。
    for (int i = 0; i < 10000; ++i) {
        auto m = rng() | 1, a = rng() % m;
        if (std::gcd(a, m) == 1 && m > 1)
            assert(cpl::number_theory_detail::mul_mod(a, cpl::discrete_log_detail::inverse(a, m), m) == 1);
    }
}
void sieve() {
    auto plain = eratosthenes(10000);
    for (int i = 0; i < 2000; ++i) {
        std::int64_t l = static_cast<int>(rng() % 11000) - 1000, r = static_cast<int>(rng() % 11000) - 1000;
        std::vector<std::int64_t> expected;
        for (auto j = std::max<std::int64_t>(l, 2); j <= r; ++j)
            if (plain[j])
                expected.push_back(j);
        assert(segmented_sieve(l, r) == expected);
    }
    assert((segmented_sieve(INT64_MIN, 2) == std::vector<std::int64_t>{2}));
    assert(segmented_sieve(INT64_MIN, -1).empty());
    assert(segmented_sieve(100, 2).empty());
    too_large([] { segmented_sieve(2, 100, 2); });
    too_large([] { segmented_sieve(2, 100, 100, 2); });
    too_large([] { segmented_sieve(INT64_MAX - 10, INT64_MAX); });
    auto primes = segmented_sieve(999'999'900'000LL, 1'000'000'000'000LL);
    std::set<std::int64_t> actual(primes.begin(), primes.end());
    for (std::int64_t j = 999'999'900'000LL; j <= 1'000'000'000'000LL; ++j)
        assert(actual.contains(j) == is_prime_u64(j));
    assert(segmented_sieve(0, 1'000'000).size() == 78498);
}
void roots() {
    assert(fps_sqrt(P{}, 0) == P{});
    assert(fps_sqrt(P{}, 6) == P(6));
    assert(!fps_sqrt(P{0, 1}, 5));
    assert(!fps_sqrt(P{3}, 1));
    assert(fps_sqrt(P{0, 0, 4}, 4) == P({0, 2, 0, 0}));
    invalid([] { fps_sqrt(P{}, -1); });
    for (int n = 1; n <= 140; ++n)
        for (int trial = 0; trial < 8; ++trial) {
            P g(n);
            for (auto &v : g)
                v = rng();
            if (trial % 3 == 0)
                for (int j = 0; j < n / 3; ++j)
                    g[j] = 0;
            P square = (g * g).pre(n);
            auto root = fps_sqrt(square, n);
            assert(root && root->size() == static_cast<std::size_t>(n));
            P out = ((*root) * (*root)).pre(n);
            out.resize(n);
            assert(out == square);
        }
    using F2 = FormalPowerSeries<atcoder::static_modint<2>>;
    for (unsigned mask = 0; mask < 1024; ++mask) {
        F2 f(10);
        for (int j = 0; j < 10; ++j)
            f[j] = (mask >> j) & 1;
        bool ok = true;
        for (int j = 1; j < 10; j += 2)
            if (f[j] != 0)
                ok = false;
        auto g = fps_sqrt(f, 10);
        assert(g.has_value() == ok);
        if (g)
            for (int j = 0; j < 10; ++j)
                assert(f[j] == (j % 2 ? 0 : (*g)[j / 2]));
    }
    P g(32768);
    for (auto &v : g)
        v = rng();
    auto f = (g * g).pre(g.size());
    auto h = fps_sqrt(f, f.size());
    assert(h && ((*h) * (*h)).pre(f.size()) == f);
}
void points() {
    assert(multipoint_evaluation(P{}, {}).empty());
    assert(polynomial_interpolation<P>({}, {}) == P{});
    assert(!polynomial_interpolation<P>({1, 1}, {2, 2}));
    invalid([] { polynomial_interpolation<P>({1}, {1, 2}); });
    for (int n = 0; n < 110; ++n) {
        P f(rng() % 150);
        for (auto &v : f)
            v = rng();
        std::vector<M> xs(n);
        for (auto &v : xs)
            v = rng() % 20;
        auto got = multipoint_evaluation(f, xs);
        for (int i = 0; i < n; ++i)
            assert(got[i] == f.eval(xs[i]));
        std::iota(xs.begin(), xs.end(), M(0));
        P orig(n);
        for (auto &v : orig)
            v = rng();
        PolynomialProductTree<P> tree(xs);
        got = tree.evaluate(orig);
        auto restored = tree.interpolate(got);
        assert(restored == orig);
        for (int i = 0; i < n; ++i)
            assert(tree.product().eval(xs[i]) == M(0));
    }
    std::vector<M> xs(8192);
    std::iota(xs.begin(), xs.end(), M(0));
    P f(8192);
    for (auto &v : f)
        v = rng();
    PolynomialProductTree<P> tree(xs);
    auto ys = tree.evaluate(f);
    assert(tree.interpolate(ys) == f);
}
template <int Mod> void systems() {
    using T = atcoder::static_modint<Mod>;
    for (int trial = 0; trial < 1500; ++trial) {
        int n = rng() % 6, m = rng() % 7;
        std::vector<std::vector<T>> a(m, std::vector<T>(n));
        std::vector<T> b(m);
        ModLinearSystem<T> system(n);
        for (int i = 0; i < m; ++i) {
            for (auto &v : a[i])
                v = rng();
            b[i] = rng();
            system.add_equation(a[i], b[i]);
        }
        std::set<std::vector<int>> brute;
        int limit = 1;
        for (int i = 0; i < n; ++i)
            limit *= Mod;
        for (int mask = 0; mask < limit; ++mask) {
            int state = mask;
            std::vector<int> x(n);
            for (auto &v : x) {
                v = state % Mod;
                state /= Mod;
            }
            bool ok = true;
            for (int i = 0; i < m; ++i) {
                T sum = 0;
                for (int j = 0; j < n; ++j)
                    sum += a[i][j] * x[j];
                if (sum != b[i])
                    ok = false;
            }
            if (ok)
                brute.insert(x);
        }
        auto result = system.solve(true);
        assert(result.solvable == !brute.empty());
        assert(result.rank == matrix_rank_mod(a));
        assert(result.rank + result.free_variables == n);
        if (!result.solvable) {
            assert(result.solution.empty() && result.nullspace.empty());
            continue;
        }
        assert(result.nullspace.size() == static_cast<std::size_t>(result.free_variables));
        std::set<std::vector<int>> generated;
        int combinations = 1;
        for (int i = 0; i < result.free_variables; ++i)
            combinations *= Mod;
        for (int mask = 0; mask < combinations; ++mask) {
            int state = mask;
            auto x = result.solution;
            for (const auto &v : result.nullspace) {
                T coefficient = state % Mod;
                state /= Mod;
                for (int j = 0; j < n; ++j)
                    x[j] += coefficient * v[j];
            }
            std::vector<int> value;
            for (auto v : x)
                value.push_back(v.val());
            generated.insert(value);
        }
        assert(generated == brute);
    }
}
void gauss() {
    systems<2>();
    systems<3>();
    for (int n = 0; n <= 7; ++n)
        for (int trial = 0; trial < 30; ++trial) {
            std::vector<std::vector<M>> a(n, std::vector<M>(n));
            for (auto &row : a)
                for (auto &v : row)
                    v = rng() % 5;
            std::vector<int> p(n);
            std::iota(p.begin(), p.end(), 0);
            M want = 0;
            do {
                int sign = 0;
                M term = 1;
                for (int i = 0; i < n; ++i) {
                    term *= a[i][p[i]];
                    for (int j = 0; j < i; ++j)
                        sign ^= p[j] > p[i];
                }
                want += sign ? -term : term;
            } while (std::next_permutation(p.begin(), p.end()));
            assert(determinant_mod(a) == want);
        }
    invalid([] { ModLinearSystem<M>(-1); });
    invalid([] { ModLinearSystem<atcoder::static_modint<4>>(1); });
    invalid([] {
        ModLinearSystem<M> s(2);
        s.add_equation({1}, 0);
    });
    invalid([] { determinant_mod(std::vector<std::vector<M>>{{1, 2}}); });
    std::vector<std::vector<M>> a(200, std::vector<M>(200));
    for (int i = 0; i < 200; ++i) {
        a[i][i] = 1;
        for (int j = i + 1; j < 200; ++j)
            a[i][j] = rng();
    }
    assert(determinant_mod(a) == M(1));
    assert(matrix_rank_mod(a) == 200);
}
int main() {
    logs();
    sieve();
    roots();
    points();
    gauss();
}
