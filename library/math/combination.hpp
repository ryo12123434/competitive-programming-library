#pragma once

#include <vector>

/**
 * @brief 階乗と逆階乗を前計算して組合せ・順列を求める。
 *
 * @tparam Mint 四則演算と inv() をサポートする型
 * @pre fact[N] が法の下で可逆であること。例: 素数 MOD に対して N < MOD。
 * @par Complexity
 * 構築 O(N)、com / permutation O(1)、メモリ O(N)。
 */
template<class Mint>
struct Combination {
    std::vector<Mint> fact;
    std::vector<Mint> inv_fact;

    /**
     * @brief 0..n の階乗と逆階乗を前計算する。
     * @param n 前計算する最大値
     * @par Complexity
     * O(N)
     */
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

    /**
     * @brief 二項係数 nCk を返す。
     * @param n 要素数
     * @param k 選ぶ個数
     * @return nCk。範囲外なら 0
     * @par Complexity
     * O(1)
     */
    Mint com(int n, int k) const {
        if (n < 0 || k < 0 || k > n) return 0;
        return fact[n] * inv_fact[k] * inv_fact[n - k];
    }

    /**
     * @brief 順列数 nPk を返す。
     * @param n 要素数
     * @param k 並べる個数
     * @return nPk。範囲外なら 0
     * @par Complexity
     * O(1)
     */
    Mint permutation(int n, int k) const {
        if (n < 0 || k < 0 || k > n) return 0;
        return fact[n] * inv_fact[n - k];
    }
};
