#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 多重集合・重み付き UF・静的リスト・Fenwick Tree・座標圧縮を素朴な容器と比較する。
#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <vector>
#include "../../library/data_structure/coordinate_compression.hpp"
#include "../../library/data_structure/fenwick_tree.hpp"
#include "../../library/data_structure/ordered_set.hpp"
#include "../../library/data_structure/static_doubly_linked_list.hpp"
#include "../../library/data_structure/weighted_union_find.hpp"

int main() {
    std::mt19937 rng(1711);
    OrderedMultiset<long long> multiset;
    std::multiset<long long> reference;
    for (int step = 0; step < 2000; ++step) {
        long long x = static_cast<int>(rng() % 15) - 7;
        if (step % 13 == 0) x = LLONG_MIN;
        if (step % 17 == 0) x = LLONG_MAX;
        if (rng() % 3) { multiset.insert(x); reference.insert(x); }
        else {
            auto it = reference.find(x);
            assert(multiset.erase_one(x) == (it != reference.end()));
            if (it != reference.end()) reference.erase(it);
        }
        assert(multiset.size() == static_cast<int>(reference.size()));
        assert(multiset.count(x) == static_cast<int>(reference.count(x)));
        assert(multiset.order_of_key(x) == std::distance(reference.begin(), reference.lower_bound(x)));
        if (!reference.empty()) {
            int k = rng() % reference.size();
            auto it = reference.begin();
            std::advance(it, k);
            assert(multiset.find_by_order(k) == *it);
        }
    }
    multiset.clear();
    assert(multiset.empty());
    multiset.insert(LLONG_MAX);
    assert(multiset.count(LLONG_MAX) == 1);

    for (int trial = 0; trial < 100; ++trial) {
        constexpr int N = 20;
        WeightedUnionFind<long long> uf(N);
        std::vector<long long> potential(N);
        for (auto& x : potential) x = static_cast<int>(rng() % 1001) - 500;
        std::vector<int> label(N);
        std::iota(label.begin(), label.end(), 0);
        for (int step = 0; step < 60; ++step) {
            int u = rng() % N, v = rng() % N;
            bool merged = label[u] != label[v];
            assert(uf.unite(u, v, potential[v] - potential[u]) == merged);
            int old = label[v], next = label[u];
            for (auto& x : label) if (x == old) x = next;
            for (int x = 0; x < N; ++x) {
                assert(uf.get_size(x) == std::count(label.begin(), label.end(), label[x]));
                for (int y = 0; y < N; ++y) {
                    assert(uf.issame(x, y) == (label[x] == label[y]));
                    if (label[x] == label[y]) assert(uf.diff(x, y) == potential[y] - potential[x]);
                }
            }
        }
    }
    Static_Doubly_LinkedList list(20);
    std::vector<int> order(20);
    std::iota(order.begin(), order.end(), 0);
    for (int i = 1; i < 20; ++i) list.connect(i - 1, i);
    list.insert_after(0, 1);
    list.insert_before(1, 0);
    list.insert_after(1, 1);
    for (int step = 0; step < 1000; ++step) {
        int x = rng() % 20, z = rng() % 20;
        bool after = rng() & 1;
        if (x != z) {
            order.erase(std::find(order.begin(), order.end(), z));
            auto it = std::find(order.begin(), order.end(), x);
            order.insert(after ? it + 1 : it, z);
        }
        if (after) list.insert_after(x, z); else list.insert_before(x, z);
        assert(list.get_path(x) == order);
        for (int i = 0; i < 20; ++i) {
            assert(list.prv[order[i]] == (i == 0 ? -1 : order[i - 1]));
            assert(list.nxt[order[i]] == (i == 19 ? -1 : order[i + 1]));
        }
    }
    Static_Doubly_LinkedList components(5);
    components.connect(0, 1); components.connect(1, 2); components.connect(3, 4);
    components.insert_after(1, 3);
    assert(components.get_path(0) == (std::vector<int>{0, 1, 3, 2}));
    assert(components.get_path(4) == std::vector<int>{4});
    components.disconnect(1, 3);
    assert(components.get_path(0) == (std::vector<int>{0, 1}));
    assert(components.get_path(2) == (std::vector<int>{3, 2}));

    FenwickTree<long long> empty(0);
    assert(empty.sum(0, 0) == 0);
    for (int n = 1; n <= 40; ++n) {
        FenwickTree<long long> tree(n);
        std::vector<long long> a(n);
        for (int step = 0; step < 100; ++step) {
            int p = rng() % n;
            long long value = static_cast<int>(rng() % 201) - 100;
            tree.add(p, value); a[p] += value;
            for (int l = 0; l <= n; ++l) {
                long long total = 0;
                for (int r = l; r <= n; ++r) {
                    assert(tree.sum(l, r) == total);
                    if (r < n) total += a[r];
                }
            }
        }
    }
    assert(compress({}).empty());
    assert(compress({INT_MAX, INT_MIN, 0, INT_MAX}) == (std::vector<int>{2, 0, 1, 2}));
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
