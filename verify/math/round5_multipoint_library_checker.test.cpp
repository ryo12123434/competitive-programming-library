#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include "../../library/math/formal_power_series.hpp"
#include "../../library/math/multipoint_evaluation.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
using P = FormalPowerSeries<M>;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    P f(n);
    std::vector<M> xs(m);
    for (auto &v : f) {
        int x;
        std::cin >> x;
        v = x;
    }
    for (auto &v : xs) {
        int x;
        std::cin >> x;
        v = x;
    }
    for (auto x : multipoint_evaluation(f, xs))
        std::cout << x.val() << ' ';
    std::cout << '\n';
}
