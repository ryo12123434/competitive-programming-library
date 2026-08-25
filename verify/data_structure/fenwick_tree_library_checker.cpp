// Problem: Library Checker - Point Add Range Sum
// URL: https://judge.yosupo.jp/problem/point_add_range_sum
// Target: library/data_structure/fenwick_tree.hpp

#include <iostream>

#include "../../library/data_structure/fenwick_tree.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    FenwickTree<long long> fw(n);
    for (int i = 0; i < n; ++i) {
        long long a;
        std::cin >> a;
        fw.add(i, a);
    }

    while (q--) {
        int type;
        std::cin >> type;
        if (type == 0) {
            int p;
            long long x;
            std::cin >> p >> x;
            fw.add(p, x);
        } else {
            int l, r;
            std::cin >> l >> r;
            std::cout << fw.sum(l, r) << '\n';
        }
    }
}
