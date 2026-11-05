#pragma once
#include "miller_rabin.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>

/**
 * @brief 素数法pでx*x=aとなる最小の非負根を返すTonelli–Shanks法。解なしはnullopt。
 * @param a 被開平数。内部でa%pに正規化する。
 * @param p unsigned 64bitの素数。2も許す。非素数はinvalid_argument。
 * @note 合成数法は扱わない。0の根は0、p=2の根はa%2。
 * @par Complexity 非剰余の探索候補数をZとして O(Z log p + log^2 p) 剰余乗算、O(1)空間。
 * 最小非剰余を2から探すためZの小ささを最悪保証しない。素数判定にも7底 O(log p) を要する。
 * @code
 * #include "library/math/mod_sqrt.hpp"
 * #include <cassert>
 * int main(){assert(mod_sqrt_u64(10,13)==6);assert(!mod_sqrt_u64(2,13));assert(mod_sqrt_u64(0,2)==0);}
 * @endcode
 */
inline std::optional<std::uint64_t> mod_sqrt_u64(std::uint64_t a, std::uint64_t p) {
    using namespace cpl::number_theory_detail;
    if (!is_prime_u64(p))
        throw std::invalid_argument("mod_sqrt_u64: modulus must be prime");
    a %= p;
    if (a == 0 || p == 2)
        return a;
    if (pow_mod(a, (p - 1) / 2, p) != 1)
        return std::nullopt;
    std::uint64_t q = p - 1;
    int s = 0;
    while (!(q & 1)) {
        q >>= 1;
        ++s;
    }
    if (s == 1) {
        auto x = pow_mod(a, (q + 1) / 2, p);
        return std::min(x, p - x);
    }
    std::uint64_t z = 2;
    while (pow_mod(z, (p - 1) / 2, p) != p - 1)
        ++z;
    auto c = pow_mod(z, q, p), x = pow_mod(a, (q + 1) / 2, p), t = pow_mod(a, q, p);
    int m = s;
    while (t != 1) {
        int i = 0;
        auto power = t;
        while (power != 1) {
            power = mul_mod(power, power, p);
            ++i;
        }
        auto b = c;
        for (int j = 0; j < m - i - 1; ++j)
            b = mul_mod(b, b, p);
        x = mul_mod(x, b, p);
        c = mul_mod(b, b, p);
        t = mul_mod(t, c, p);
        m = i;
    }
    return std::min(x, p - x);
}
