#define PROBLEM "https://judge.yosupo.jp/problem/tree_diameter"

#include <iostream>
#include <vector>
#include "../../library/graph/tree_diameter.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    using Diameter = TreeDiameter<long long>;
    std::vector<std::vector<Diameter::Edge>> tree(n);
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        long long cost;
        std::cin >> a >> b >> cost;
        tree[a].push_back({b, cost});
        tree[b].push_back({a, cost});
    }
    const Diameter diameter(tree);
    const auto& path = diameter.get_path();
    std::cout << diameter.length() << ' ' << path.size() << '\n';
    for (std::size_t i = 0; i < path.size(); ++i) std::cout << (i == 0 ? "" : " ") << path[i];
    std::cout << '\n';
}
