#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_14_B"

#include <iostream>
#include "../../library/string/rolling_hash.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string text, pattern;
    std::cin >> text >> pattern;
    RollingHash t(text), p(pattern);
    int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
    for (int i = 0; i + m <= n; ++i)
        if (t.get(i, i + m) == p.get(0, m)) std::cout << i << '\n';
}
