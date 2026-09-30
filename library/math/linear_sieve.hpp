#pragma once

#include <vector>
#include <utility>

/**
 * @brief 線形篩で最小素因数と素数一覧を前計算する。
 *
 * @par Complexity
 * 構築 O(N)、メモリ O(N)。
 */
struct LinearSieve {
    std::vector<int> lpf;
    std::vector<int> primes;

    /**
     * @brief 1..n の最小素因数と素数一覧を前計算する。
     * @param n 前計算する最大値
     * @pre n >= 0
     * @par Complexity
     * 時間 O(N)、メモリ O(N)。
     */
    explicit LinearSieve(int n) : lpf(n + 1, 0) {
        for (int i = 2; i <= n; ++i) {
            if (lpf[i] == 0) {
                lpf[i] = i;
                primes.push_back(i);
            }

            for (int p : primes) {
                if (p > lpf[i] || 1LL * i * p > n) break;
                lpf[i * p] = p;
            }
        }
    }

    /**
     * @brief x を素因数分解する。
     * @param x 素因数分解する整数
     * @return (素因数, 指数) の列
     * @pre 1 <= x < lpf.size()
     * @par Complexity
     * O(log x)
     */
    std::vector<std::pair<int, int>> factorize(int x) const {
        std::vector<std::pair<int, int>> res;

        while (x > 1) {
            int p = lpf[x];
            int cnt = 0;

            while (x % p == 0) {
                x /= p;
                ++cnt;
            }

            res.emplace_back(p, cnt);
        }

        return res;
    }
};
