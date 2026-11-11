#pragma once
#include <cstdint>
#include <initializer_list>

namespace cpl::number_theory_detail {
inline std::uint64_t mul_mod(std::uint64_t a, std::uint64_t b, std::uint64_t mod) {
    return static_cast<__uint128_t>(a) * b % mod;
}
inline std::uint64_t pow_mod(std::uint64_t a, std::uint64_t n, std::uint64_t mod) {
    std::uint64_t result = 1;
    for (; n; n >>= 1, a = mul_mod(a, a, mod))
        if (n & 1)
            result = mul_mod(result, a, mod);
    return result;
}
} // namespace cpl::number_theory_detail

/**
 * @brief unsigned 64bit全域の決定的Miller–Rabin素数判定。0,1はfalse。
 * @param n 判定する整数。[0,2^64-1]。
 * @return nが素数ならtrue。
 * @note 底 {2,325,9375,28178,450775,9780504,1795265022} は n<2^64 で十分な既知の集合。
 * 剰余積はunsigned 128bitで計算する。GNU/Clangの128bit対応が必要。
 * @par Complexity 7底それぞれ O(log n) 回の128bit剰余乗算、追加空間 O(1)。固定幅演算をO(1)とする。
 * @code
 * #include "library/math/miller_rabin.hpp"
 * #include <cassert>
 * int main(){assert(is_prime_u64(18446744073709551557ULL));assert(!is_prime_u64(561));}
 * @endcode
 */
inline bool is_prime_u64(std::uint64_t n) {
    if (n < 2)
        return false;
    for (unsigned p : {2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u})
        if (n % p == 0)
            return n == p;
    std::uint64_t d = n - 1;
    int s = 0;
    while (!(d & 1)) {
        d >>= 1;
        ++s;
    }
    for (std::uint64_t a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0)
            continue;
        auto x = cpl::number_theory_detail::pow_mod(a % n, d, n);
        if (x == 1 || x == n - 1)
            continue;
        bool pass = false;
        for (int r = 1; r < s; ++r) {
            x = cpl::number_theory_detail::mul_mod(x, x, n);
            if (x == n - 1) {
                pass = true;
                break;
            }
        }
        if (!pass)
            return false;
    }
    return true;
}
