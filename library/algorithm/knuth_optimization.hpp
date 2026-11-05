#pragma once
#include <algorithm>
#include <cassert>
#include <vector>

/**
 * @brief 半開区間DPの表と最適分割位置。split[l][r]は右部分の開始点、長さ0/1では-1。
 * @note dp[l][r]=min_{l<k<r}(dp[l][k]+dp[k][r])+cost(l,r)、dp[l][l+1]=0。
 * @code
 * #include "library/algorithm/knuth_optimization.hpp"
 * #include <cassert>
 * int main(){auto r=knuth_optimization<long long>(3,[](int l,int r){return 1LL*(r-l);});assert(r.dp[0][3]==5&&r.split[0][3]==2);}
 * @endcode
 */
template <class T> struct KnuthResult {
    std::vector<std::vector<T>> dp;
    std::vector<std::vector<int>> split;
};
/**
 * @brief Knuth最適化による区間DP。等値なら最大の分割位置を選ぶ。
 * @tparam T 0、加算、大小比較を持つコスト型。全中間値が表現可能であること。
 * @param n 要素数（0も可）。コールバックcost(l,r)は0-indexed半開区間の共通追加費用。
 * @pre 最適位置がopt[l][r-1]<=opt[l][r]<=opt[l+1][r]を満たすこと。
 * 十分条件はa<=b<=c<=dでcost(b,c)<=cost(a,d)かつ
 * cost(a,c)+cost(b,d)<=cost(a,d)+cost(b,c)。任意の区間DPには使えない。
 * @par Complexity コスト評価O(1)ならO(N^2)時間・空間。候補範囲は単調性で償却O(N^2)。
 */
template <class T, class Cost> KnuthResult<T> knuth_optimization(int n, Cost cost) {
    assert(n >= 0);
    KnuthResult<T> result;
    result.dp.assign(static_cast<std::size_t>(n) + 1, std::vector<T>(static_cast<std::size_t>(n) + 1, T(0)));
    result.split.assign(result.dp.size(), std::vector<int>(result.dp.size(), -1));
    std::vector<std::vector<int>> opt(result.dp.size(), std::vector<int>(result.dp.size()));
    for (int i = 0; i < n; ++i)
        opt[i][i + 1] = i + 1;
    for (int length = 2; length <= n; ++length)
        for (int l = 0; l <= n - length; ++l) {
            int r = l + length, from = std::max(l + 1, opt[l][r - 1]), to = std::min(r - 1, opt[l + 1][r]);
            assert(from <= to);
            int best = from;
            T value = result.dp[l][from] + result.dp[from][r];
            for (int k = from + 1; k <= to; ++k) {
                T candidate = result.dp[l][k] + result.dp[k][r];
                if (!(value < candidate)) {
                    value = candidate;
                    best = k;
                }
            }
            result.dp[l][r] = value + cost(l, r);
            result.split[l][r] = opt[l][r] = best;
        }
    return result;
}
