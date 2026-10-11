#pragma once

#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief 文字列を連続する同一文字ごとにランレングス圧縮する。
 * @param s 圧縮する文字列
 * @return (文字, 連続個数) の列。空文字列なら空の vector
 * @pre s.size() は int に収まること。
 * @par Complexity
 * O(|s|)
 *
 * @note 結果は元の順序で並ぶ。出力空間 O(K)（K は連続区間数）。
 *
 * @code
 * #include "library/string/run_length_encoding.hpp"
 * #include <cassert>
 * int main() {
 *     assert((RLE("aaabb") == std::vector<std::pair<char, int>>{{'a', 3}, {'b', 2}}));
 * }
 * @endcode
 */
inline std::vector<std::pair<char, int>> RLE(const std::string& s) {
    assert(s.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    std::vector<std::pair<char, int>> res;
    if (s.empty()) return res;

    char c = s[0];
    int cnt = 1;

    for (int i = 1; i < static_cast<int>(s.size()); ++i) {
        if (s[i] == c) {
            ++cnt;
        } else {
            res.push_back({c, cnt});
            c = s[i];
            cnt = 1;
        }
    }

    res.push_back({c, cnt});
    return res;
}
