#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations_mod_2"
#include "../../library/math/gauss_elimination_gf2.hpp"
#include <iostream>
#include <string>
int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<std::string> a(n);
    for (auto &row : a)
        std::cin >> row;
    std::string b;
    std::cin >> b;
    GF2LinearSystem system(m);
    for (int i = 0; i < n; ++i) {
        std::vector<int> ones;
        for (int j = 0; j < m; ++j)
            if (a[i][j] == '1')
                ones.push_back(j);
        system.add_equation(ones, b[i] == '1');
    }
    auto r = system.solve(true);
    if (!r.solvable) {
        std::cout << -1 << '\n';
        return 0;
    }
    std::cout << r.free_variables << '\n';
    for (int x : r.solution)
        std::cout << x;
    std::cout << '\n';
    for (auto &v : r.nullspace) {
        for (int x : v)
            std::cout << x;
        std::cout << '\n';
    }
}
