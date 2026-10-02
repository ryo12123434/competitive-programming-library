#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 多項式の素朴な積・長除法と、微積分・逆元・log/exp・pow の恒等式を検証する。
#include <cassert>
#include <climits>
#include <iostream>
#include <random>
#include <atcoder/modint>
#include "../../library/math/formal_power_series.hpp"

using mint = atcoder::modint998244353;
using FPS = FormalPowerSeries<mint>;

FPS multiply(const FPS& a, const FPS& b) {
    if (a.empty() || b.empty()) return {};
    FPS result(a.size() + b.size() - 1);
    for (int i = 0; i < static_cast<int>(a.size()); ++i)
        for (int j = 0; j < static_cast<int>(b.size()); ++j) result[i + j] += a[i] * b[j];
    return result;
}

void equal(FPS a, FPS b) { assert(a.normalize() == b.normalize()); }

int main() {
    FPS alias{2, 3, 4};
    alias *= alias[0];
    assert(alias == (FPS{4, 6, 8}));
    FPS zeros(10);
    equal(exp(zeros, 10), FPS{1});
    assert(inv(FPS{}, 0).empty() && log(FPS{}, 0).empty() && exp(FPS{}, 0).empty());
    assert(pow(FPS{}, 0, 0).empty());
    equal(pow(FPS{0, 1}, LLONG_MAX, 10), FPS{});
    equal(pow(FPS{1}, LLONG_MAX, 10), FPS{1});
    assert(integrate(FPS{}).size() == 1);
    // 小さな法でも ACL の畳み込み長制約を満たす範囲で検証する。
    using SmallFPS = FormalPowerSeries<atcoder::static_modint<17>>;
    SmallFPS small{1, 2, 3, 4, 5};
    assert(exp(log(small, 5), 5) == small);
    FPS long_prefix(100, 0);
    long_prefix[80] = 3;
    equal(pow(long_prefix, 1, 5), FPS{});
    std::mt19937 rng(1710);
    for (int trial = 0; trial < 150; ++trial) {
        int n = 1 + rng() % 25;
        FPS a(n), b(1 + rng() % 15);
        for (auto& x : a) x = rng() % 30;
        for (auto& x : b) x = rng() % 30;
        b.back() = 1 + rng() % 30;
        equal(a * b, multiply(a, b));
        equal(diff(integrate(a)), a);
        for (int i = 0; i < n; ++i) assert(integrate(a)[i + 1] == a[i] / mint(i + 1));

        FPS remainder = a;
        remainder.normalize();
        FPS quotient(remainder.size() >= b.size() ? remainder.size() - b.size() + 1 : 0);
        while (remainder.size() >= b.size()) {
            int shift = static_cast<int>(remainder.size() - b.size());
            mint coefficient = remainder.back() / b.back();
            quotient[shift] = coefficient;
            for (int i = 0; i < static_cast<int>(b.size()); ++i) remainder[shift + i] -= coefficient * b[i];
            remainder.normalize();
        }
        equal(a / b, quotient);
        equal(a % b, remainder);
        FPS self = b;
        self /= self;
        equal(self, FPS{1});
        self = b;
        self %= self;
        assert(self.empty());
        auto divisor = gcd(a, b);
        if (!divisor.empty()) { assert((a % divisor).empty() && (b % divisor).empty()); assert(divisor.back() == 1); }

        a[0] = 1;
        FPS identity(n);
        identity[0] = 1;
        assert((multiply(a, inv(a, n))).pre(n) == identity);
        equal(exp(log(a, n), n), a);
        int degree = 1 + rng() % n;
        equal(log(a, degree), log(a.pre(degree), degree));
        a[0] = 0;
        equal(log(exp(a, n), n), a);
        int exponent = rng() % 8;
        FPS power{1};
        for (int k = 0; k < exponent; ++k) power = multiply(power, a).pre(n);
        equal(pow(a, exponent, n), power);
        equal(pow(a, exponent, degree), pow(a.pre(degree), exponent, degree));
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
