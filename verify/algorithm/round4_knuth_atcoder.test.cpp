#define PROBLEM "https://atcoder.jp/contests/dp/tasks/dp_n"
#include "../../library/algorithm/knuth_optimization.hpp"
#include <iostream>
int main() {
    int n;
    std::cin >> n;
    std::vector<long long> p(n + 1);
    for (int i = 0; i < n; ++i) {
        long long x;
        std::cin >> x;
        p[i + 1] = p[i] + x;
    }
    auto result = knuth_optimization<long long>(n, [&](int l, int r) { return p[r] - p[l]; });
    std::cout << result.dp[0][n] << "\n";
}
