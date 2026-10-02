#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"

#include <algorithm>
#include <iostream>
#include <vector>
#include "../../library/algorithm/mo.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<int> a(n);
    for (auto& x : a) std::cin >> x;
    auto values = a;
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    for (auto& x : a) x = static_cast<int>(std::lower_bound(values.begin(), values.end(), x) - values.begin());

    Mo mo(n);
    for (int i = 0; i < q; ++i) {
        int l, r;
        std::cin >> l >> r;
        mo.add_query(l, r);
    }
    std::vector<int> counts(values.size()), answers(q);
    int distinct = 0;
    mo.run(
        [&](int i) { if (counts[a[i]]++ == 0) ++distinct; },
        [&](int i) { if (--counts[a[i]] == 0) --distinct; },
        [&](int id) { answers[id] = distinct; }
    );
    for (int answer : answers) std::cout << answer << '\n';
}
