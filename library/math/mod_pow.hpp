#pragma once

#include <cassert>

/**
 * @brief a^n mod mod を繰り返し二乗法で求める。
 * @param a 底
 * @param n 指数
 * @param mod 法
 * @return a^n mod mod
 * @pre n >= 0、mod > 0。
 * @note 負の底も扱い、結果は [0, mod)。積は __int128 で計算してオーバーフローを避ける。
 * @par Complexity
 * O(log n)
 */
inline long long modpow(long long a, long long n, long long mod) {
    assert(n >= 0 && mod > 0);
    a %= mod;
    if (a < 0) a += mod;
    long long result = 1 % mod;
    while (n != 0) {
        if (n & 1) result = static_cast<long long>(static_cast<__int128>(result) * a % mod);
        a = static_cast<long long>(static_cast<__int128>(a) * a % mod);
        n >>= 1;
    }
    return result;
}
