#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include "../../library/math/formal_power_series.hpp"
#include "../../library/math/polynomial_interpolation.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
using P = FormalPowerSeries<M>;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<M> xs(n), ys(n);
    for (auto &v : xs) {
        int x;
        std::cin >> x;
        v = x;
    }
    for (auto &v : ys) {
        int x;
        std::cin >> x;
        v = x;
    }
    auto f = polynomial_interpolation<P>(xs, ys);
    if (!f)
        return 1;
    for (auto x : *f)
        std::cout << x.val() << ' ';
    std::cout << '\n';
}
