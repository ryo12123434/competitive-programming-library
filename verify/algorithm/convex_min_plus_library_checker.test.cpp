#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary"
#include "../../library/algorithm/monotone_minima.hpp"
#include "../../library/algorithm/divide_and_conquer_dp.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    std::cin >> n >> m;
    std::vector<long long> a(n), b(m);
    for (auto &x : a)
        std::cin >> x;
    for (auto &x : b)
        std::cin >> x;
    int length = n + m - 1;
    const __int128_t INF = __int128_t(1) << 100;
    auto cost = [&](int row, int col) -> __int128_t {
        int i = row - col;
        return 0 <= i && i < n ? __int128_t(a[i]) + b[col] : INF;
    };
    auto minima = monotone_minima(length, m, cost);
    std::vector<std::optional<__int128_t>> prev(length + 1);
    for (int i = 0; i < m; ++i)
        prev[i] = b[i];
    auto dp = divide_and_conquer_dp(prev, [&](int k, int j) -> __int128_t {
        int i = j - k - 1;
        return i < n ? a[i] : INF;
    });
    for (int i = 0; i < length; ++i) {
        auto x = cost(i, minima.argmin[i]);
        if (dp.value[i + 1] != x)
            return 1;
        std::cout << (i ? " " : "") << static_cast<long long>(x);
    }
    std::cout << '\n';
}
