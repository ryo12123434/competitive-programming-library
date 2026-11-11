#define PROBLEM "https://judge.yosupo.jp/problem/find_linear_recurrence"
#include "../../library/math/berlekamp_massey.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<M> a(n);
    for (auto &x : a) {
        int v;
        std::cin >> v;
        x = v;
    }
    auto c = berlekamp_massey(a);
    std::cout << c.size() << "\n";
    for (auto x : c)
        std::cout << x.val() << " ";
    std::cout << "\n";
}
