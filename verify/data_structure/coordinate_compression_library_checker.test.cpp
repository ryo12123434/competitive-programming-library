#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"

// Problem: Library Checker - Static Range Frequency
// Target: library/data_structure/coordinate_compression.hpp

#include <algorithm>
#include <iostream>
#include <vector>

#include "../../library/data_structure/coordinate_compression.hpp"

struct Query {
    int l;
    int r;
    int x;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    std::vector<int> a(n);
    for (int& x : a) std::cin >> x;

    std::vector<Query> queries(q);
    std::vector<int> values = a;
    values.reserve(n + q);

    for (auto& query : queries) {
        std::cin >> query.l >> query.r >> query.x;
        values.push_back(query.x);
    }

    const std::vector<int> compressed = compress(values);

    int kinds = 0;
    for (int x : compressed) kinds = std::max(kinds, x + 1);

    std::vector<std::vector<int>> positions(kinds);
    for (int i = 0; i < n; ++i) {
        positions[compressed[i]].push_back(i);
    }

    for (int i = 0; i < q; ++i) {
        const auto& query = queries[i];
        const auto& pos = positions[compressed[n + i]];
        const auto first = std::lower_bound(pos.begin(), pos.end(), query.l);
        const auto last = std::lower_bound(pos.begin(), pos.end(), query.r);
        std::cout << last - first << '\n';
    }
}
