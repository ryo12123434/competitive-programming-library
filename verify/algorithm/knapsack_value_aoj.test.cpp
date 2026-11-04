#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DPL_1_B"
#include "../../library/algorithm/knapsack.hpp"
#include <iostream>
int main() {
    int n, w;
    std::cin >> n >> w;
    std::vector<KnapsackItem> items(n);
    for (auto &item : items)
        std::cin >> item.value >> item.weight;
    std::cout << knapsack_by_value(items, w) << '\n';
}
