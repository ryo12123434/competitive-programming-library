#pragma once

#include "detail/integer_traits.hpp"
#include <cassert>
#include <limits>

/**
 * @brief 非負整数のfloor(sqrt(value))を、浮動小数点なしで厳密に返す。
 * @tparam T boolを除く標準整数型とGNU signed/unsigned 128bit。返り値もT。
 * @param value 非負整数。uint64_t・__uint128_tの最大値を含む全域に対応。
 * @pre 符号付き型ではvalue>=0。負値はassertの対象で、事前条件違反。
 * @note mid*midを作らず value/mid で比較する。0除算・積のオーバーフローは発生しない。
 * @par Complexity B=型のビット幅に対し O(B) 時間・O(1) 追加空間（64bitで高々32段、128bitで高々64段）。
 * @code
 * #include "library/math/integer_sqrt.hpp"
 * #include <cassert>
 * #include <cstdint>
 * int main(){assert(isqrt(15)==3);assert(isqrt(~std::uint64_t(0))==4294967295ULL);}
 * @endcode
 */
template <cpl::integer_detail::Integer T> constexpr T isqrt(T value) {
    using U = cpl::integer_detail::Unsigned<T>;
    if constexpr (cpl::integer_detail::signed_integer<T>)
        assert(value >= 0);
    U x = static_cast<U>(value);
    if (x < 2)
        return static_cast<T>(x);
    constexpr int bits = std::numeric_limits<U>::digits;
    U left = 1, right = U(1) << ((bits + 1) / 2);
    while (left + 1 < right) {
        U mid = left + (right - left) / 2;
        if (mid <= x / mid)
            left = mid;
        else
            right = mid;
    }
    return static_cast<T>(left);
}
