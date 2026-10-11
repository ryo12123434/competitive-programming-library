#pragma once

#include <cassert>
#include <limits>
#include <vector>

/**
 * @brief 静的な遷移 next[v] を二分累乗で繰り返す Doubling。
 * @note 添字は 0-indexed。遷移先 -1 は終端であり、以後も -1 のまま。
 * @par Complexity
 * N 要素・L 段に対して構築 O(NL) 時間・空間、jump O(L)。
 *
 * @code
 * #include "library/algorithm/doubling.hpp"
 * #include <cassert>
 * int main() {
 *     Doubling doubling(std::vector<int>{1, 2, -1});
 *     assert(doubling.jump(0, 2) == 2);
 *     assert(doubling.jump(0, 3) == -1);
 * }
 * @endcode
 */
class Doubling {
public:
    /**
     * @brief 2^0 から 2^(levels-1) 回の遷移を前計算する。
     * @pre next.size() は int に収まること。各遷移先は -1 または [0, N)。
     * @pre 1 <= levels <= 64。
     * @note 既定の 64 段では unsigned long long の全範囲を扱える。
     *
     * @param next 各頂点の遷移先。-1 は終端。
     * @param levels 前計算する二進桁数。既定値は 64。
     */
    explicit Doubling(const std::vector<int>& next, int levels = 64) {
        assert(next.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        assert(1 <= levels && levels <= 64);
        int n = static_cast<int>(next.size());
        for (int v : next) assert(-1 <= v && v < n);
        table_.assign(levels, std::vector<int>(n, -1));
        table_[0] = next;
        for (int k = 1; k < levels; ++k) {
            for (int v = 0; v < n; ++v) {
                int mid = table_[k - 1][v];
                if (mid != -1) table_[k][v] = table_[k - 1][mid];
            }
        }
    }

    /**
     * @brief v から steps 回遷移した先を返す。途中で終端に達した場合は -1。
     * @pre v は -1 または [0, N)。L < 64 の場合、steps < 2^L。
     *
     * @param v 出発頂点（0-indexed）。-1 も指定可能。
     * @param steps 非負の遷移回数。0 なら v をそのまま返す。
     * @return 到達頂点、終端に達した場合は -1。
     * @par Complexity
     * O(levels) 時間、追加空間 O(1)。
     */
    int jump(int v, unsigned long long steps) const {
        assert(-1 <= v && v < static_cast<int>(table_[0].size()));
        assert(table_.size() == 64 || (steps >> table_.size()) == 0);
        for (int k = 0; steps != 0 && v != -1; ++k, steps >>= 1) {
            if (steps & 1ULL) v = table_[k][v];
        }
        return v;
    }

private:
    std::vector<std::vector<int>> table_;
};
