#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

/**
 * @brief H 行 W 列のグリッドへの長方形加算を、2 次元いもす法で一括処理する。
 * @pre T{} が零元であり、途中の値を含め加減算の結果が T に収まること。
 * @note add は build 前に行い、build は 1 回だけ呼ぶ。data には境界処理用の行と列を含む。
 */
template <class T>
struct Imos2D {
    int H, W;
    std::vector<std::vector<T>> data;

    /// @brief 零で初期化したグリッドを O((H+1)(W+1)) 時間・空間で構築する。@pre H, W >= 0。
    Imos2D(int H, int W) : H(H), W(W), data(H + 1, std::vector<T>(W + 1, T{})) {
        assert(H >= 0 && W >= 0);
    }

    /// @brief [u, d) x [l, r) に val を O(1) で加算する。範囲外は切り詰め、空領域は無視する。
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
        for (int i = 0; i <= H; ++i)
            for (int j = 1; j <= W; ++j) data[i][j] += data[i][j - 1];
        for (int j = 0; j <= W; ++j)
            for (int i = 1; i <= H; ++i) data[i][j] += data[i - 1][j];
    }

    /// @brief セルの値を O(1) で返す。@pre build 実行済み、0 <= i < H、0 <= j < W。
    T get(int i, int j) const { return data[i][j]; }
};
