#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_mod"
#include "../../library/math/discrete_log.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int t;
    std::cin >> t;
    while (t--) {
        std::uint64_t a, b, m;
        std::cin >> a >> b >> m;
        auto x = discrete_log_u64(a, b, m);
        if (x)
            std::cout << *x;
        else
            std::cout << -1;
        std::cout << '\n';
    }
}
