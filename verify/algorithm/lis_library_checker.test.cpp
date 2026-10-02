#define PROBLEM "https://judge.yosupo.jp/problem/longest_increasing_subsequence"

#include <iostream>
#include "../../library/algorithm/lis.hpp"

int main() {
    int n;
    std::cin >> n;
    std::vector<long long> a(n);
    for (auto& x : a) std::cin >> x;
    auto indices = LIS<true>(a);
    std::cout << indices.size() << '\n';
    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
        std::cout << (i ? " " : "") << indices[i];
    std::cout << '\n';
}
