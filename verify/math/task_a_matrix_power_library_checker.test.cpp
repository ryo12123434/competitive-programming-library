#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_matrix"

#include <atcoder/modint>
#include <iostream>
#include "../../library/math/matrix.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    unsigned long long exponent;
    std::cin >> n >> exponent;
    using Mint = atcoder::modint998244353;
    Matrix<Mint> a(n, n);
    for (int y = 0; y < n; ++y) for (int x = 0; x < n; ++x) {
        int value;
        std::cin >> value;
        a(y, x) = value;
    }
    const auto result = a.pow(exponent);
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) std::cout << (x == 0 ? "" : " ") << result(y, x).val();
        std::cout << '\n';
    }
}
