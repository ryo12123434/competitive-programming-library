#pragma once

#include "detail/integer_traits.hpp"
#include <optional>
#include <type_traits>

/**
 * @brief 64bit以下の同一整数型の積を、符号に合った128bit型へ拡張して厳密に計算する。
 * @return 符号付き入力なら__int128_t、符号なしなら__uint128_t。乗算前に拡張する。
 * @note bool、混在型、128bit入力は対象外。128bit同士の積にはchecked_mulを使う。
 * @par Complexity O(1) 時間・追加空間。
 * @code
 * #include "library/math/safe_arithmetic.hpp"
 * #include <cassert>
 * #include <climits>
 * int main(){auto x=wide_mul(LLONG_MAX,2LL);assert(x>LLONG_MAX&&!checked_mul(LLONG_MAX,2LL));}
 * @endcode
 */
template <cpl::integer_detail::Integer T>
    requires(sizeof(T) <= 8)
constexpr auto wide_mul(T a, T b) {
    using Wide = std::conditional_t<cpl::integer_detail::signed_integer<T>, __int128_t, __uint128_t>;
    return static_cast<Wide>(a) * static_cast<Wide>(b);
}

/**
 * @brief a*bとc*dを、オーバーフローなしで比較する。
 * @return 左が小さいなら-1、等しいなら0、大きいなら+1。
 * @note 対応型・計算量はwide_mulと同じ。4引数は同一型にする。
 */
template <cpl::integer_detail::Integer T>
    requires(sizeof(T) <= 8)
constexpr int compare_products(T a, T b, T c, T d) {
    auto left = wide_mul(a, b), right = wide_mul(c, d);
    return left < right ? -1 : left > right ? 1 : 0;
}

/**
 * @brief a+bをTの範囲で計算し、オーバーフローならnullopt。
 * @tparam T boolを除く標準整数型とGNU signed/unsigned 128bit。同一型の2引数を受け取る。
 * @note GCC/Clangのオーバーフロー検出組み込み関数を利用。検出時も符号付き演算のUBは起こさない。
 * @par Complexity O(1) 時間・追加空間。
 */
template <cpl::integer_detail::Integer T> constexpr std::optional<T> checked_add(T a, T b) {
    T result{};
    if (__builtin_add_overflow(a, b, &result))
        return std::nullopt;
    return result;
}

/// @brief a*bをTの範囲で計算し、オーバーフローならnullopt。仕様・型・計算量はchecked_addと同じ。
template <cpl::integer_detail::Integer T> constexpr std::optional<T> checked_mul(T a, T b) {
    T result{};
    if (__builtin_mul_overflow(a, b, &result))
        return std::nullopt;
    return result;
}
