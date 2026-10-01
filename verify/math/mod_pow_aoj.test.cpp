#define PROBLEM "http://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=NTL_1_B"

// Problem: AOJ NTL_1_B - Power
// Target: library/math/mod_pow.hpp

#include <iostream>

#include "../../library/math/mod_pow.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    long long m, n;
    std::cin >> m >> n;

    constexpr long long MOD = 1000000007LL;
    std::cout << modpow(m, n, MOD) << '\n';
}
