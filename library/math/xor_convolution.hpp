#pragma once
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

/**
 * @brief XOR用Walsh–Hadamard変換。inverse=trueなら長さで割って逆変換する。
 * @tparam T 加減算を持つ型。逆変換には長さ2^Bが可逆な係数環（奇数法modint等）が必要。
 * @param a 長さ2^B（空も可）。整数型は中間値の範囲と逆変換時の完全な割り切れを要求。
 * @throws std::invalid_argument 逆変換で偶数法modintを指定、または整数除算が割り切れない場合。
 * @note modint以外の係数環の可逆性は呼び出し側が保証する。エラー時は配列が変更される場合がある。
 * @par Complexity O(B*2^B) 時間、O(1) 追加空間。
 * @code
 * #include "library/math/xor_convolution.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using M=atcoder::modint998244353;std::vector<M>a{1,2},b{3,4};auto c=xor_convolution(a,b);assert(c[0]==11&&c[1]==10);}
 * @endcode
 */
template <class T> void walsh_hadamard_transform(std::vector<T> &a, bool inverse = false) {
    assert(a.empty() || std::has_single_bit(a.size()));
    if (a.empty())
        return;
    if constexpr (std::numeric_limits<T>::is_integer)
        if (inverse && __uint128_t(a.size()) > __uint128_t(std::numeric_limits<T>::max()))
            throw std::length_error("WHT: length does not fit the integer coefficient type");
    if constexpr (requires { T::mod(); })
        if (inverse && a.size() > 1 && T::mod() % 2 == 0)
            throw std::invalid_argument("WHT: 2 is not invertible");
    for (std::size_t half = 1; half < a.size(); half <<= 1)
        for (std::size_t begin = 0; begin < a.size(); begin += 2 * half)
            for (std::size_t j = 0; j < half; ++j) {
                T x = a[begin + j], y = a[begin + j + half];
                a[begin + j] = x + y;
                a[begin + j + half] = x - y;
            }
    if (inverse) {
        T size = T(a.size());
        for (auto &x : a) {
            if constexpr (std::numeric_limits<T>::is_integer)
                if (size == 0 || x % size != 0)
                    throw std::invalid_argument("WHT: non-exact integer division");
            x /= size;
        }
    }
}
/// @brief c[S]=sum_{i^j=S}a[i]*b[j]。同じ2冪長（空も可）。WHTと同じ型条件。O(B*2^B)時間・O(2^B)空間。
template <class T> std::vector<T> xor_convolution(std::vector<T> a, std::vector<T> b) {
    assert(a.size() == b.size());
    walsh_hadamard_transform(a);
    walsh_hadamard_transform(b);
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] *= b[i];
    walsh_hadamard_transform(a, true);
    return a;
}
