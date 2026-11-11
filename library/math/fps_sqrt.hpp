#pragma once

#include "mod_sqrt.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>

/**
 * @brief fの平方根をx^nで打ち切って求める。存在しない場合はnullopt。
 * @tparam Polynomial FormalPowerSeries<Mint>相当。value_type、pre、積、和、スカラー除算、ADL invが必要。
 * @param f 昇べき順の係数。不足項は0とみなす。
 * @param n 返す項数。負数はinvalid_argument、0は空多項式。
 * @pre 係数は素数法のACL static_modint。NTTの長さ・法の制約を満たすこと。
 * @note 先頭非零次数が奇数なら解なし。先頭根はmod_sqrt_u64の小さい根を選び、未確定の高次項は0。
 * 法2は奇数次係数の消滅を調べ、偶数次係数から直接復元する。
 * @note 既存FPSを再利用する。依存ACLの非inline定義を持ち込まないようFPSヘッダは利用側でincludeする。
 * @par Complexity NTT可能な奇素数法ではO(n log(n+1))時間・O(n)作業空間と定数項の平方根計算。
 * 法2はO(n)時間・空間。
 * @code
 * #include "library/math/formal_power_series.hpp"
 * #include "library/math/fps_sqrt.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using P=FormalPowerSeries<atcoder::modint998244353>; auto g=fps_sqrt(P{1,2,1},3);
 * assert(g && ((*g)*(*g)).pre(3)==(P{1,2,1}));}
 * @endcode
 */
template <class Polynomial> std::optional<Polynomial> fps_sqrt(const Polynomial &f, int n) {
    using T = typename Polynomial::value_type;
    if (n < 0)
        throw std::invalid_argument("fps_sqrt: negative length");
    if (!is_prime_u64(T::mod()))
        throw std::invalid_argument("fps_sqrt: modulus must be prime");
    if (!n)
        return Polynomial{};
    Polynomial result(n, T(0));
    int limit = static_cast<int>(std::min<std::size_t>(f.size(), n));
    if (T::mod() == 2) {
        for (int i = 0; i < limit; ++i) {
            if ((i & 1) && f[i] != T(0))
                return std::nullopt;
            if (!(i & 1))
                result[i / 2] = f[i];
        }
        return result;
    }
    int first = 0;
    while (first < limit && f[first] == T(0))
        ++first;
    if (first == limit)
        return result;
    if (first & 1)
        return std::nullopt;
    auto root = mod_sqrt_u64(f[first].val(), T::mod());
    if (!root)
        return std::nullopt;
    int need = n - first;
    Polynomial h{T(*root)};
    while (static_cast<int>(h.size()) < need) {
        int m = static_cast<int>(std::min<std::size_t>(need, h.size() * 2));
        Polynomial cut(f.begin() + first,
                       f.begin() + std::min<std::size_t>(f.size(), static_cast<std::size_t>(first) + m));
        cut.resize(m);
        h = (h + (cut * inv(h, m)).pre(m)) / T(2);
        h.resize(m);
    }
    std::copy(h.begin(), h.end(), result.begin() + first / 2);
    return result;
}
