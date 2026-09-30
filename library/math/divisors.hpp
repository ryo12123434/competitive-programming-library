#pragma once

#include <algorithm>
#include <vector>

// N の正の約数を昇順で列挙する。
// 計算量: O(sqrt(N))
// 前提: N >= 1
inline std::vector<long long> calc_divisors(long long N) {
    std::vector<long long> res;

    for (long long i = 1; i <= N / i; ++i) {
        if (N % i != 0) continue;

        res.push_back(i);
        if (i != N / i) res.push_back(N / i);
    }

    std::sort(res.begin(), res.end());
    return res;
}
