#pragma once
#include "miller_rabin.hpp"
#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cpl::number_theory_detail {
inline std::uint64_t rho_attempt(std::uint64_t n, std::mt19937_64 &rng) {
    const auto c = 1 + rng() % (n - 1);
    auto step = [&](std::uint64_t x) { return static_cast<std::uint64_t>((__uint128_t(x) * x + c) % n); };
    std::uint64_t y = 1 + rng() % (n - 1), x = 0, saved = y, g = 1;
    constexpr unsigned limit = 1u << 20;
    unsigned work = 0, length = 1;
    while (g == 1 && work < limit) {
        x = y;
        for (unsigned i = 0; i < length && work < limit; ++i, ++work)
            y = step(y);
        for (unsigned i = 0; i < length && g == 1 && work < limit; i += 128) {
            saved = y;
            std::uint64_t product = 1;
            for (unsigned j = 0; j < std::min(128u, length - i) && work < limit; ++j, ++work) {
                y = step(y);
                product = mul_mod(product, x > y ? x - y : y - x, n);
            }
            g = std::gcd(product, n);
        }
        if (length >= limit / 2)
            break;
        length *= 2;
    }
    if (g == n) {
        do {
            if (work++ >= limit)
                return 0;
            saved = step(saved);
            g = std::gcd(x > saved ? x - saved : saved - x, n);
        } while (g == 1);
    }
    return 1 < g && g < n ? g : 0;
}
} // namespace cpl::number_theory_detail

/**
 * @brief Miller–RabinとBrent版Pollard Rhoで素因数を重複込み・昇順で返す。1は空列。
 * @param n 1以上のunsigned 64bit整数。0はinvalid_argument。
 * @param seed 固定すれば同じ探索を再現する乱数seed。
 * @param attempts 合成数1個の分割に許す再試行回数。0はinvalid_argument。
 * @throws std::runtime_error 再試行を使い切った場合。seedやattemptsを変更して再実行できる。
 * @note 各試行は高々2^20回の軌道更新で打ち切る。失敗した結果を因数として返さない。
 * 既存の試し割りprime_factorizeや小さい値のLinearSieveとは別用途。
 * @par Complexity 確率的。最小素因数pの分割は乱数写像のモデルで期待 O(sqrt(p)) 剰余演算。
 * 最悪成功時間は保証しない。1分割の探索は O(attempts*2^20)、MRは O(log n) 剰余演算。
 * 固定幅演算をO(1)とし、因数・作業領域は O(log n)。
 * @code
 * #include "library/math/pollard_rho.hpp"
 * #include <cassert>
 * int main(){assert((factorize_u64(72)==std::vector<std::uint64_t>{2,2,2,3,3}));}
 * @endcode
 */
inline std::vector<std::uint64_t> factorize_u64(std::uint64_t n, std::uint64_t seed = 0x3141592653589793ULL,
                                                unsigned attempts = 64) {
    if (n == 0 || attempts == 0)
        throw std::invalid_argument("factorize_u64: n and attempts must be positive");
    std::vector<std::uint64_t> result, pending{n};
    std::mt19937_64 rng(seed);
    while (!pending.empty()) {
        auto value = pending.back();
        pending.pop_back();
        if (value == 1)
            continue;
        if (is_prime_u64(value)) {
            result.push_back(value);
            continue;
        }
        std::uint64_t divisor = 0;
        for (unsigned p : {2u, 3u, 5u})
            if (value % p == 0) {
                divisor = p;
                break;
            }
        for (unsigned trial = 0; !divisor && trial < attempts; ++trial)
            divisor = cpl::number_theory_detail::rho_attempt(value, rng);
        if (!divisor)
            throw std::runtime_error("factorize_u64: retry budget exhausted");
        pending.push_back(divisor);
        pending.push_back(value / divisor);
    }
    std::sort(result.begin(), result.end());
    return result;
}
/// @brief (素因数,指数)の昇順列。引数・例外はfactorize_u64と同じ。分解に加えて O(log n) 時間・空間。
inline std::vector<std::pair<std::uint64_t, int>>
factorize_u64_counts(std::uint64_t n, std::uint64_t seed = 0x3141592653589793ULL, unsigned attempts = 64) {
    std::vector<std::pair<std::uint64_t, int>> result;
    for (auto p : factorize_u64(n, seed, attempts)) {
        if (result.empty() || result.back().first != p)
            result.emplace_back(p, 1);
        else
            ++result.back().second;
    }
    return result;
}
