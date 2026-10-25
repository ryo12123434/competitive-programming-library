#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product"

#include <atcoder/modint>
#include <iostream>
#include "../../library/math/matrix.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m, k;
    std::cin >> n >> m >> k;
    using Mint = atcoder::modint998244353;
    Matrix<Mint> a(n, m), b(m, k);
    for (int y = 0; y < n; ++y) for (int x = 0; x < m; ++x) {
        int value;
        std::cin >> value;
        a(y, x) = value;
    }
    for (int y = 0; y < m; ++y) for (int x = 0; x < k; ++x) {
        int value;
        std::cin >> value;
        b(y, x) = value;
    }
    const auto c = a * b;
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < k; ++x) std::cout << (x == 0 ? "" : " ") << c(y, x).val();
        std::cout << '\n';
    }
}
