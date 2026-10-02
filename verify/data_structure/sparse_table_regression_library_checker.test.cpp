#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 全非空区間の min / max / gcd と、非可換な冪等演算を素朴な走査と比較する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "../../library/data_structure/sparse_table.hpp"

int minimum(int a, int b) { return std::min(a, b); }
int maximum(int a, int b) { return std::max(a, b); }
int gcd_op(int a, int b) { return std::gcd(a, b); }
int rightmost(int, int b) { return b; }

void check(const std::vector<int>& a) {
    SparseTable<int, minimum> min_table(a);
    SparseTable<int, maximum> max_table(a);
    SparseTable<int, gcd_op> gcd_table(a);
    SparseTable<int, rightmost> right_table(a);
    int n = static_cast<int>(a.size());
    assert(min_table.size() == n && max_table.size() == n);
    assert(gcd_table.size() == n && right_table.size() == n);
    for (int l = 0; l < n; ++l) {
        int lo = a[l], hi = a[l], g = 0;
        for (int r = l + 1; r <= n; ++r) {
            lo = std::min(lo, a[r - 1]);
            hi = std::max(hi, a[r - 1]);
            g = std::gcd(g, a[r - 1]);
            assert(min_table.prod(l, r) == lo);
            assert(max_table.prod(l, r) == hi);
            assert(gcd_table.prod(l, r) == g);
            assert(right_table.prod(l, r) == a[r - 1]);
        }
    }
}

int main() {
    check({});
    check({0});
    check({5, 5, 5});
    check({12, 18, 6, 24, 0, 30, 9, 3, 15});
    std::mt19937 rng(1707);
    for (int trial = 0; trial < 200; ++trial) {
        std::vector<int> a(rng() % 71);
        for (auto& x : a) x = static_cast<int>(rng() % 201) - 100;
        check(a);
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
