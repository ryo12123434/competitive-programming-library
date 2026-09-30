#pragma once

#include <algorithm>
#include <vector>

/**
 * @brief N の正の約数を昇順で列挙する。
 * @param N 正整数
 * @return N の正の約数を昇順に並べた列
 * @pre N >= 1
 * @par Complexity
 * O(sqrt(N))
 */
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
