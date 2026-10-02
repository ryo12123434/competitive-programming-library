#pragma once

#include <cassert>
#include <vector>

/**
 * @brief 0..n の素数判定表をエラトステネスの篩で構築する。
 * @param n 判定する最大値
 * @return is_prime[x] が x の素数判定を表す配列
 * @pre n >= 0
 * @par Complexity
 * 時間 O(N log log N)、メモリ O(N)。
 */
inline std::vector<bool> eratosthenes(int n) {
    assert(n >= 0);
    std::vector<bool> is_prime(static_cast<std::size_t>(n) + 1, true);

    if (n >= 0) is_prime[0] = false;
    if (n >= 1) is_prime[1] = false;

    for (int p = 2; p <= n / p; ++p) {
        if (!is_prime[p]) continue;

        for (long long i = 1LL * p * p; i <= n; i += p) {
            is_prime[i] = false;
        }
    }

    return is_prime;
}
