#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 整数演算の端点と、篩・素因数分解・基数変換・組合せを独立な参照実装で検証する。
#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "../../library/math/base_conversion.hpp"
#include "../../library/math/combination.hpp"
#include "../../library/math/divisors.hpp"
#include "../../library/math/eratosthenes.hpp"
#include "../../library/math/linear_sieve.hpp"
#include "../../library/math/mod_pow.hpp"
#include "../../library/math/prime_factorize.hpp"

long long reference_pow(long long a, int n, long long mod) {
    __int128 result = 1 % mod;
    __int128 value = (static_cast<__int128>(a) % mod + mod) % mod;
    for (int i = 0; i < n; ++i) result = result * value % mod;
    return static_cast<long long>(result);
}

int main() {
    assert(modpow(2, 0, 1) == 0);
    assert(modpow(0, 0, 7) == 1);
    assert(modpow(-2, 3, 7) == 6);
    assert(modpow(LLONG_MIN, 1, LLONG_MAX) == LLONG_MAX - 1);
    assert(modpow(LLONG_MAX - 1, 2, LLONG_MAX) == 1);
    assert(modpow(LLONG_MAX - 1, LLONG_MAX, LLONG_MAX) == LLONG_MAX - 1);
    std::mt19937_64 rng(1709);
    for (int trial = 0; trial < 1000; ++trial) {
        long long a = static_cast<long long>(rng() >> 1);
        if (rng() & 1) a = -a;
        long long mod = 1 + static_cast<long long>(rng() % LLONG_MAX);
        int exponent = rng() % 31;
        assert(modpow(a, exponent, mod) == reference_pow(a, exponent, mod));
    }
    assert(eratosthenes(0) == std::vector<bool>{false});
    assert(LinearSieve(0).primes.empty());
    constexpr int N = 1000;
    auto sieve = eratosthenes(N);
    LinearSieve linear(N);
    for (int n = 1; n <= N; ++n) {
        bool prime = n >= 2;
        std::vector<long long> divisors;
        for (int d = 1; d <= n; ++d) {
            if (n % d == 0) divisors.push_back(d);
            if (1 < d && d < n && n % d == 0) prime = false;
        }
        assert(sieve[n] == prime);
        assert(calc_divisors(n) == divisors);
        auto factors = prime_factorize(n);
        auto small = linear.factorize(n);
        assert(factors.size() == small.size());
        long long product = 1;
        for (int i = 0; i < static_cast<int>(factors.size()); ++i) {
            auto [p, count] = factors[i];
            assert(prime_factorize(p) == (std::vector<std::pair<long long, long long>>{{p, 1}}));
            assert(p == small[i].first && count == small[i].second);
            for (int k = 0; k < count; ++k) product *= p;
        }
        assert(product == n);
    }
    for (int base = 2; base <= 36; ++base) {
        for (long long n : {0LL, 1LL, 35LL, 123456789LL, LLONG_MAX}) {
            assert(from_base(to_base(n, base), base) == n);
            auto digits = to_digits(n, base), reverse = to_digits_rev(n, base);
            std::reverse(reverse.begin(), reverse.end());
            assert(digits == reverse);
            __int128 value = 0;
            for (int digit : digits) { assert(0 <= digit && digit < base); value = value * base + digit; }
            assert(value == n);
        }
    }
    assert(from_base("z", 36) == 35);
    using mint = atcoder::modint998244353;
    Combination<mint> empty_combination(0);
    assert(empty_combination.com(0, 0) == 1 && empty_combination.permutation(0, 0) == 1);
    Combination<mint> combination(50);
    std::vector<std::vector<mint>> pascal(51, std::vector<mint>(51));
    pascal[0][0] = 1;
    for (int n = 1; n <= 50; ++n) {
        pascal[n][0] = 1;
        for (int k = 1; k <= n; ++k) pascal[n][k] = pascal[n - 1][k - 1] + pascal[n - 1][k];
    }
    for (int n = 0; n <= 50; ++n) for (int k = 0; k <= n; ++k) {
        assert(combination.com(n, k) == pascal[n][k]);
        mint product = 1;
        for (int i = 0; i < k; ++i) product *= n - i;
        assert(combination.permutation(n, k) == product);
    }
    assert(combination.com(-1, 0) == 0 && combination.permutation(1, 2) == 0);
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
