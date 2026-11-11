#define PROBLEM "https://judge.yosupo.jp/problem/bitwise_and_convolution"
#include "../../library/math/subset_transform.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int k;
    std::cin >> k;
    std::vector<M> a(1 << k), b(a.size());
    for (auto &x : a) {
        int v;
        std::cin >> v;
        x = v;
    }
    for (auto &x : b) {
        int v;
        std::cin >> v;
        x = v;
    }
    auto c = and_convolution(a, b);
    for (auto x : c)
        std::cout << x.val() << " ";
    std::cout << "\n";
}
