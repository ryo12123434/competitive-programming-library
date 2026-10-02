#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 左右の操作順と区間の内容を deque で、種類数を素朴な集合で検証する。
#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <random>
#include <set>
#include <utility>
#include <vector>
#include "../../library/algorithm/mo.hpp"

void check(const std::vector<int>& a, const std::vector<std::pair<int, int>>& queries) {
    int n = static_cast<int>(a.size());
    Mo mo(n);
    for (int id = 0; id < static_cast<int>(queries.size()); ++id) {
        auto [l, r] = queries[id];
        assert(mo.add_query(l, r) == id);
    }
    std::vector<int> expected;
    for (auto [l, r] : queries) expected.push_back(static_cast<int>(std::set<int>(a.begin() + l, a.begin() + r).size()));

    // 同じ Mo を空の状態から再実行できることも確認する。
    for (int repeat = 0; repeat < 2; ++repeat) {
        std::deque<int> active;
        std::vector<int> seen(queries.size());
        mo.run(
            [&](int i) {
                assert(0 <= i && i < n);
                assert(active.empty() || i + 1 == active.front());
                active.push_front(i);
            },
            [&](int i) {
                assert(0 <= i && i < n);
                assert(active.empty() || active.back() + 1 == i);
                active.push_back(i);
            },
            [&](int i) {
                assert(!active.empty() && active.front() == i);
                active.pop_front();
            },
            [&](int i) {
                assert(!active.empty() && active.back() == i);
                active.pop_back();
            },
            [&](int id) {
                assert(0 <= id && id < static_cast<int>(queries.size()));
                assert(++seen[id] == 1);
                auto [l, r] = queries[id];
                assert(static_cast<int>(active.size()) == r - l);
                for (int i = l; i < r; ++i) assert(active[i - l] == i);
            }
        );
        for (int count : seen) assert(count == 1);
    }

    std::vector<int> counts(8), answers(queries.size(), -1), seen(queries.size());
    int distinct = 0;
    mo.run(
        [&](int i) { assert(0 <= i && i < n); if (counts[a[i]]++ == 0) ++distinct; },
        [&](int i) { assert(0 <= i && i < n && counts[a[i]] > 0); if (--counts[a[i]] == 0) --distinct; },
        [&](int id) { assert(++seen[id] == 1); answers[id] = distinct; }
    );
    assert(answers == expected);
    for (int count : seen) assert(count == 1);
}

int main() {
    check({}, {});
    check({}, {{0, 0}, {0, 0}});
    check({3}, {});
    check({3}, {{1, 1}, {0, 1}, {0, 0}, {0, 1}});
    check({1, 2, 1, 3, 2, 4}, {{5, 6}, {0, 1}, {0, 6}, {3, 3}, {1, 5}, {6, 6}, {0, 6}});
    std::mt19937 rng(1705);
    for (int trial = 0; trial < 300; ++trial) {
        int n = rng() % 41, q = rng() % 101;
        std::vector<int> a(n);
        for (auto& x : a) x = rng() % 8;
        std::vector<std::pair<int, int>> queries;
        for (int i = 0; i < q; ++i) {
            int l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r) std::swap(l, r);
            queries.emplace_back(l, r);
        }
        check(a, queries);
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
