#pragma once

// a^n % modを計算
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
