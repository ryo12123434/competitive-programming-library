#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"

#include <bit>
#include <iostream>
#include <vector>
#include "../../library/algorithm/doubling.hpp"
#include "../../library/graph/lca.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<std::vector<int>> tree(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        std::cin >> u >> v;
        tree[u].push_back(v);
        tree[v].push_back(u);
    }
    LCA lca(tree);
    std::vector<int> parent(n);
    for (int v = 0; v < n; ++v) parent[v] = lca.parent(v);
    Doubling doubling(parent, static_cast<int>(std::bit_width(static_cast<unsigned int>(n))));
    while (q--) {
        int s, t, k;
        std::cin >> s >> t >> k;
        int w = lca.query(s, t);
        int up = lca.depth(s) - lca.depth(w);
        int down = lca.depth(t) - lca.depth(w);
        if (k > up + down) std::cout << -1 << '\n';
        else if (k <= up) std::cout << doubling.jump(s, k) << '\n';
        else std::cout << doubling.jump(t, up + down - k) << '\n';
    }
}
