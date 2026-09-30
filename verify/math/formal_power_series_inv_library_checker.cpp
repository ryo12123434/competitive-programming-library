// Problem: Library Checker - Inv of Formal Power Series
// URL: https://judge.yosupo.jp/problem/inv_of_formal_power_series
// Target: library/math/formal_power_series.hpp

#include <iostream>

#include <atcoder/modint>

#include "../../library/math/formal_power_series.hpp"

using mint = atcoder::modint998244353;
using FPS = FormalPowerSeries<mint>;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;

    FPS f(n);
    for (auto& x : f) {
        int v;
        std::cin >> v;
        x = v;
    }

    FPS g = inv(f, n);

    for (int i = 0; i < n; ++i) {
        if (i) std::cout << ' ';
        std::cout << g[i].val();
    }
    std::cout << '\n';
}
