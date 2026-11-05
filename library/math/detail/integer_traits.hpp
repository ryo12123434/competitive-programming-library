#pragma once

#include <cstdint>
#include <type_traits>

/**
 * @brief 整数ユーティリティ内部の型判定。標準整数とGNU 128bitを扱い、bool・cv修飾型は除く。
 * @note 通常は個々の公開ヘッダをincludeする。Boostや浮動小数点型はこの対象に含めない。
 * @par Complexity 型判定はコンパイル時、magnitude は O(1) 時間・追加空間。
 * @code
 * #include "library/math/detail/integer_traits.hpp"
 * static_assert(cpl::integer_detail::Integer<__int128_t>);
 * static_assert(!cpl::integer_detail::Integer<bool>);
 * int main() { return cpl::integer_detail::magnitude(-3) != 3u; }
 * @endcode
 */
namespace cpl::integer_detail {
template <class T>
concept Integer =
    (std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>) &&
    !std::is_same_v<T, bool> && std::is_same_v<T, std::remove_cv_t<T>>;

template <class T> struct UnsignedType {
    using type = std::make_unsigned_t<T>;
};
template <> struct UnsignedType<__int128_t> {
    using type = __uint128_t;
};
template <> struct UnsignedType<__uint128_t> {
    using type = __uint128_t;
};
template <Integer T> using Unsigned = typename UnsignedType<T>::type;
template <Integer T>
inline constexpr bool signed_integer = std::is_signed_v<T> || std::is_same_v<T, __int128_t>;

template <Integer T> constexpr Unsigned<T> magnitude(T value) {
    using U = Unsigned<T>;
    U converted = static_cast<U>(value);
    if constexpr (signed_integer<T>) {
        if (value < 0)
            return U(0) - converted;
    }
    return converted;
}
} // namespace cpl::integer_detail
