#pragma once

#include <algorithm>
#include <vector>

/**
 * @brief 最長増加部分列を構成する 0-indexed の添字列を返す。
 * @tparam Strict 狭義単調増加なら true、広義単調増加なら false。
 * @pre v.size() が int に収まり、Type の operator< が狭義弱順序を満たすこと。
 * @par Complexity
 * O(N log N) 時間、O(N) 空間。
 *
 * @tparam Type コピー可能で < による比較ができる値型。
 * @param v 部分列を求める入力列（変更しない）。
 * @return 元の列の添字を昇順に並べた最長部分列。解が複数あればその一つ。空列は空配列。
 *
 * @code
 * #include "library/algorithm/lis.hpp"
 * #include <cassert>
 * int main() {
 *     auto indices = LIS<true>(std::vector<int>{2, 1, 3});
 *     assert(indices.size() == 2);
 * }
 * @endcode
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
