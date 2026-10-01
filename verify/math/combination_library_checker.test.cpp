#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod"

// Problem: Library Checker - Binomial Coefficient (Prime Mod)
// Target: library/math/combination.hpp

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include <atcoder/modint>

#include "../../library/math/combination.hpp"

using mint = atcoder::dynamic_modint<0>;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int t, mod;
    std::cin >> t >> mod;

    std::vector<std::pair<int, int>> queries(t);
    int max_n = 0;

    for (auto& [n, k] : queries) {
        std::cin >> n >> k;
        max_n = std::max(max_n, n);
    }

    mint::set_mod(mod);
    Combination<mint> comb(max_n);

    for (const auto& [n, k] : queries) {
        std::cout << comb.com(n, k).val() << '\n';
    }
}
