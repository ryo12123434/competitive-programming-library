#pragma once
#include "miller_rabin.hpp"
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * @brief 有理形式的冪級数P(x)/Q(x)のx^nの係数をBostan–Mori法で求める。
 * @tparam Polynomial FormalPowerSeries<Mint>等、value_type、添字、push_back、畳み込み積operator*を持つ係数列。
 * @param p 分子（昇べき順）。空列は0。分母より次数が大きくてもよい。
 * @param q 非空の分母。定数項0はinvalid_argument。係数は体、mod()があれば素数法を検査する。
 * @param n unsigned 64bit全域の係数番号。
 * @note FPSを使う場合は利用側でformal_power_series.hppをincludeする。積は既存FPS/ACLを再利用する。
 * その場合はACL convolutionの法・長さの条件を満たすこと。ヘッダ自体はACLに依存しない。
 * @par Complexity K=max(P.size(),Q.size())、積M(K)にO(M(K) log(n+1))時間・O(K)空間。
 * ACL NTTならO(K log K log(n+1))。係数逆元と素数判定のコストは別途必要。
 * @code
 * #include "library/math/bostan_mori.hpp"
 * #include "library/math/formal_power_series.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using P=FormalPowerSeries<atcoder::modint998244353>;assert(bostan_mori(P{0,1},P{1,-1,-1},10)==55);}
 * @endcode
 */
template <class Polynomial>
typename Polynomial::value_type bostan_mori(Polynomial p, Polynomial q, std::uint64_t n) {
    using T = typename Polynomial::value_type;
    if (q.empty() || q[0] == T(0))
        throw std::invalid_argument("Bostan-Mori: invertible constant term required");
    if constexpr (requires { T::mod(); })
        if (!is_prime_u64(T::mod()))
            throw std::invalid_argument("Bostan-Mori: coefficient field required");
    while (n) {
        if (p.empty())
            return T(0);
        auto negative = q;
        for (std::size_t i = 1; i < negative.size(); i += 2)
            negative[i] = -negative[i];
        auto numerator = p * negative, denominator = q * negative;
        Polynomial next_p, next_q;
        for (std::size_t i = n & 1; i < numerator.size(); i += 2)
            next_p.push_back(numerator[i]);
        for (std::size_t i = 0; i < denominator.size(); i += 2)
            next_q.push_back(denominator[i]);
        p = std::move(next_p);
        q = std::move(next_q);
        n >>= 1;
    }
    return p.empty() ? T(0) : p[0] / q[0];
}
