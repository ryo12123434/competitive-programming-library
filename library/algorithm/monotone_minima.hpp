#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <vector>

/// @brief 各行の最小列（列なしなら-1）と、cost の呼び出し回数。
struct MonotoneMinimaResult {
    std::vector<int> argmin;
    std::size_t evaluations = 0;
};
/**
 * @brief 行最小列が広義単調増加な行列の、各行の最小列を分割統治で求める。
 * @param rows 行数。@param columns 列数。0なら各行に-1を返しcallbackは呼ばない。
 * @param cost cost(row,column) で値を返す純粋な関数。返り値はコピー・代入可能。
 * @param compare 小さい値を選ぶ狭義弱順序。比較同値は最も左の列。
 * @pre rows,columns>=0。左端最小列が行番号順で広義単調増加すること。一般の行列には使用不可。
 * @note 単調性は実行時には検査しない。Monge性は十分条件だが必須ではない。
 * @par Complexity
 * cost・比較・コピーが O(1) なら O(H+W log(H+1)) 時間、O(H+log(H+1)) 追加空間。
 * @code
 * #include "library/algorithm/monotone_minima.hpp"
 * #include <cassert>
 * int main(){auto r=monotone_minima(4,5,[](int i,int j){return (i-j)*(i-j);});assert((r.argmin==std::vector<int>{0,1,2,3}));}
 * @endcode
 */
template <class Cost, class Compare = std::less<>>
MonotoneMinimaResult monotone_minima(int rows, int columns, Cost cost, Compare compare = {}) {
    assert(rows >= 0 && columns >= 0);
    MonotoneMinimaResult result;
    result.argmin.assign(rows, -1);
    if (!rows || !columns)
        return result;
    auto solve = [&](auto &&self, int top, int bottom, int left, int right) -> void {
        if (top == bottom)
            return;
        int row = top + (bottom - top) / 2, best = left;
        auto value = cost(row, left);
        ++result.evaluations;
        for (int j = left + 1; j < right; ++j) {
            auto candidate = cost(row, j);
            ++result.evaluations;
            if (compare(candidate, value)) {
                value = candidate;
                best = j;
            }
        }
        result.argmin[row] = best;
        self(self, top, row, left, best + 1);
        self(self, row + 1, bottom, best, right);
    };
    solve(solve, 0, rows, 0, columns);
    return result;
}
