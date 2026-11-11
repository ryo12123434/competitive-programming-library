#pragma once

#include "integer_sqrt.hpp"
#include "miller_rabin.hpp"
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace cpl::discrete_log_detail {
inline std::uint64_t inverse(std::uint64_t a, std::uint64_t m) {
    std::uint64_t r = m, s = a;
    __int128_t x = 0, y = 1;
    while (s) {
        auto q = r / s, next = r % s;
        r = s;
        s = next;
        auto z = x - static_cast<__int128_t>(q) * y;
        x = y;
        y = z;
    }
    x %= static_cast<__int128_t>(m);
    if (x < 0)
        x += m;
    return static_cast<std::uint64_t>(x);
}
} // namespace cpl::discrete_log_detail

/**
 * @brief a^x=b (mod m) の最小の非負解。一般の正整数法に対応する拡張BSGS。
 * @param a 底。内部で法により正規化する。0^0=1 とする。
 * @param b 目標値。
 * @param m 正の法（uint64_t全域）。0はinvalid_argument。
 * @param max_baby_steps メモリ予算。gcd除去後のceil(sqrt(m))が超過するとlength_error。
 * @return 最小のx。解なしはnullopt。m=1では0。
 * @note 128bit剰余積を使用する。予算不足と解なしを区別する。
 * @par Complexity
 * gcd除去後の法をMとして、ハッシュ表の期待 O(sqrt(M)+log^2(m)) 時間、O(sqrt(M)) 空間。
 * unordered_mapの衝突が集中する最悪時は O(M+log^2(m)) 時間。固定幅整数演算をO(1)とする。
 * @code
 * #include "library/math/discrete_log.hpp"
 * #include <cassert>
 * int main(){assert(discrete_log_u64(4,8,14)==3);assert(!discrete_log_u64(2,3,8));}
 * @endcode
 */
inline std::optional<std::uint64_t> discrete_log_u64(std::uint64_t a, std::uint64_t b, std::uint64_t m,
                                                     std::size_t max_baby_steps = 2'000'000) {
    using cpl::number_theory_detail::mul_mod;
    using cpl::number_theory_detail::pow_mod;
    if (!m)
        throw std::invalid_argument("discrete_log_u64: zero modulus");
    if (m == 1)
        return 0;
    a %= m;
    b %= m;
    if (b == 1)
        return 0;
    if (!a)
        return b == 0 ? std::optional<std::uint64_t>(1) : std::nullopt;
    if (a == 1)
        return std::nullopt;
    if (a == m - 1)
        return b == a ? std::optional<std::uint64_t>(1) : std::nullopt;
    std::uint64_t offset = 0, acc = 1;
    for (auto g = std::gcd(a, m); g != 1; g = std::gcd(a, m)) {
        if (b == acc)
            return offset;
        if (b % g)
            return std::nullopt;
        b /= g;
        m /= g;
        ++offset;
        if (m == 1)
            return offset;
        acc = mul_mod(acc, a / g, m);
    }
    auto target = mul_mod(b, cpl::discrete_log_detail::inverse(acc, m), m);
    if (target == 1)
        return offset;
    auto step = isqrt(m);
    if (step * step != m)
        ++step; // floor(sqrt(m))の二乗はuint64_t内。
    if (step > max_baby_steps)
        throw std::length_error("discrete_log_u64: baby-step budget exceeded");
    std::unordered_map<std::uint64_t, std::uint64_t> baby;
    baby.reserve(static_cast<std::size_t>(step));
    std::uint64_t value = 1;
    for (std::uint64_t j = 0; j < step; ++j) {
        baby.try_emplace(value, j); // 同じ剰余には最小の指数を保持する。
        value = mul_mod(value, a, m);
    }
    auto factor = cpl::discrete_log_detail::inverse(pow_mod(a, step, m), m);
    for (std::uint64_t i = 0; i <= (m - 1) / step; ++i) {
        if (auto it = baby.find(target); it != baby.end()) {
            __uint128_t answer = static_cast<__uint128_t>(i) * step + it->second + offset;
            if (answer <= std::numeric_limits<std::uint64_t>::max())
                return static_cast<std::uint64_t>(answer);
        }
        target = mul_mod(target, factor, m);
    }
    return std::nullopt;
}
