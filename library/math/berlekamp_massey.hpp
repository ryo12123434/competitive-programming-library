#pragma once
#include "miller_rabin.hpp"
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief 有限列を説明する最短の線形漸化式をBerlekamp–Massey法で求める。
 * @tparam T 体の元。prime modintを推奨。mod()があれば素数性を検査し、合成数法は例外。
 * @param sequence 観測した先頭項。空列・全零列の返り値は空列。
 * @return c[j]がa[t-1-j]に掛かる「遅れ順」の係数。linear_recurrenceにそのまま渡せる。
 * @note 観測より後の項を一意に保証するものではない。次数Kの真の列には通常2K項以上を用意する。
 * @pre 項数とその2倍はintに収まる。modint以外の除算・可逆性は呼び出し側が保証する。
 * @par Complexity N項にO(N^2)体演算、O(N)空間。逆元・素数判定のコストは別途必要。
 * @code
 * #include "library/math/berlekamp_massey.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using M=atcoder::modint998244353;assert((berlekamp_massey(std::vector<M>{0,1,1,2,3,5})==std::vector<M>{1,1}));}
 * @endcode
 */
template <class T> std::vector<T> berlekamp_massey(const std::vector<T> &sequence) {
    if constexpr (requires { T::mod(); })
        if (!is_prime_u64(T::mod()))
            throw std::invalid_argument("BM: coefficient field required");
    std::vector<T> c{T(1)}, previous{T(1)};
    int length = 0, shift = 1;
    T last = T(1);
    for (int n = 0; n < static_cast<int>(sequence.size()); ++n) {
        T discrepancy = sequence[n];
        for (int i = 1; i <= length; ++i)
            discrepancy += c[i] * sequence[n - i];
        if (discrepancy == T(0)) {
            ++shift;
            continue;
        }
        auto old = c;
        T ratio = discrepancy / last;
        if (c.size() < previous.size() + shift)
            c.resize(previous.size() + shift, T(0));
        for (std::size_t i = 0; i < previous.size(); ++i)
            c[i + shift] -= ratio * previous[i];
        if (2 * length <= n) {
            length = n + 1 - length;
            previous = std::move(old);
            last = discrepancy;
            shift = 1;
        } else
            ++shift;
    }
    c.resize(length + 1);
    c.erase(c.begin());
    for (auto &x : c)
        x = -x;
    return c;
}
