#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_of_formal_power_series"
#include "../../library/math/formal_power_series.hpp"
#include "../../library/math/fps_sqrt.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
using P = FormalPowerSeries<M>;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    P f(n);
    for (auto &v : f) {
        int x;
        std::cin >> x;
        v = x;
    }
    auto g = fps_sqrt(f, n);
    if (!g) {
        std::cout << -1 << '\n';
        return 0;
    }
    for (auto x : *g)
        std::cout << x.val() << ' ';
    std::cout << '\n';
}
