#pragma once

#include <vector>

// 0..N が素数かどうかをエラトステネスの篩で求める。
// 計算量: O(N log log N)
// メモリ: O(N)
// 前提: N >= 0
inline std::vector<bool> eratosthenes(int n) {
    std::vector<bool> is_prime(n + 1, true);

    if (n >= 0) is_prime[0] = false;
    if (n >= 1) is_prime[1] = false;

    for (int p = 2; p <= n / p; ++p) {
        if (!is_prime[p]) continue;

        for (int i = p * p; i <= n; i += p) {
            is_prime[i] = false;
        }
    }

    return is_prime;
}
