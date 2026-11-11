#pragma once
#include "subset_transform.hpp"
#include <algorithm>
#include <bit>

/**
 * @brief c[S]=sum_{T subset S} a[T]*b[S\T]をranked Zeta変換で計算する。
 * @tparam T 可換環（modint等）。通常整数は全中間値の範囲を保証する。
 * @param a,b 同じ2冪長2^Bの配列。ともに空なら空列。OR畳み込みとは重複を許さない点が異なる。
 * @note 部分集合変換は既存subset_zeta/mobiusを利用。大きいBではメモリに注意する。
 * @par Complexity O(B^2 2^B)時間、O(B 2^B)空間。主な作業領域は(2(B+1)+2)*2^B係数。
 * @code
 * #include "library/math/subset_convolution.hpp"
 * #include <cassert>
 * int main(){auto c=subset_convolution(std::vector<int>{1,2},std::vector<int>{3,4});assert((c==std::vector<int>{3,10}));}
 * @endcode
 */
template <class T> std::vector<T> subset_convolution(const std::vector<T> &a, const std::vector<T> &b) {
    assert(a.size() == b.size());
    if (a.empty())
        return {};
    assert(std::has_single_bit(a.size()));
    int bits = static_cast<int>(std::bit_width(a.size()) - 1);
    std::size_t n = a.size();
    std::vector<std::vector<T>> left(bits + 1, std::vector<T>(n, T(0))), right = left;
    for (std::size_t mask = 0; mask < n; ++mask) {
        int k = std::popcount(mask);
        left[k][mask] = a[mask];
        right[k][mask] = b[mask];
    }
    for (int k = 0; k <= bits; ++k) {
        subset_zeta(left[k]);
        subset_zeta(right[k]);
    }
    std::vector<T> result(n), work(n);
    for (int k = 0; k <= bits; ++k) {
        std::fill(work.begin(), work.end(), T(0));
        for (int i = 0; i <= k; ++i)
            for (std::size_t mask = 0; mask < n; ++mask)
                work[mask] += left[i][mask] * right[k - i][mask];
        subset_mobius(work);
        for (std::size_t mask = 0; mask < n; ++mask)
            if (std::popcount(mask) == k)
                result[mask] = work[mask];
    }
    return result;
}
