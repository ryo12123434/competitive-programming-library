#define PROBLEM "https://judge.yosupo.jp/problem/factorize"
#include "../../library/math/pollard_rho.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int t;
    std::cin >> t;
    while (t--) {
        std::uint64_t n;
        std::cin >> n;
        auto f = factorize_u64(n);
        std::cout << f.size();
        for (auto x : f)
            std::cout << " " << x;
        std::cout << "\n";
    }
}
