#pragma once
#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

/**
 * @brief a[S]をsum_{T subset S} a[T]へ上書きする部分集合Zeta変換。
 * @tparam T 可換な加算を持つ型。逆変換には加法群、畳み込みには分配律を持つ積も必要。
 * @param a 長さ2^Bの列。空列は何もしない。各演算の中間値も型の範囲に収める。
 * @par Complexity O(B*2^B) 時間、O(1) 追加空間。以下の4変換で共通。
 * @code
 * #include "library/math/subset_transform.hpp"
 * #include <cassert>
 * int main(){std::vector<int> a{1,2,3,4},b=a;subset_zeta(a);assert(a[3]==10);subset_mobius(a);assert(a==b);}
 * @endcode
 */
template <class T> void subset_zeta(std::vector<T> &a) {
    assert(a.empty() || std::has_single_bit(a.size()));
    for (std::size_t bit = 1; bit < a.size(); bit <<= 1)
        for (std::size_t s = 0; s < a.size(); ++s)
            if (s & bit)
                a[s] += a[s ^ bit];
}
/// @brief 部分集合Zeta変換の逆。仕様・計算量はsubset_zetaと同じ。
template <class T> void subset_mobius(std::vector<T> &a) {
    assert(a.empty() || std::has_single_bit(a.size()));
    for (std::size_t bit = 1; bit < a.size(); bit <<= 1)
        for (std::size_t s = 0; s < a.size(); ++s)
            if (s & bit)
                a[s] -= a[s ^ bit];
}
/// @brief a[S]をsum_{T superset S} a[T]へ上書きする。仕様・計算量はsubset_zetaと同じ。
template <class T> void superset_zeta(std::vector<T> &a) {
    assert(a.empty() || std::has_single_bit(a.size()));
    for (std::size_t bit = 1; bit < a.size(); bit <<= 1)
        for (std::size_t s = 0; s < a.size(); ++s)
            if (!(s & bit))
                a[s] += a[s | bit];
}
/// @brief 上位集合Zeta変換の逆。仕様・計算量はsubset_zetaと同じ。
template <class T> void superset_mobius(std::vector<T> &a) {
    assert(a.empty() || std::has_single_bit(a.size()));
    for (std::size_t bit = 1; bit < a.size(); bit <<= 1)
        for (std::size_t s = 0; s < a.size(); ++s)
            if (!(s & bit))
                a[s] -= a[s | bit];
}
/// @brief c[S]=sum_{i|j=S} a[i]*b[j]。同じ2冪長（ともに空も可）。O(B*2^B)時間・O(2^B)空間。
template <class T> std::vector<T> or_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    subset_zeta(a);
    subset_zeta(b);
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] *= b[i];
    subset_mobius(a);
    return a;
}
/// @brief c[S]=sum_{i&j=S} a[i]*b[j]。仕様・計算量はor_convolutionと同じ。
template <class T> std::vector<T> and_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    superset_zeta(a);
    superset_zeta(b);
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] *= b[i];
    superset_mobius(a);
    return a;
}
