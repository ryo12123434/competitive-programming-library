#pragma once

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "debug.hpp"

namespace cpl::debug {
/**
 * @brief テーブルの表示範囲・幅。範囲は 0-indexed の半開区間。
 * @note row_end / col_end は終端を自動で切り詰める。max_rows / max_cols は表示件数の上限。
 * @code
 * #include "library/debug/table_debug.hpp"
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

#ifdef LOCAL
#define CPL_DEBUG_TABLE(table, ...) do { ::cpl::debug::log_table(__FILE__, __LINE__, #table, (table) __VA_OPT__(,) __VA_ARGS__); } while (false)
#else
#define CPL_DEBUG_TABLE(...) ((void)0)
#endif
#if !defined(CPL_DEBUG_NO_SHORT_MACRO) && !defined(debug_table)
#define debug_table(...) CPL_DEBUG_TABLE(__VA_ARGS__)
#endif
