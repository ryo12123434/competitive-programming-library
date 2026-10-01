#define PROBLEM "http://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=DSL_1_B"

// Problem: AOJ DSL_1_B - Weighted Union Find Trees
// Target: library/data_structure/weighted_union_find.hpp

#include <iostream>

#include "../../library/data_structure/weighted_union_find.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    WeightedUnionFind<long long> uf(n);

    while (q--) {
        int type, x, y;
        std::cin >> type >> x >> y;

        if (type == 0) {
            long long z;
            std::cin >> z;
            uf.unite(x, y, z);
        } else if (uf.issame(x, y)) {
            std::cout << uf.diff(x, y) << '\n';
        } else {
            std::cout << "?\n";
        }
    }
}
