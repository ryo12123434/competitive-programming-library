#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"

#include <atcoder/segtree>
#include <iostream>
#include <vector>
#include "../../library/graph/hld.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<long long> values(n);
    for (auto& value : values) std::cin >> value;
    std::vector<std::vector<int>> tree(n);
    for (int v = 1; v < n; ++v) {
        int parent;
        std::cin >> parent;
        tree[parent].push_back(v); tree[v].push_back(parent);
    }
    const HLD hld(tree);
    std::vector<long long> ordered(n);
    for (int v = 0; v < n; ++v) ordered[hld.tin(v)] = values[v];
    atcoder::segtree<long long, op, e> seg(ordered);
    while (q--) {
        int type, v;
        std::cin >> type >> v;
        if (type == 0) {
            long long x;
            std::cin >> x;
            seg.set(hld.tin(v), seg.get(hld.tin(v)) + x);
        } else {
            const auto [l, r] = hld.subtree_range(v);
            std::cout << seg.prod(l, r) << '\n';
        }
    }
}
