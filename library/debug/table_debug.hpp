#pragma once

/**
 * @brief テーブル表示 API の互換用ヘッダ。新規コードは debug.hpp だけで利用できる。
 * @note TableOptions、render_table、log_table と表示マクロは debug.hpp で定義する。
 * @code
 * #include "library/debug/table_debug.hpp"
 * int main() {
 *     std::vector<std::vector<int>> dp{{0, 1}, {2, 3}};
 *     cpl::debug::TableOptions view;
 *     CPL_DEBUG_TABLE(dp, view);
 * }
 * @endcode
 */
#include "debug.hpp"
