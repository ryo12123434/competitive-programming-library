#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_5_B"
#include "../../library/data_structure/fenwick_tree_2d.hpp"
#include <algorithm>
#include <iostream>
int main() {
    int n;
    std::cin >> n;
    FenwickTree2D<int> bit(1001, 1001);
    while (n--) {
        int l, u, r, d;
        std::cin >> l >> u >> r >> d;
        bit.add(u, l, 1);
        bit.add(d, l, -1);
        bit.add(u, r, -1);
        bit.add(d, r, 1);
    }
    int result = 0;
    for (int y = 0; y < 1000; ++y)
        for (int x = 0; x < 1000; ++x)
            result = std::max(result, bit.sum(y + 1, x + 1));
    std::cout << result << '\n';
}
