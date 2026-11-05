#pragma once

#include <algorithm>
#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

/**
 * @brief 符号なし128bit整数を、先頭0のない10進文字列に変換する。0は"0"。
 * @param value [0,2^128-1] の全域に対応する値。
 * @return ASCIIの数字列。ストリームの書式設定に依存しない。
 * @par Complexity 桁数Dに対し O(D) 時間・返り値の空間。D<=39。
 * @code
 * #include "library/math/int128_io.hpp"
 * #include <cassert>
 * int main(){assert(to_string_uint128(__uint128_t(1)<<64)=="18446744073709551616");}
 * @endcode
 */
inline std::string to_string_uint128(__uint128_t value) {
    std::string result;
    do {
        result.push_back(static_cast<char>('0' + value % 10));
        value /= 10;
    } while (value != 0);
    std::reverse(result.begin(), result.end());
    return result;
}

/**
 * @brief 符号付き128bit整数を10進文字列に変換。[-2^127,2^127-1]全域に対応する。
 * @note 負の絶対値は符号なし演算で求め、最小値を符号付きで反転しない。
 * @par Complexity 桁数Dに対し O(D) 時間・返り値の空間。
 */
inline std::string to_string_int128(__int128_t value) {
    __uint128_t magnitude = static_cast<__uint128_t>(value);
    if (value >= 0)
        return to_string_uint128(magnitude);
    return "-" + to_string_uint128(__uint128_t(0) - magnitude);
}

namespace cpl::integer_io_detail {
inline std::optional<__uint128_t> parse_magnitude(std::string_view digits, __uint128_t limit) {
    if (digits.empty())
        return std::nullopt;
    __uint128_t value = 0;
    for (char c : digits) {
        if (c < '0' || c > '9')
            return std::nullopt;
        unsigned digit = static_cast<unsigned>(c - '0');
        if (value > (limit - digit) / 10)
            return std::nullopt;
        value = value * 10 + digit;
    }
    return value;
}
} // namespace cpl::integer_io_detail

/**
 * @brief 10進文字列を符号付き128bitへ厳密に変換。形式不正・範囲外ならnullopt。
 * @param text ASCIIの[+-]?[0-9]+。先頭0と符号付き0は許し、空白は前後・内部とも拒否する。
 * @return [-2^127,2^127-1]の値。0x表記・小数・区切り文字は受け付けない。
 * @par Complexity 文字数Lに対し O(L) 時間・O(1) 追加空間。
 */
inline std::optional<__int128_t> parse_int128(std::string_view text) {
    if (text.empty())
        return std::nullopt;
    bool negative = text.front() == '-';
    if (negative || text.front() == '+')
        text.remove_prefix(1);
    const __uint128_t boundary = __uint128_t(1) << 127;
    auto value = cpl::integer_io_detail::parse_magnitude(text, negative ? boundary : boundary - 1);
    if (!value)
        return std::nullopt;
    if (negative && *value != 0)
        return -static_cast<__int128_t>(*value - 1) - 1;
    return static_cast<__int128_t>(*value);
}

/**
 * @brief 10進文字列を符号なし128bitへ変換。形式不正・範囲外ならnullopt。
 * @param text ASCIIの[+]?[0-9]+。先頭0は許す。負号は-0も含め拒否、空白も拒否する。
 * @return [0,2^128-1]の値。
 * @par Complexity O(L) 時間・O(1) 追加空間。
 */
inline std::optional<__uint128_t> parse_uint128(std::string_view text) {
    if (!text.empty() && text.front() == '+')
        text.remove_prefix(1);
    return cpl::integer_io_detail::parse_magnitude(text, ~__uint128_t(0));
}

/**
 * @brief 空白区切りの1トークンを読み、符号付き128bitへ変換する。演算子は追加しない。
 * @param input 入力ストリーム。トークン抽出は標準stringの規則（skipws等）に従う。
 * @param value 成功時だけ代入する出力先。失敗時は変更せずfailbitを設定する。
 * @return input。例外マスクが設定されていれば標準ストリームの例外規則に従う。
 * @par Complexity トークン長Lに対し O(L) 時間・一時文字列の空間。
 * @code
 * #include "library/math/int128_io.hpp"
 * #include <cassert>
 * #include <sstream>
 * int main(){std::istringstream in("-123");__int128_t x=0;read_int128(in,x);std::ostringstream out;print_int128(out,x);assert(out.str()=="-123");}
 * @endcode
 */
inline std::istream &read_int128(std::istream &input, __int128_t &value) {
    std::string token;
    if (input >> token) {
        auto parsed = parse_int128(token);
        if (parsed)
            value = *parsed;
        else
            input.setstate(std::ios_base::failbit);
    }
    return input;
}

/// @brief 符号なし版。成功時だけvalueを代入。仕様・計算量はread_int128と同じ。
inline std::istream &read_uint128(std::istream &input, __uint128_t &value) {
    std::string token;
    if (input >> token) {
        auto parsed = parse_uint128(token);
        if (parsed)
            value = *parsed;
        else
            input.setstate(std::ios_base::failbit);
    }
    return input;
}

/**
 * @brief 常に10進で符号付き128bitを出力し、outputを返す。
 * @note stringとして挿入するためwidth/fill/左右寄せを適用する。hex/showposは無視し、フラグ自体は変更しない。
 * 改行は付けない。例: print_int128(std::cout,x) << '\n';
 * @par Complexity O(D) 時間・一時文字列の空間。
 */
inline std::ostream &print_int128(std::ostream &output, __int128_t value) {
    return output << to_string_int128(value);
}
/// @brief 符号なし版。仕様・計算量はprint_int128と同じ。
inline std::ostream &print_uint128(std::ostream &output, __uint128_t value) {
    return output << to_string_uint128(value);
}
