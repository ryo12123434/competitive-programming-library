#pragma once

/**
 * @brief a^n mod mod を繰り返し二乗法で求める。
 * @param a 底
 * @param n 指数
 * @param mod 法
 * @return a^n mod mod
 * @pre n >= 0、mod > 0。途中の積が long long の範囲で扱えること。
 * @par Complexity
 * O(log n)
 */
long long modpow(long long a, long long n, long long mod) {
    if (n == 0) return 1;
    if (n == 1) return a % mod;

    long long ret = modpow(a, n / 2, mod) % mod;
    (ret *= ret) %= mod;
    if (n % 2 == 1) {
        (ret *= a) %= mod;
    }
    return ret;
}
