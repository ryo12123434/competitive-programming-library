#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

/**
 * @brief H 行 W 列のグリッドへの長方形加算を、2 次元いもす法で一括処理する。
 * @pre T{} が零元であり、途中の値を含め加減算の結果が T に収まること。
 * @note add は build 前に行い、build は 1 回だけ呼ぶ。data には境界処理用の行と列を含む。
 *
 * @tparam T T{} を零元とする可換加法群の型。+=、-= に対応し、既定の val を使う場合は 1 から変換可能。
 * @par Complexity
 * 構築 O((H+1)(W+1)) 時間・保持空間、build は同じ時間で追加空間 O(1)。add / get O(1)。
 *
 * @code
 * #include "library/data_structure/imos_2d.hpp"
 * #include <cassert>
 * int main() {
 *     Imos2D<long long> imos(2, 3);
 *     imos.add(0, 2, 1, 3, 4);
 *     imos.build();
 *     assert(imos.get(1, 2) == 4);
 * }
 * @endcode
 */
template <class T>
struct Imos2D {
    int H, W;
    std::vector<std::vector<T>> data;

    /**
     * @brief 零で初期化したグリッドを O((H+1)(W+1)) 時間・空間で構築する。
     * @pre H, W >= 0。
     * @param H 行数。行添字は [0, H)。
     * @param W 列数。列添字は [0, W)。
     */
    Imos2D(int H, int W) : H(H), W(W) {
        assert(H >= 0 && W >= 0);
        data.assign(static_cast<std::size_t>(H) + 1, std::vector<T>(static_cast<std::size_t>(W) + 1, T{}));
    }

    /**
     * @brief [u, d) x [l, r) に val を O(1) で加算する。範囲外は切り詰め、空領域は無視する。
     * @param u 上端の行（含む）。
     * @param d 下端の行（含まない）。
     * @param l 左端の列（含む）。
     * @param r 右端の列（含まない）。
     * @param val 各セルへ加算する値。
     * @pre build 未実行。
     */
    void add(int u, int d, int l, int r, T val = 1) {
        u = std::clamp(u, 0, H); d = std::clamp(d, 0, H);
        l = std::clamp(l, 0, W); r = std::clamp(r, 0, W);
        if (u >= d || l >= r) return;
        data[u][l] += val;
        data[u][r] -= val;
        data[d][l] -= val;
        data[d][r] += val;
    }

    /// @brief 差分を各セルの値に O((H+1)(W+1)) で変換する。@pre build 未実行。
    void build() {
        for (std::size_t i = 0; i <= static_cast<std::size_t>(H); ++i)
            for (std::size_t j = 1; j <= static_cast<std::size_t>(W); ++j) data[i][j] += data[i][j - 1];
        for (std::size_t j = 0; j <= static_cast<std::size_t>(W); ++j)
            for (std::size_t i = 1; i <= static_cast<std::size_t>(H); ++i) data[i][j] += data[i - 1][j];
    }

    /**
     * @brief セルの値を O(1) で返す。
     * @pre build 実行済み、0 <= i < H、0 <= j < W。
     * @param i 行番号（0-indexed）。
     * @param j 列番号（0-indexed）。
     * @return 長方形加算後のセルの値。
     */
    T get(int i, int j) const { return data[i][j]; }
};
