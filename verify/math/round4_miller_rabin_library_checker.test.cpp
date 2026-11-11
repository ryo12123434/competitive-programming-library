#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
#include "../../library/math/miller_rabin.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int t;
    std::cin >> t;
    while (t--) {
        std::uint64_t n;
        std::cin >> n;
        std::cout << (is_prime_u64(n) ? "Yes" : "No") << "\n";
    }
}
