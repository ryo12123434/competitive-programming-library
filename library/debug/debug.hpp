#pragma once

#include "../math/int128_io.hpp"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <ranges>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace cpl::debug {

/**
 * @brief 再帰表示の上限。print_options() の返す設定を変更して使う。
 * @note max_items は各コンテナの表示件数、max_depth は入れ子の深さ。0 も指定できる。
 * 最外層を深さ 0 とし、max_depth に達したコンテナは空でも ... に省略する。
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
// 標準のコンテナアダプタだけを識別し、range や独自型の判定に影響させない。
template <class T>
inline constexpr std::string_view adapter_name = "";
template <class T, class Container>
inline constexpr std::string_view adapter_name<std::queue<T, Container>> = "queue";
template <class T, class Container>
inline constexpr std::string_view adapter_name<std::stack<T, Container>> = "stack";
template <class T, class Container, class Compare>
inline constexpr std::string_view adapter_name<std::priority_queue<T, Container, Compare>> = "priority_queue";

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
    if constexpr (std::is_same_v<U, __int128_t>) {
        out << ::to_string_int128(value);
    } else if constexpr (std::is_same_v<U, __uint128_t>) {
        out << ::to_string_uint128(value);
    } else if constexpr (std::is_same_v<U, bool>) {
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
    } else if constexpr (!adapter_name<U>.empty()) {
        if (depth >= options.max_depth) { out << "..."; return; }
        out << adapter_name<U> << '[';
        if (value.empty() || options.max_items == 0) {
            if (!value.empty()) out << "...";
        } else {
            auto copy = value;
            std::size_t count = 0;
            while (!copy.empty() && count < options.max_items) {
                if (count) out << ", ";
                if constexpr (requires { copy.front(); }) {
                    write(out, copy.front(), depth + 1, options);
                } else {
                    write(out, copy.top(), depth + 1, options);
                }
                copy.pop();
                ++count;
            }
            if (!copy.empty()) out << ", ...";
        }
        out << ']';
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
 * @tparam T 128bit整数、文字列、range、pair / tuple、queue / stack / priority_queue、または ostream に出力可能な型。
 * @param value 表示する値。STL の operator<< は追加・変更しない。
 * @param options 各コンテナの件数・深さの上限。
 * @return 文字列・文字は引用符付き。未対応型は <unprintable>。
 * @pre コンテナアダプタの要素・内部コンテナ・比較関数はコピー可能であること。
 * @note vector / array / deque / list は []、set / map 系（multi / unordered を含む）は {} で表示する。
 * これらと pair / tuple / コンテナアダプタは任意に入れ子にでき、各階層に options を適用する。
 * queue は front から、stack は top から、priority_queue は比較関数に従った pop 順に表示する。
 * 例: queue[1, 2]、stack[2, 1]、priority_queue[3, 1]。元のコンテナは変更しない。
 * @note GNU signed/unsigned 128bitは10進で表示する。cpp_intは利用側でBoostをincludeすれば標準のストリーム出力で表示可能。
 * unordered コンテナの順序は不定。文字列自体の長さは制限しない。
 * @par Complexity
 * 表示する要素数と文字数に比例する時間・出力空間。
 * ただし、要素数 N のアダプタは表示件数 K を制限してもコピー・破棄に O(N) 時間・コピーに O(N) 追加空間を使う。
 * priority_queue はさらに O(K log N) の取り出し時間が必要。要素の表示・コピーと比較は別途加算する。
 * 空・max_items = 0・深さ上限で省略するアダプタはコピーしない。ネスト時は各アダプタのコストを加算する。
 * @code
 * #include "library/debug/debug.hpp"
 * #include <set>
 * int main() {
 *     std::vector<std::set<int>> groups{{1, 2}, {3}};
 *     std::priority_queue<int> heap;
 *     heap.push(1); heap.push(3);
 *     debug(groups, heap); // LOCAL 時に [{1, 2}, {3}] と priority_queue[3, 1] を表示
 *     const auto text = cpl::debug::repr(heap, {1, 8}); // priority_queue[3, ...]
 *     (void)text;
 * }
 * @endcode
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

/**
 * @brief テーブルの表示範囲・幅。範囲は 0-indexed の半開区間。
 * @note row_end / col_end は終端を自動で切り詰める。max_rows / max_cols は表示件数の上限。
 * @code
 * #include "library/debug/debug.hpp"
 * int main() {
 *     std::vector<std::vector<int>> dp{{0, 1}, {2, 3}};
 *     cpl::debug::TableOptions view;
 *     view.row_begin = 1;
 *     view.max_cols = 2;
 *     CPL_DEBUG_TABLE(dp, view);
 * }
 * @endcode
 */
struct TableOptions {
    std::size_t row_begin = 0, row_end = std::numeric_limits<std::size_t>::max();
    std::size_t col_begin = 0, col_end = std::numeric_limits<std::size_t>::max();
    std::size_t max_rows = 20, max_cols = 20;
    std::size_t min_width = 3, max_cell_width = 80;
};

/**
 * @brief 2 次元 vector を行・列の添字と ASCII の罫線付きで文字列化する。
 * @tparam T repr で表示するセルの型。
 * @param table 行ごとに長さが異なってもよい表。存在しないセルは空欄。
 * @param options 表示範囲と最大件数。幅は文字数ではなくバイト数で数える。
 * @return 表の文字列。範囲外・空の表示範囲なら (empty view)。省略時は末尾に ...。
 * @note LOCAL に関係なく動く補助関数。通常は CPL_DEBUG_TABLE を使用する。
 * @par Complexity
 * 表示する R 行 C 列とセルの文字列化のコストに比例する時間・空間。
 */
template <class T>
std::string render_table(const std::vector<std::vector<T>>& table, const TableOptions& options = {}) {
    const std::size_t begin = std::min(options.row_begin, table.size());
    const std::size_t end = std::min(options.row_end, table.size());
    if (begin >= end || options.max_rows == 0 || options.max_cols == 0) return "(empty view)\n";
    const std::size_t rows = std::min(end - begin, options.max_rows);
    std::size_t columns = 0;
    for (std::size_t i = 0; i < rows; ++i) columns = std::max(columns, table[begin + i].size());
    const std::size_t all_columns = columns;
    columns = std::min(columns, options.col_end);
    if (options.col_begin >= columns) return "(empty view)\n";
    const std::size_t count = std::min(columns - options.col_begin, options.max_cols);
    std::vector<std::size_t> width(count, options.min_width);
    std::vector<std::vector<std::string>> cells(rows, std::vector<std::string>(count));
    for (std::size_t j = 0; j < count; ++j) width[j] = std::max(width[j], std::to_string(options.col_begin + j).size());
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < count; ++j) {
            const auto col = options.col_begin + j;
            if (col >= table[begin + i].size()) continue;
            auto& cell = cells[i][j];
            cell = repr(table[begin + i][col]);
            for (char& ch : cell) if (ch == '\n' || ch == '\r' || ch == '\t') ch = ' ';
            if (cell.size() > options.max_cell_width) cell = cell.substr(0, options.max_cell_width) + "...";
            width[j] = std::max(width[j], cell.size());
        }
    }
    const std::size_t index_width = std::max<std::size_t>(3, std::to_string(begin + rows - 1).size());
    std::ostringstream out;
    auto rule = [&]() {
        out << '+' << std::string(index_width + 2, '-');
        for (auto w : width) out << '+' << std::string(w + 2, '-');
        out << "+\n";
    };
    auto field = [&](std::string_view value, std::size_t w) { out << "| " << std::setw(static_cast<int>(w)) << value << ' '; };
    rule();
    field("r/c", index_width);
    for (std::size_t j = 0; j < count; ++j) field(std::to_string(options.col_begin + j), width[j]);
    out << "|\n";
    rule();
    for (std::size_t i = 0; i < rows; ++i) {
        field(std::to_string(begin + i), index_width);
        for (std::size_t j = 0; j < count; ++j) field(cells[i][j], width[j]);
        out << "|\n";
    }
    rule();
    if (begin != 0 || rows < table.size() || options.col_begin != 0 || options.col_begin + count < all_columns) out << "... (limited view)\n";
    return out.str();
}

/// @brief 表示元情報と表を cerr に出す。引数は CPL_DEBUG_TABLE と同じ。
template <class T>
void log_table(const char* file, int line, const char* name, const std::vector<std::vector<T>>& table,
               const TableOptions& options = {}) {
    std::cerr << file << ':' << line << " | " << name << '\n' << render_table(table, options);
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

#ifdef LOCAL
#define CPL_DEBUG_TABLE(table, ...) do { ::cpl::debug::log_table(__FILE__, __LINE__, #table, (table) __VA_OPT__(,) __VA_ARGS__); } while (false)
#else
#define CPL_DEBUG_TABLE(...) ((void)0)
#endif
#if !defined(CPL_DEBUG_NO_SHORT_MACRO) && !defined(debug_table)
#define debug_table(...) CPL_DEBUG_TABLE(__VA_ARGS__)
#endif
