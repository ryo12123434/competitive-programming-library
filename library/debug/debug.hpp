#pragma once

#include <cstddef>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cpl::debug {

/**
 * @brief 再帰表示の上限。print_options() の返す設定を変更して使う。
 * @note max_items は各コンテナの表示件数、max_depth は入れ子の深さ。0 も指定できる。
 * @code
 * #include "library/debug/debug.hpp"
 * #include <vector>
 * int main() {
 *     cpl::debug::PrintOptions options{3, 4};
 *     cpl::debug::print_options() = options;
 *     std::vector<int> a{1, 2, 3, 4};
 *     debug(a, a.size());
 * }
 * @endcode
 */
struct PrintOptions {
    std::size_t max_items = 20;
    std::size_t max_depth = 8;
};

/// @brief プログラム内で共有する表示設定を返す。O(1)。
inline PrintOptions& print_options() {
    static PrintOptions options;
    return options;
}

namespace detail {
inline std::string quote(std::string_view value, char delimiter) {
    std::string result(1, delimiter);
    for (unsigned char ch : value) {
        switch (ch) {
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '\0': result += "\\0"; break;
        default:
            if (ch < 32 || ch == 127) {
                constexpr char digits[] = "0123456789abcdef";
                result += "\\x";
                result += digits[ch >> 4];
                result += digits[ch & 15];
            } else {
                if (ch == static_cast<unsigned char>(delimiter)) result += '\\';
                result += static_cast<char>(ch);
            }
        }
    }
    return result + delimiter;
}

template <class T>
void write(std::ostream& out, const T& value, std::size_t depth, const PrintOptions& options) {
    using U = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<U, bool>) {
        out << (value ? "true" : "false");
    } else if constexpr (std::is_same_v<U, char>) {
        const char ch = value;
        out << quote(std::string_view(&ch, 1), '\'');
    } else if constexpr (std::is_same_v<U, signed char> || std::is_same_v<U, unsigned char>) {
        out << static_cast<int>(value);
    } else if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view>) {
        out << quote(value, '"');
    } else if constexpr (std::is_array_v<U> && std::is_same_v<std::remove_extent_t<U>, char>) {
        constexpr std::size_t n = std::extent_v<U>;
        out << quote(std::string_view(value, n - (n != 0 && value[n - 1] == '\0')), '"');
    } else if constexpr (std::is_same_v<U, char*> || std::is_same_v<U, const char*>) {
        if (value) out << quote(value, '"');
        else out << "nullptr";
    } else if constexpr (std::ranges::input_range<const T>) {
        if (depth >= options.max_depth) { out << "..."; return; }
        constexpr bool associative = requires { typename U::key_type; };
        out << (associative ? '{' : '[');
        auto it = std::ranges::begin(value);
        const auto end = std::ranges::end(value);
        std::size_t count = 0;
        for (; it != end && count < options.max_items; ++it, ++count) {
            if (count) out << ", ";
            write(out, *it, depth + 1, options);
        }
        if (it != end) out << (count ? ", ..." : "...");
        out << (associative ? '}' : ']');
    } else if constexpr (requires { typename std::tuple_size<U>::type; }) {
        if (depth >= options.max_depth) { out << "..."; return; }
        out << '(';
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            [[maybe_unused]] auto element = [&]<std::size_t Index>() {
                if constexpr (Index < std::tuple_size_v<U>) {
                    if (Index < options.max_items) {
                        if (Index) out << ", ";
                        write(out, std::get<Index>(value), depth + 1, options);
                    }
                }
            };
            (element.template operator()<I>(), ...);
        }(std::make_index_sequence<std::tuple_size_v<U>>{});
        if (std::tuple_size_v<U> > options.max_items) out << (options.max_items ? ", ..." : "...");
        out << ')';
    } else if constexpr (requires { out << value; }) {
        out << value;
    } else {
        out << "<unprintable>";
    }
}
} // namespace detail

/**
 * @brief 値を再帰的に文字列化する。LOCAL に関係なく実行する補助関数。
 * @tparam T 文字列、range、pair / tuple、または ostream に出力可能な型。
 * @param value 表示する値。STL の operator<< は追加・変更しない。
 * @param options 各コンテナの件数・深さの上限。
 * @return 文字列・文字は引用符付き。未対応型は <unprintable>。
 * @note unordered コンテナの順序は不定。文字列自体の長さは制限しない。
 * @par Complexity
 * 表示する要素数と文字数に比例する時間・出力空間。
 */
template <class T>
std::string repr(const T& value, const PrintOptions& options = print_options()) {
    std::ostringstream out;
    detail::write(out, value, 0, options);
    return out.str();
}

/**
 * @brief debug / CPL_DEBUG マクロの出力先。通常はマクロ経由で呼ぶ。
 * @param file 呼び出し元ファイル名。
 * @param line 呼び出し元行番号。
 * @param expressions 式の一覧。そのまま表示し、テンプレート内のカンマも保持する。
 * @param values 式の一覧と同じ順の値。
 * @note 各引数は一度だけ評価するが、評価順には依存しないこと。
 */
template <class... T>
void log(const char* file, int line, const char* expressions, const T&... values) {
    std::cerr << file << ':' << line << " | (" << expressions << ") = (";
    [[maybe_unused]] std::size_t index = 0;
    ((std::cerr << (index++ ? ", " : "") << repr(values)), ...);
    std::cerr << ")\n";
}
} // namespace cpl::debug

// LOCAL 無効時には、未定義の名前を含む引数も評価・コンパイルしない。
#ifdef LOCAL
#define CPL_DEBUG(...) do { ::cpl::debug::log(__FILE__, __LINE__, #__VA_ARGS__ __VA_OPT__(,) __VA_ARGS__); } while (false)
#else
#define CPL_DEBUG(...) ((void)0)
#endif

// 既存の debug マクロは上書きしない。短い名前が不要なら include 前に無効化する。
#if !defined(CPL_DEBUG_NO_SHORT_MACRO) && !defined(debug)
#define debug(...) CPL_DEBUG(__VA_ARGS__)
#endif
