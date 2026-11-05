#define PROBLEM "https://judge.yosupo.jp/problem/cartesian_tree"
#include "../../library/algorithm/cartesian_tree.hpp"
#include <iostream>
int main() {
    int n;
    std::cin >> n;
    std::vector<int> a(n);
    for (int &x : a)
        std::cin >> x;
    auto r = cartesian_tree(a);
    for (int i = 0; i < n; ++i)
        std::cout << (r.parent[i] < 0 ? i : r.parent[i]) << ' ';
    std::cout << '\n';
}
