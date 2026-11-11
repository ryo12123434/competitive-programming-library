#include "library/math/miller_rabin.hpp"
#include "library/math/pollard_rho.hpp"
#include "library/math/multiplicative_functions.hpp"
#include "library/math/subset_transform.hpp"
#include "library/math/xor_convolution.hpp"
#include "library/math/mod_sqrt.hpp"

#include <atcoder/modint>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
using U = std::uint64_t;
using W = __uint128_t;
using M = atcoder::modint998244353;
bool trial(U n) {
    if (n < 2)
        return false;
    for (U d = 2; d <= n / d; ++d)
        if (n % d == 0)
            return false;
    return true;
}
void primes() {
    for (U n = 0; n < 100000; ++n)
        assert(is_prime_u64(n) == trial(n));
    for (U n : {561ULL, 1105ULL, 1729ULL, 2465ULL, 2821ULL, 6601ULL, 3215031751ULL, 341550071728321ULL,
                3825123056546413051ULL, 18446744073709551615ULL})
        assert(!is_prime_u64(n));
    for (U n : {2ULL, 3ULL, 4294967291ULL, 18446744073709551557ULL})
        assert(is_prime_u64(n));
    std::mt19937_64 rng(441);
    for (int t = 0; t < 500; ++t) {
        U n = rng() % 1000000000;
        assert(is_prime_u64(n) == trial(n));
    }
    auto check = [](U n, U seed) {
        auto f = factorize_u64(n, seed);
        W product = 1;
        for (auto p : f) {
            assert(is_prime_u64(p));
            product *= p;
        }
        assert(product == n && std::is_sorted(f.begin(), f.end()));
        assert(f == factorize_u64(n, seed));
    };
    for (U n = 1; n < 2000; ++n)
        check(n, 42);
    for (U n : {18446744073709551615ULL, 18446744073709551557ULL, 18446744030759878681ULL,
                9223372036854775808ULL, 12157665459056928801ULL, 1000000016000000063ULL})
        for (U seed : {0, 1, 13})
            check(n, seed);
    for (int t = 0; t < 100; ++t)
        check((rng() % 4000000000 + 1) * (rng() % 4000000000 + 1), rng());
    bool caught = false;
    try {
        factorize_u64(0);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    caught = false;
    try {
        factorize_u64(4, 1, 0);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    MultiplicativeFunctions empty(0), f(3000);
    assert(empty.phi == std::vector<int>{0});
    for (int n = 1; n <= 3000; ++n) {
        int phi = 0;
        for (int i = 1; i <= n; ++i)
            phi += std::gcd(i, n) == 1;
        assert(f.phi[n] == phi && euler_phi_u64(n) == U(phi));
        int sum = 0;
        for (int d = 1; d <= n; ++d)
            if (n % d == 0)
                sum += f.mu[d];
        assert(sum == (n == 1));
        assert(mobius_u64(n) == f.mu[n]);
    }
}
void transforms() {
    std::mt19937 rng(556);
    for (int bits = 0; bits <= 8; ++bits)
        for (int t = 0; t < 20; ++t) {
            int n = 1 << bits;
            std::vector<M> a(n), b(n);
            for (auto &x : a)
                x = int(rng() % 21) - 10;
            for (auto &x : b)
                x = int(rng() % 21) - 10;
            auto z = a;
            subset_zeta(z);
            for (int s = 0; s < n; ++s) {
                M sum = 0;
                for (int u = s;; u = (u - 1) & s) {
                    sum += a[u];
                    if (!u)
                        break;
                }
                assert(sum == z[s]);
            }
            subset_mobius(z);
            assert(z == a);
            z = a;
            superset_zeta(z);
            for (int s = 0; s < n; ++s) {
                M sum = 0;
                for (int u = 0; u < n; ++u)
                    if ((u & s) == s)
                        sum += a[u];
                assert(sum == z[s]);
            }
            superset_mobius(z);
            assert(z == a);
            std::vector<M> expected_or(n), expected_and(n), expected_xor(n);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j) {
                    expected_or[i | j] += a[i] * b[j];
                    expected_and[i & j] += a[i] * b[j];
                    expected_xor[i ^ j] += a[i] * b[j];
                }
            assert(or_convolution(a, b) == expected_or && and_convolution(a, b) == expected_and &&
                   xor_convolution(a, b) == expected_xor);
            z = a;
            walsh_hadamard_transform(z);
            walsh_hadamard_transform(z, true);
            assert(z == a);
        }
    assert(or_convolution(std::vector<int>{}, {}).empty());
    assert(xor_convolution(std::vector<M>{}, {}).empty());
    std::vector<long long> a{1, -2, 3, 4}, copy = a;
    walsh_hadamard_transform(a);
    walsh_hadamard_transform(a, true);
    assert(a == copy);
    bool caught = false;
    std::vector<atcoder::static_modint<2>> bad{1, 0};
    try {
        walsh_hadamard_transform(bad, true);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    std::vector<M> large(1 << 18, 1);
    subset_zeta(large);
    subset_mobius(large);
    assert(large[12345] == 1);
}
void roots() {
    for (U p = 2; p <= 500; ++p)
        if (trial(p))
            for (U a = 0; a < p; ++a) {
                std::optional<U> expected;
                for (U x = 0; x < p; ++x)
                    if (x * x % p == a) {
                        expected = x;
                        break;
                    }
                assert(mod_sqrt_u64(a, p) == expected);
            }
    std::mt19937_64 rng(535);
    for (U p : {2ULL, 998244353ULL, 1000000007ULL, 18446744073709551557ULL})
        for (int t = 0; t < 200; ++t) {
            U x = rng() % p;
            U a = W(x) * x % p;
            auto root = mod_sqrt_u64(a, p);
            assert(root && W(*root) * (*root) % p == a && *root <= p - *root);
        }
    bool caught = false;
    try {
        mod_sqrt_u64(1, 15);
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
}
int main() {
    primes();
    transforms();
    roots();
}
