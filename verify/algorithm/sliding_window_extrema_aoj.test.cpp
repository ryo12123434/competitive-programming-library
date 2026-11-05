#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_3_D"
#include "../../library/algorithm/sliding_window_extrema.hpp"
#include <iostream>
int main() {
    int n, k;
    std::cin >> n >> k;
    std::vector<int> a(n);
    for (int &x : a)
        std::cin >> x;
    auto r = sliding_window_min(a, k);
    for (std::size_t i = 0; i < r.values.size(); ++i)
        std::cout << (i ? " " : "") << r.values[i];
    std::cout << '\n';
}
