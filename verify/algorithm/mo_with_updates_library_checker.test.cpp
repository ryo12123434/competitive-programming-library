#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_frequency"
#include "../../library/algorithm/mo_with_updates.hpp"
#include <iostream>
struct Query {
    int type, l, r;
    long long x;
};
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<long long> a(n), keys;
    for (auto &x : a)
        std::cin >> x;
    keys = a;
    std::vector<Query> queries(q);
    for (auto &v : queries) {
        std::cin >> v.type;
        if (v.type == 0)
            std::cin >> v.l >> v.x;
        else
            std::cin >> v.l >> v.r >> v.x;
        keys.push_back(v.x);
    }
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    auto code = [&](long long x) {
        return static_cast<int>(std::lower_bound(keys.begin(), keys.end(), x) - keys.begin());
    };
    std::vector<int> values(n);
    for (int i = 0; i < n; ++i)
        values[i] = code(a[i]);
    MoWithUpdates<int> mo(values);
    std::vector<int> query_value;
    for (auto v : queries) {
        if (v.type == 0)
            mo.add_update(v.l, code(v.x));
        else {
            mo.add_query(v.l, v.r);
            query_value.push_back(code(v.x));
        }
    }
    std::vector<int> counts(keys.size()), answers(query_value.size());
    mo.run([&](int, int x) { ++counts[x]; }, [&](int, int x) { --counts[x]; },
           [&](int id) { answers[id] = counts[query_value[id]]; });
    for (int x : answers)
        std::cout << x << '\n';
}
