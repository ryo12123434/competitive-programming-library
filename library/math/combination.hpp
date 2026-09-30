#pragma once

#include <vector>

// 二項係数 nCk を階乗と逆階乗で前計算する。
// 構築: O(N)
// com(n, k): O(1)
// メモリ: O(N)
// 前提:
// - Mint が四則演算と inv() をサポートすること。
// - fact[N] が法の下で可逆であること。
//   例: 素数 MOD に対して N < MOD。
template<class Mint>
struct Combination {
    std::vector<Mint> fact;
    std::vector<Mint> inv_fact;

    explicit Combination(int n) : fact(n + 1), inv_fact(n + 1) {
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) {
            fact[i] = fact[i - 1] * i;
        }

        inv_fact[n] = fact[n].inv();
        for (int i = n; i >= 1; --i) {
            inv_fact[i - 1] = inv_fact[i] * i;
        }
    }

    Mint com(int n, int k) const {
        if (n < 0 || k < 0 || k > n) return 0;
        return fact[n] * inv_fact[k] * inv_fact[n - k];
    }

    Mint permutation(int n, int k) const {
        if (n < 0 || k < 0 || k > n) return 0;
        return fact[n] * inv_fact[n - k];
    }
};
