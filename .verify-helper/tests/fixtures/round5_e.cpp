#include "library/data_structure/coordinate_compression.hpp"
#include "library/data_structure/union_find.hpp"
#include "library/algorithm/doubling.hpp"
#include "library/graph/bellman_ford.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <numeric>
#include <random>
#include <vector>
// std::countの導入で、利用者の変数と衝突していた回帰。
int count = 7;
int main() {
    assert(count == 7);
    std::mt19937 rng(5005);
    assert(compress({}).empty());
    assert((compress({INT_MIN, 0, INT_MAX, INT_MIN}) == std::vector<int>{0, 1, 2, 0}));
    for (int n = 0; n < 100; ++n) {
        std::vector<int> values(n);
        for (auto &v : values)
            v = static_cast<int>(rng() % 20) - 10;
        auto ranks = compress(values);
        for (int i = 0; i < n; ++i) {
            std::vector<int> smaller;
            for (int x : values)
                if (x < values[i])
                    smaller.push_back(x);
            std::sort(smaller.begin(), smaller.end());
            smaller.erase(std::unique(smaller.begin(), smaller.end()), smaller.end());
            assert(ranks[i] == static_cast<int>(smaller.size()));
        }
        if (!n)
            continue;
        UnionFind uf(n);
        std::vector<int> labels(n), edges(n);
        std::iota(labels.begin(), labels.end(), 0);
        for (int q = 0; q < 2000; ++q) {
            int a = rng() % n, b = rng() % n, x = labels[a], y = labels[b];
            bool merged = uf.unite(a, b);
            assert(merged == (x != y));
            if (x != y) {
                edges[x] += edges[y];
                for (auto &v : labels)
                    if (v == y)
                        v = x;
            }
            ++edges[x];
            int v = rng() % n;
            assert(uf.getsize(v) == std::count(labels.begin(), labels.end(), labels[v]));
            assert(uf.getedges(v) == edges[labels[v]]);
            assert(uf.issame(a, b));
        }
    }
    Doubling doubling({1, 2, -1});
    assert(doubling.jump(0, 2) == 2 && doubling.jump(0, 3) == -1);
    Doubling self({0});
    assert(self.jump(0, ~0ULL) == 0);
    std::vector<long long> distance;
    assert(!bellman_ford({{0, 1, LLONG_MIN}, {1, 2, LLONG_MAX}}, 3, 0, distance));
    assert(distance[1] == LLONG_MIN && distance[2] == -1);
    assert(bellman_ford({{0, 1, -1}, {1, 0, 0}}, 2, 0, distance));
}
