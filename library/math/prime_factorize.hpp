#pragma once

#include <utility>
#include <vector>

/**
 * @brief N を試し割りで素因数分解する。
 * @param N 素因数分解する正整数
 * @return (素因数, 指数) の列
 * @pre N >= 1
 * @par Complexity
 * O(sqrt(N))
 */
inline std::vector<std::pair<long long, long long>> prime_factorize(long long N) {
    std::vector<std::pair<long long, long long>> res;

    for (long long p = 2; p <= N / p; ++p) {
        if (N % p != 0) continue;

        long long cnt = 0;
        while (N % p == 0) {
            N /= p;
            ++cnt;
        }

        res.push_back({p, cnt});
    }

    if (N > 1) {
        res.push_back({N, 1});
    }

    return res;
}
