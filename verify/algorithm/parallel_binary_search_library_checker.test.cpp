#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../../library/algorithm/parallel_binary_search.hpp"
#include "../../library/data_structure/fenwick_tree.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <tuple>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<int> a(n), order(n);
    for (int &x : a)
        std::cin >> x;
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int i, int j) { return a[i] < a[j]; });
    std::vector<std::tuple<int, int, int>> queries(q);
    for (auto &[l, r, k] : queries)
        std::cin >> l >> r >> k;
    FenwickTree<int> bit(n);
    auto answers = parallel_binary_search(
        n, q, [&] { bit = FenwickTree<int>(n); }, [&](int i) { bit.add(order[i], 1); },
        [&](int id) {
            auto [l, r, k] = queries[id];
            return bit.sum(l, r) > k;
        });
    for (int t : answers)
        std::cout << a[order[t - 1]] << '\n';
}
