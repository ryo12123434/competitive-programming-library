#pragma once

#include <cassert>
#include <limits>
#include <vector>

/**
 * @brief 通常の加算・乗算による動的サイズの行列。長方形の乗算と正方行列の累乗に対応する。
 * @tparam T T{} が加法単位元、T{1} が乗法単位元で、+= と * が可能な型。ACL modint も使用可。
 * @pre 加算は可換・結合的、乗算は結合的で、加算に分配的であること。零元による積は零元。
 * @pre 途中の加算・乗算も T で表現可能なこと。浮動小数点では丸めにより厳密な結合則は成り立たない。
 * @note dynamic_modint の法は使用中に変更しない。要素は (行, 列) で 0-indexed。
 * @par Complexity
 * HxW と WxK の乗算 O(HWK + HK + H) 時間・結果空間 O(HK + H)。
 * KxK の pow(e) は O(K^3 log(e+1) + K^2) 時間・追加空間 O(K^2)（K > 0）。
 * @code
 * #include "library/math/matrix.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main() {
 *     using Mint = atcoder::modint998244353;
 *     Matrix<Mint> a(std::vector<std::vector<Mint>>{{1, 1}, {1, 0}});
 *     auto result = a.pow(10);
 *     assert(result(0, 1).val() == 55);
 *     assert(a.pow(0)(1, 1) == 1);
 *     Matrix<long long> row(1, 2, 3), column(2, 1, 4);
 *     assert((row * column)(0, 0) == 24);
 * }
 * @endcode
 */
template <class T>
class Matrix {
public:
    /**
     * @brief H 行 W 列を value で初期化する。
     * @param height 行数。@param width 列数。@param value 全要素の初期値（既定は零元）。
     * @pre height, width >= 0。
     * @note 0xW と Hx0 も構築可能。既定構築は 0x0。
     * @par Complexity
     * O(HW + H) 時間・空間。
     */
    explicit Matrix(int height = 0, int width = 0, const T& value = T{}) : height_(height), width_(width) {
        assert(height >= 0 && width >= 0);
        data_.resize(height);
        for (auto& row : data_) row.assign(width, value);
    }

    /**
     * @brief 長方形の 2 次元配列をコピーして構築する。
     * @param values 各行の列数が等しい配列。空配列は 0x0。
     * @pre 行数・列数が int に収まり、各行の長さが等しいこと。
     * @par Complexity
     * O(HW + H) 時間・空間。
     */
    explicit Matrix(const std::vector<std::vector<T>>& values) {
        assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const std::size_t width = values.empty() ? 0 : values.front().size();
        assert(width <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        for ([[maybe_unused]] const auto& row : values) assert(row.size() == width);
        height_ = static_cast<int>(values.size());
        width_ = static_cast<int>(width);
        data_ = values;
    }

    /// @brief 行数を O(1) で返す。
    int height() const { return height_; }
    /// @brief 列数を O(1) で返す。
    int width() const { return width_; }
    /// @brief 要素への変更可能な参照を O(1) で返す。@param y 行。@param x 列。@pre (y,x) は行列内。
    T& operator()(int y, int x) { check_index(y, x); return data_[y][x]; }
    /// @brief 要素への const 参照を O(1) で返す。@param y 行。@param x 列。@pre (y,x) は行列内。
    const T& operator()(int y, int x) const { check_index(y, x); return data_[y][x]; }

    /**
     * @brief n 次の単位行列を返す。
     * @param n 次数（0 も可）。@pre n >= 0。
     * @return 対角要素 T{1}、それ以外 T{} の nxn 行列。
     * @par Complexity
     * O(n^2 + 1) 時間・空間。
     */
    static Matrix identity(int n) {
        Matrix result(n, n);
        for (int i = 0; i < n; ++i) result(i, i) = T{1};
        return result;
    }

    /**
     * @brief この行列に rhs を右から掛けた積を返す。元の行列は変更しない。
     * @param rhs 右側の行列。@pre width() == rhs.height()。不一致は assert で検出する。
     * @return height() x rhs.width() 行列。共通次元が 0 なら全要素 T{}。
     * @par Complexity
     * HxW と WxK に対して O(HWK + HK + H) 時間・結果空間 O(HK + H)。
     */
    Matrix operator*(const Matrix& rhs) const {
        assert(width_ == rhs.height_);
        Matrix result(height_, rhs.width_);
        for (int i = 0; i < height_; ++i)
            for (int k = 0; k < width_; ++k)
                for (int j = 0; j < rhs.width_; ++j)
                    result.data_[i][j] += data_[i][k] * rhs.data_[k][j];
        return result;
    }

    /**
     * @brief 正方行列の非負整数乗を二分累乗で返す。元の行列は変更しない。
     * @param exponent 指数。0 は単位行列、1 は元の行列。unsigned long long の全範囲に対応。
     * @pre height() == width()。長方形の累乗は指数 0 でも assert で検出する。
     * @return この行列の exponent 乗。0x0 行列の累乗は常に 0x0。
     * @note NDEBUG 時は前提条件の assert が無効になる。負の整数を渡して符号なしへ変換しないこと。
     * @par Complexity
     * K > 0 で O(K^3 log(exponent+1) + K^2) 時間、追加空間 O(K^2)。
     */
    Matrix pow(unsigned long long exponent) const {
        assert(height_ == width_);
        Matrix result = identity(height_), base = *this;
        while (exponent != 0) {
            if (exponent & 1ULL) result = result * base;
            exponent >>= 1;
            if (exponent != 0) base = base * base;
        }
        return result;
    }

private:
    int height_, width_;
    std::vector<std::vector<T>> data_;
    void check_index([[maybe_unused]] int y, [[maybe_unused]] int x) const {
        assert(0 <= y && y < height_ && 0 <= x && x < width_);
    }
};
