#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// A + B を入出力用に使い、LIS の復元と転倒数を二次時間の参照実装と比較する。
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include "../../library/algorithm/lis.hpp"
#include "../../library/algorithm/inversion_number.hpp"

template <bool Strict>
void check_lis(const std::vector<long long>& a) {
    auto indices = LIS<Strict>(a);
    std::vector<int> dp(a.size(), 1);
    int best = 0;
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        for (int j = 0; j < i; ++j)
            if (Strict ? a[j] < a[i] : a[j] <= a[i]) dp[i] = std::max(dp[i], dp[j] + 1);
        best = std::max(best, dp[i]);
    }
    assert(static_cast<int>(indices.size()) == best);
    for (int i = 0; i < best; ++i) {
        assert(0 <= indices[i] && indices[i] < static_cast<int>(a.size()));
        if (i) {
            assert(indices[i - 1] < indices[i]);
            assert(Strict ? a[indices[i - 1]] < a[indices[i]] : a[indices[i - 1]] <= a[indices[i]]);
        }
    }
}

int main() {
    std::mt19937 rng(1702);
    for (int trial = 0; trial < 1000; ++trial) {
        std::vector<long long> a(rng() % 30);
        for (auto& x : a) x = static_cast<int>(rng() % 11) - 5;
        check_lis<true>(a);
        check_lis<false>(a);
        long long expected = 0;
        for (int i = 0; i < static_cast<int>(a.size()); ++i)
            for (int j = i + 1; j < static_cast<int>(a.size()); ++j) expected += a[i] > a[j];
        assert(count_inversions(a) == expected);
    }
    std::vector<long long> reverse(100000);
    std::iota(reverse.rbegin(), reverse.rend(), 0);
    assert(count_inversions(reverse) == 100000LL * 99999 / 2);
    assert((LIS<true>(std::vector<int>{2, 2, 2}).size() == 1));
    assert((LIS<false>(std::vector<int>{2, 2, 2}).size() == 3));
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
