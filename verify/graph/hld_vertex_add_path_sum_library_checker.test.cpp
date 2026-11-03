#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_path_sum"

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
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        std::cin >> u >> v;
        tree[u].push_back(v); tree[v].push_back(u);
    }
    const HLD hld(tree);
    std::vector<long long> ordered(n);
    for (int v = 0; v < n; ++v) ordered[hld.tin(v)] = values[v];
    atcoder::segtree<long long, op, e> seg(ordered);
    while (q--) {
        int type, u;
        std::cin >> type >> u;
        if (type == 0) {
            long long x;
            std::cin >> x;
            seg.set(hld.tin(u), seg.get(hld.tin(u)) + x);
        } else {
            int v;
            std::cin >> v;
            long long answer = 0;
            for (auto part : hld.path_segments(u, v)) answer += seg.prod(part.l, part.r);
            std::cout << answer << '\n';
        }
    }
}
