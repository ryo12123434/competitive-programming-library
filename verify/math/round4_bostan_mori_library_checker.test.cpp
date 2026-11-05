#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "../../library/math/bostan_mori.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
#include "../../library/math/formal_power_series.hpp"
using P = FormalPowerSeries<M>;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int k;
    std::uint64_t n;
    std::cin >> k >> n;
    P a(k), q(k + 1);
    q[0] = 1;
    for (auto &x : a) {
        int v;
        std::cin >> v;
        x = v;
    }
    for (int i = 1; i <= k; ++i) {
        int v;
        std::cin >> v;
        q[i] = -M(v);
    }
    P p = a * q;
    p.resize(k);
    std::cout << bostan_mori(p, q, n).val() << "\n";
}
