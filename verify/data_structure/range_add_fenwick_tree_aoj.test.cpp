#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_G"
#include "../../library/data_structure/range_add_fenwick_tree.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    RangeAddFenwickTree<long long> bit(n);
    while (q--) {
        int t, l, r;
        std::cin >> t >> l >> r;
        if (t == 0) {
            long long x;
            std::cin >> x;
            bit.add(l - 1, r, x);
        } else
            std::cout << bit.sum(l - 1, r) << '\n';
    }
}
