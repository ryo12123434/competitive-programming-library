#define PROBLEM "https://judge.yosupo.jp/problem/matrix_rank"
#include "../../library/math/gauss_elimination_mod.hpp"
#include <iostream>
#include <atcoder/modint>
using M = atcoder::modint998244353;
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    std::vector<std::vector<M>> a(n, std::vector<M>(m));
    for (auto &row : a)
        for (auto &v : row) {
            int x;
            std::cin >> x;
            v = x;
        }
    std::cout << matrix_rank_mod(a) << '\n';
}
