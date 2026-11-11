#pragma once

#include "eratosthenes.hpp"
#include "integer_sqrt.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

/**
 * @brief 閉区間[L,R]の素数を昇順列挙する区間篩。
 * @param L 下端。負数、0、1は無視する。
 * @param R 上端。L>RまたはR<2なら空列。
 * @param max_base floor(sqrt(R))の上限。基礎篩のメモリ予算（既定1000万）。
 * @param max_width 2以上に切り詰めた区間長の上限（既定1000万）。
 * @return 区間内の素数。予算超過またはsqrt(R)>INT_MAXはlength_error。
 * @note 篩を再利用する。巨大なRにはsqrt(R)の前処理が必要で、狭い区間でも省略できない。
 * 開始倍数と次の倍数は128bitで計算する。上限の増加には相応のメモリが必要。
 * @par Complexity
 * S=floor(sqrt(max(R,0))), W=対象区間長として O(S log log(S+2)+W log log(S+2)+S) 時間、
 * O(S+W+出力件数) 空間。空区間はO(1)。
 * @code
 * #include "library/math/segmented_sieve.hpp"
 * #include <cassert>
 * int main(){assert((segmented_sieve(-10,7)==std::vector<std::int64_t>{2,3,5,7}));}
 * @endcode
 */
inline std::vector<std::int64_t> segmented_sieve(std::int64_t L, std::int64_t R,
                                                 std::size_t max_base = 10'000'000,
                                                 std::size_t max_width = 10'000'000) {
    if (L > R || R < 2)
        return {};
    L = std::max<std::int64_t>(L, 2);
    auto s = isqrt(static_cast<std::uint64_t>(R));
    auto width = static_cast<std::uint64_t>(R - L) + 1;
    if (s > max_base || s > static_cast<std::uint64_t>(std::numeric_limits<int>::max()) ||
        width > max_width || width > std::vector<bool>().max_size())
        throw std::length_error("segmented_sieve: sieve budget exceeded");
    auto base = eratosthenes(static_cast<int>(s));
    std::vector<bool> prime(static_cast<std::size_t>(width), true);
    for (std::uint64_t p = 2; p <= s; ++p)
        if (base[p]) {
            __int128_t start = std::max<__int128_t>(static_cast<__int128_t>(p) * p,
                                                    (static_cast<__int128_t>(L) + p - 1) / p * p);
            for (auto j = start; j <= R; j += p)
                prime[static_cast<std::size_t>(j - L)] = false;
        }
    std::vector<std::int64_t> result;
    for (std::size_t i = 0; i < width; ++i)
        if (prime[i])
            result.push_back(L + static_cast<std::int64_t>(i));
    return result;
}
