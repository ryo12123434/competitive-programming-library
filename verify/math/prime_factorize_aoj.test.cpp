#define PROBLEM "http://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=NTL_1_A"

// Problem: AOJ NTL_1_A - Prime Factorize
// Target: library/math/prime_factorize.hpp

#include <iostream>

#include "../../library/math/prime_factorize.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    long long n;
    std::cin >> n;

    std::cout << n << ':';
    for (const auto& [p, e] : prime_factorize(n)) {
        for (long long i = 0; i < e; ++i) {
            std::cout << ' ' << p;
        }
    }
    std::cout << '\n';
}
