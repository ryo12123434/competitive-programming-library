#define PROBLEM "https://judge.yosupo.jp/problem/unionfind"

// Problem: Library Checker - Unionfind
// Target: library/data_structure/union_find.hpp

#include <iostream>

#include "../../library/data_structure/union_find.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    UnionFind uf(n);

    while (q--) {
        int type, u, v;
        std::cin >> type >> u >> v;

        if (type == 0) {
            uf.unite(u, v);
        } else {
            std::cout << (uf.issame(u, v) ? 1 : 0) << '\n';
        }
    }
}
