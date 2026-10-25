#pragma once

#include <cassert>
#include <limits>
#include <vector>

/**
 * @brief 静的な 2 次元配列の矩形和を求める。
 * @tparam T T{} が零元で、+= と -= を持つ可換加法群の型（long long、ACL modint など）。
 * @pre 構築・クエリ途中の加減算も T で表現できること。浮動小数点型では丸め誤差が生じる。
 * @note query の引数順は Imos2D::add と同じ (上端, 下端, 左端, 右端)。範囲外の切り詰めは行わない。
 * @par Complexity
 * 構築 O((H+1)(W+1)) 時間・空間（H,W > 0 なら O(HW)）、query O(1)。
 * @code
 * #include "library/data_structure/prefix_sum_2d.hpp"
 * #include <cassert>
 * int main() {
 *     PrefixSum2D<long long> sum(std::vector<std::vector<long long>>{{1, 2, 3}, {4, 5, 6}});
 *     assert(sum.height() == 2 && sum.width() == 3);
 *     assert(sum.query(0, 2, 1, 3) == 16); // [0,2) x [1,3)
 *     assert(sum.query(1, 1, 0, 3) == 0);
 * }
 * @endcode
 */
template <class T>
class PrefixSum2D {
public:
    /**
     * @brief 長方形の 2 次元配列から累積和を構築する。
     * @param values 全行の列数が等しい配列。元の配列は変更しない。
     * @pre 行数・列数は int に収まる。
     * @note 空配列は 0x0、空行 H 個は Hx0 として扱う。構築後の値変更は反映しない。
     * @par Complexity
     * O((H+1)(W+1)) 時間・空間。
     */
    explicit PrefixSum2D(const std::vector<std::vector<T>>& values) {
        assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        height_ = static_cast<int>(values.size());
        const std::size_t width = values.empty() ? 0 : values.front().size();
        assert(width <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        width_ = static_cast<int>(width);
        sum_.assign(values.size() + 1, std::vector<T>(width + 1, T{}));
        for (int y = 0; y < height_; ++y) {
            assert(values[y].size() == width);
            T row_sum{};
            for (int x = 0; x < width_; ++x) {
                row_sum += values[y][x];
                sum_[y + 1][x + 1] = sum_[y][x + 1];
                sum_[y + 1][x + 1] += row_sum;
            }
        }
    }

    /// @brief 行数を O(1) で返す。
    int height() const { return height_; }
    /// @brief 列数を O(1) で返す。
    int width() const { return width_; }
    /**
     * @brief 半開矩形 [u,d) x [l,r) の和を O(1) で返す。
     * @param u 上端の行（含む）。@param d 下端の行（含まない）。
     * @param l 左端の列（含む）。@param r 右端の列（含まない）。
     * @pre 0 <= u <= d <= height()、0 <= l <= r <= width()。
     * @return 矩形内の和。u == d または l == r なら T{}。
     */
    T query(int u, int d, int l, int r) const {
        assert(0 <= u && u <= d && d <= height_ && 0 <= l && l <= r && r <= width_);
        if (u == d || l == r) return T{};
        T result = sum_[d][r];
        result -= sum_[u][r];
        result -= sum_[d][l];
        result += sum_[u][l];
        return result;
    }

private:
    int height_, width_;
    std::vector<std::vector<T>> sum_;
};
