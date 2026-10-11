#pragma once

#include <algorithm>
#include <vector>

/**
 * @brief N の正の約数を昇順で列挙する。
 * @param N 正整数
 * @return N の正の約数を昇順に並べた列
 * @pre N >= 1
 * @note 重複は含まない。N == 1 では {1}。
 * @par Complexity
 * 約数の個数を K として O(sqrt(N) + K log K) 時間、O(K) 出力空間。
 *
 * @code
 * #include "library/math/divisors.hpp"
 * #include <cassert>
 * int main() {
 *     assert((calc_divisors(6) == std::vector<long long>{1, 2, 3, 6}));
 * }
 * @endcode
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
