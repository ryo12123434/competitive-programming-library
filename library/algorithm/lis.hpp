#pragma once

#include <algorithm>
#include <vector>

/**
 * @brief 最長増加部分列を構成する 0-indexed の添字列を返す。
 * @tparam Strict 狭義単調増加なら true、広義単調増加なら false。
 * @pre v.size() が int に収まり、Type の operator< が狭義弱順序を満たすこと。
 * @par 計算量 O(N log N) 時間、O(N) 空間。
 */
template <bool Strict, class Type>
std::vector<int> LIS(const std::vector<Type>& v) {
    std::vector<Type> dp;
    std::vector<int> positions;
    for (const auto& elem : v) {
        auto it = Strict ? std::lower_bound(dp.begin(), dp.end(), elem)
                         : std::upper_bound(dp.begin(), dp.end(), elem);
        positions.push_back(static_cast<int>(it - dp.begin()));
        if (it == dp.end()) dp.push_back(elem);
        else *it = elem;
    }
    std::vector<int> subseq(dp.size());
    int si = static_cast<int>(subseq.size()) - 1;
    for (int pi = static_cast<int>(positions.size()) - 1; pi >= 0 && si >= 0; --pi) {
        if (positions[pi] == si) subseq[si--] = pi;
    }
    return subseq;
}
