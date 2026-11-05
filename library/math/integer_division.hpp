#pragma once

#include "detail/integer_traits.hpp"
#include <limits>
#include <optional>

/**
 * @brief a/bを負の無限大へ丸めた商。不正な除算はnulloptで返す。
 * @tparam T boolを除く標準整数型またはGNU signed/unsigned 128bit。a,bは同じ型にする。
 * @param a 被除数。
 * @param b 除数。負数を含む全組み合わせに対応。
 * @return floor(a/b)。b=0、または符号付き最小値/-1ならnullopt。除算前に検査する。
 * @note 混在型を暗黙に共通型へ変換しない。必要ならfloor_div<long long>(a,b)のように型を指定する。
 * @par Complexity O(1) 時間・追加空間。
 * @code
 * #include "library/math/integer_division.hpp"
 * #include <cassert>
 * int main(){assert(floor_div(-7,3)==-3&&ceil_div(-7,3)==-2&&!floor_div(1,0));}
 * @endcode
 */
template <cpl::integer_detail::Integer T> constexpr std::optional<T> floor_div(T a, T b) {
    if (b == 0)
        return std::nullopt;
    if constexpr (cpl::integer_detail::signed_integer<T>) {
        if (a == std::numeric_limits<T>::lowest() && b == T(-1))
            return std::nullopt;
    }
    T quotient = a / b, remainder = a % b;
    if constexpr (cpl::integer_detail::signed_integer<T>) {
        if (remainder != 0 && (a < 0) != (b < 0))
            --quotient;
    }
    return quotient;
}

/**
 * @brief a/bを正の無限大へ丸めた商。対応型・エラー・計算量はfloor_divと同じ。
 * @return ceil(a/b)。b=0、符号付き最小値/-1ならnullopt。
 */
template <cpl::integer_detail::Integer T> constexpr std::optional<T> ceil_div(T a, T b) {
    if (b == 0)
        return std::nullopt;
    if constexpr (cpl::integer_detail::signed_integer<T>) {
        if (a == std::numeric_limits<T>::lowest() && b == T(-1))
            return std::nullopt;
    }
    T quotient = a / b, remainder = a % b;
    if (remainder != 0) {
        if constexpr (cpl::integer_detail::signed_integer<T>) {
            if ((a < 0) == (b < 0))
                ++quotient;
        } else
            ++quotient;
    }
    return quotient;
}
