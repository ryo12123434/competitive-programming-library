#pragma once

#include "detail/integer_traits.hpp"

/**
 * @brief 整数の絶対値の10進桁数を返す。0は1桁、符号は数えない。
 * @tparam T boolを除く標準整数型とGNU signed/unsigned 128bit。最小負数も扱う。
 * @note 文字列は受け取らないので、既存テンプレートのdigit_sum(string)と共存できる。
 * @par Complexity 桁数Dに対し O(D) 時間・O(1) 追加空間。
 * @code
 * #include "library/math/integer_digits.hpp"
 * #include <cassert>
 * #include <climits>
 * int main(){assert(digit_count(0)==1&&digit_count(LLONG_MIN)==19&&digit_sum(-120)==3);}
 * @endcode
 */
template <cpl::integer_detail::Integer T> constexpr int digit_count(T value) {
    auto x = cpl::integer_detail::magnitude(value);
    int result = 0;
    do {
        ++result;
        x /= 10;
    } while (x != 0);
    return result;
}

/// @brief 整数の絶対値の10進桁和。符号を無視し、0は0。対応型・計算量はdigit_countと同じ。
template <cpl::integer_detail::Integer T> constexpr int digit_sum(T value) {
    auto x = cpl::integer_detail::magnitude(value);
    int result = 0;
    do {
        result += static_cast<int>(x % 10);
        x /= 10;
    } while (x != 0);
    return result;
}
