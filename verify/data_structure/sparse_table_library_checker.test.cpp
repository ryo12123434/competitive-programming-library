#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"

#include <algorithm>
#include <iostream>
#include <vector>
#include "../../library/data_structure/sparse_table.hpp"

int minimum(int a, int b) { return std::min(a, b); }

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<int> a(n);
    for (auto& x : a) std::cin >> x;
    SparseTable<int, minimum> table(a);
    while (q--) {
        int l, r;
        std::cin >> l >> r;
        std::cout << table.prod(l, r) << '\n';
    }
}
