#pragma once
#include "monotone_minima.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

/// @brief 次段DP値・最適遷移元・実際のcost呼び出し回数。到達不能は nullopt / -1。
template <class T> struct DivideAndConquerDPResult {
    std::vector<std::optional<T>> value;
    std::vector<int> argmin;
    std::size_t evaluations = 0;
};
/**
 * @brief next[j]=min_{0<=k<j}(previous[k]+cost(k,j)) の1段を分割統治で計算する。
 * @param previous 長さ N+1。nullopt は到達不能。next[0] は常に nullopt（空遷移を許さない）。
 * @param cost cost(k,j) は半開区間 [k,j) などの遷移コスト T を返す純粋な関数。
 * @return 長さ N+1 の値・左端の最適k・評価回数。到達不能な前段と k>=j は評価しない。
 * @pre previous は非空、N<=INT_MAX。加算・比較は T で正確に表現できること。
 * 有限値を持つ各 j の左端最適kが j とともに広義単調増加すること。一般のDPへの適用は不可。
 * @note 既存 monotone_minima を利用。評価回数には最適値の取り出し直しも含む。
 * @par Complexity コスト・型の操作が O(1) なら O(N log(N+1)) 時間、O(N) 追加空間。
 * @code
 * #include "library/algorithm/divide_and_conquer_dp.hpp"
 * #include <cassert>
 * int main(){std::vector<std::optional<long long>> p(5);p[0]=0;auto r=divide_and_conquer_dp(p,[](int k,int j){return 1LL*(j-k)*(j-k);});assert(r.value[4]==16);}
 * @endcode
 */
template <class T, class Cost>
DivideAndConquerDPResult<T> divide_and_conquer_dp(const std::vector<std::optional<T>> &previous, Cost cost) {
    assert(!previous.empty() &&
           previous.size() - 1 <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int n = static_cast<int>(previous.size() - 1);
    DivideAndConquerDPResult<T> result;
    result.value.resize(previous.size());
    result.argmin.assign(previous.size(), -1);
    auto evaluate = [&](int row, int k) -> std::optional<T> {
        int j = row + 1;
        if (k >= j || !previous[k])
            return std::nullopt;
        ++result.evaluations;
        return *previous[k] + cost(k, j);
    };
    auto minima = monotone_minima(n, n, evaluate, [](const std::optional<T> &a, const std::optional<T> &b) {
        return a && (!b || *a < *b);
    });
    for (int row = 0; row < n; ++row) {
        int j = row + 1;
        result.value[j] = evaluate(j - 1, minima.argmin[j - 1]);
        if (result.value[j])
            result.argmin[j] = minima.argmin[j - 1];
    }
    return result;
}
