#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_5_A"
#include "../../library/algorithm/knapsack.hpp"
#include <iostream>
int main() {
    int n;
    std::cin >> n;
    std::vector<int> a(n);
    for (int &x : a)
        std::cin >> x;
    int q;
    std::cin >> q;
    std::vector<int> queries(q);
    int limit = 0;
    for (int &x : queries) {
        std::cin >> x;
        limit = std::max(limit, x);
    }
    SubsetSum sums(a, limit);
    for (int x : queries)
        std::cout << (sums.reachable(x) ? "yes" : "no") << '\n';
}
