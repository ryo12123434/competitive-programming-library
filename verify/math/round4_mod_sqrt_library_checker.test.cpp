#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_mod"
#include "../../library/math/mod_sqrt.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int t;
    std::cin >> t;
    while (t--) {
        std::uint64_t a, p;
        std::cin >> a >> p;
        auto x = mod_sqrt_u64(a, p);
        if (x)
            std::cout << *x;
        else
            std::cout << -1;
        std::cout << "\n";
    }
}
