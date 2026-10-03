#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// A + B を入出力用に使い、辺数・区間操作・長方形加算を素朴な参照実装と比較する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include "../../library/data_structure/union_find.hpp"
#include "../../library/data_structure/interval_set.hpp"
#include "../../library/data_structure/imos_2d.hpp"

int main() {
    Imos2D<long long> empty_grid(0, 0);
    empty_grid.add(-1, 1, -1, 1, 5);
    empty_grid.build();
    assert(empty_grid.data == (std::vector<std::vector<long long>>{{0}}));
    std::mt19937 rng(1704);
    UnionFind uf(20);
    std::vector<int> label(20);
    std::iota(label.begin(), label.end(), 0);
    std::vector<std::pair<int, int>> edges;
    for (int step = 0; step < 300; ++step) {
        int u = rng() % 20, v = rng() % 20;
        bool merged = label[u] != label[v];
        assert(uf.unite(u, v) == merged);
        int old = label[v], next = label[u];
        for (auto& x : label) if (x == old) x = next;
        edges.emplace_back(u, v);
        for (int x = 0; x < 20; ++x) {
            int count = 0;
            for (const auto& e : edges) if (label[e.first] == label[x]) ++count;
            assert(uf.getedges(x) == count);
            assert(uf.getsize(x) == std::count(label.begin(), label.end(), label[x]));
        }
    }
    UnionFind loops(2);
    assert(!loops.unite(0, 0));
    assert(!loops.unite(1, 1));
    assert(loops.unite(0, 1) && loops.getedges(1) == 3);
    assert(!loops.unite(0, 1) && loops.getedges(0) == 4);

    IntervalSet<int> intervals;
    std::vector<bool> covered(40, false);
    assert(!intervals.contains(0) && intervals.mex() == 0 && !intervals.intersects(-1, 1));
    for (int step = 0; step < 1000; ++step) {
        int l = static_cast<int>(rng() % 41) - 20;
        int r = static_cast<int>(rng() % 41) - 20;
        bool insert = rng() % 2;
        if (insert) intervals.insert(l, r); else intervals.erase(l, r);
        for (int x = l; x < r; ++x) covered[x + 20] = insert;
        assert(intervals.covered_length == std::count(covered.begin(), covered.end(), true));
        int previous_end = -21;
        for (const auto& p : intervals.st) {
            assert(p.first < p.second && previous_end < p.first);
            previous_end = p.second;
        }
        for (int x = -20; x <= 20; ++x) {
            assert(intervals.contains(x) == (x < 20 && covered[x + 20]));
            int m = x;
            while (m < 20 && covered[m + 20]) ++m;
            assert(intervals.mex(x) == m);
        }
        for (int trial = 0; trial < 10; ++trial) {
            int a = static_cast<int>(rng() % 41) - 20;
            int b = static_cast<int>(rng() % 41) - 20;
            bool overlap = false;
            for (int x = a; x < b; ++x) overlap = overlap || covered[x + 20];
            assert(intervals.intersects(a, b) == overlap);
        }
    }
    IntervalSet<int> wide;
    wide.insert(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    assert(wide.covered_length == 4294967295LL);
    wide.erase(-1, 1);
    assert(wide.covered_length == 4294967293LL && wide.mex(-1) == -1);
    IntervalSet<long long> extreme;
    auto low = std::numeric_limits<long long>::lowest(), high = std::numeric_limits<long long>::max();
    extreme.insert(low, low + 2); extreme.insert(high - 2, high);
    assert(extreme.covered_length == 4 && extreme.mex(low) == low + 2);
    assert(extreme.contains(high - 1) && !extreme.contains(high));
    IntervalSet<double> fractional;
    fractional.insert(0.25, 0.75); fractional.erase(0.5, 0.75);
    assert(fractional.covered_length == 0.25);

    Imos2D<int> zero(0, 0);
    zero.add(-1, 2, -1, 2); zero.build();
    for (int trial = 0; trial < 100; ++trial) {
        Imos2D<long long> imos(7, 8);
        std::vector<std::vector<long long>> expected(7, std::vector<long long>(8));
        for (int step = 0; step < 100; ++step) {
            int u = static_cast<int>(rng() % 15) - 3, d = static_cast<int>(rng() % 15) - 3;
            int l = static_cast<int>(rng() % 15) - 3, r = static_cast<int>(rng() % 15) - 3;
            long long value = static_cast<int>(rng() % 11) - 5;
            imos.add(u, d, l, r, value);
            for (int y = 0; y < 7; ++y) for (int x = 0; x < 8; ++x)
                if (u <= y && y < d && l <= x && x < r) expected[y][x] += value;
        }
        imos.build();
        for (int y = 0; y < 7; ++y) for (int x = 0; x < 8; ++x) assert(imos.get(y, x) == expected[y][x]);
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
