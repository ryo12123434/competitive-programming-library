#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 遷移を実際に辿り、終端または周期を検出する参照実装と比較する。
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include "../../library/algorithm/doubling.hpp"

int reference(const std::vector<int>& next, int v, unsigned long long steps) {
    if (v == -1) return -1;
    std::vector<int> position(next.size(), -1), path;
    while (v != -1 && position[v] == -1) {
        position[v] = static_cast<int>(path.size());
        path.push_back(v);
        v = next[v];
    }
    if (steps < path.size()) return path[steps];
    if (v == -1) return -1;
    unsigned long long start = position[v], length = path.size() - start;
    return path[start + (steps - start) % length];
}

void check(const std::vector<int>& next, std::mt19937_64& rng) {
    Doubling doubling(next);
    assert(doubling.jump(-1, 0) == -1);
    assert(doubling.jump(-1, std::numeric_limits<unsigned long long>::max()) == -1);
    for (int v = 0; v < static_cast<int>(next.size()); ++v) {
        for (unsigned long long steps : {0ULL, 1ULL, 2ULL, 63ULL, 64ULL, 1000ULL,
                                        1ULL << 63, std::numeric_limits<unsigned long long>::max()}) {
            assert(doubling.jump(v, steps) == reference(next, v, steps));
        }
        for (int trial = 0; trial < 20; ++trial) {
            auto steps = rng();
            assert(doubling.jump(v, steps) == reference(next, v, steps));
        }
    }
    Doubling one_level(next, 1);
    for (int v = 0; v < static_cast<int>(next.size()); ++v) {
        assert(one_level.jump(v, 0) == v);
        assert(one_level.jump(v, 1) == next[v]);
    }
}

int main() {
    std::mt19937_64 rng(1706);
    check({}, rng);
    check({-1}, rng);
    check({0}, rng);
    check({1, 2, 3, -1}, rng);
    check({1, 2, 3, 1}, rng);
    for (int trial = 0; trial < 200; ++trial) {
        int n = rng() % 31;
        std::vector<int> next(n);
        for (auto& v : next) v = static_cast<int>(rng() % (n + 1)) - 1;
        check(next, rng);
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
