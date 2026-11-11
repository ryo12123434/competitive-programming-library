#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations"
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
    ModLinearSystem<M> system(m);
    for (auto &row : a) {
        int x;
        std::cin >> x;
        system.add_equation(std::move(row), M(x));
    }
    auto result = system.solve(true);
    if (!result.solvable) {
        std::cout << -1 << '\n';
        return 0;
    }
    std::cout << result.free_variables << '\n';
    for (auto x : result.solution)
        std::cout << x.val() << ' ';
    std::cout << '\n';
    for (auto &row : result.nullspace) {
        for (auto x : row)
            std::cout << x.val() << ' ';
        std::cout << '\n';
    }
}
