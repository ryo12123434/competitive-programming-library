#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

/**
 * @brief 整数列を座標圧縮する。
 *
 * 各要素を、元の列に現れる異なる値を昇順に並べたときの 0-indexed の順位へ変換する。
 *
 * @param a 圧縮する整数列
 * @return 座標圧縮後の整数列
 * @pre a.size() は int に収まること。
 * @par Complexity
 * O(N log N)
 *
 * @note 元の値を復元する表は返さない。重複する値は同じ順位になる。追加空間 O(N)。
 *
 * @code
 * #include "library/data_structure/coordinate_compression.hpp"
 * #include <cassert>
 * int main() {
 *     assert((compress({10, -2, 10}) == std::vector<int>{1, 0, 1}));
 * }
 * @endcode
 */
inline std::vector<int> compress(std::vector<int> a){
    // コピー
    std::vector<int> b = a;
    
    // B を小さい順にソート
    std::sort(b.begin(), b.end());
    
    // B から重複を除去する
    b.erase(std::unique(b.begin(), b.end()), b.end());

    // 座標圧縮した結果を求める
    std::vector<int> res(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        res[i] = static_cast<int>(std::lower_bound(b.begin(), b.end(), a[i]) - b.begin());
    }
    return res;
};
