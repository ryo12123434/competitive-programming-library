#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"

#include <iostream>
#include <vector>
#include "../../library/graph/euler_tour.hpp"
#include "../../library/data_structure/fenwick_tree.hpp"

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
        tree[parent].push_back(v);
        tree[v].push_back(parent);
    }
    const EulerTour tour(tree);
    FenwickTree<long long> sum(n);
    for (int v = 0; v < n; ++v) sum.add(tour.tin(v), values[v]);
    while (q--) {
        int type, v;
        std::cin >> type >> v;
        if (type == 0) {
            long long value;
            std::cin >> value;
            sum.add(tour.tin(v), value);
        } else {
            const auto [l, r] = tour.subtree_range(v);
            std::cout << sum.sum(l, r) << '\n';
        }
    }
}
