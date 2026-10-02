#pragma once

#include <bit>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 静的配列の非空区間を O(1) で集計する Sparse Table。
 * @tparam op 結合的かつ冪等な演算。min / max / gcd など。和には使えない。
 * @note 添字は 0-indexed、区間は [l, r)。配列の更新・空区間の集計は扱わない。
 * @par 計算量 op が O(1) のとき構築 O(N log N) 時間・空間、prod O(1)。
 */
template <class T, T (*op)(T, T)>
class SparseTable {
public:
    /// @brief a を前計算する。@pre a.size() は int に収まること。空配列も構築可能。
    explicit SparseTable(const std::vector<T>& a) {
        assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        n_ = static_cast<int>(a.size());
        if (n_ == 0) return;
        table_.push_back(a);
        int levels = static_cast<int>(std::bit_width(static_cast<unsigned int>(n_)));
        for (int k = 1; k < levels; ++k) {
            int length = 1 << k, half = length >> 1;
            std::vector<T> row;
            row.reserve(n_ - length + 1);
            for (int i = 0; i <= n_ - length; ++i) {
                row.push_back(op(table_[k - 1][i], table_[k - 1][i + half]));
            }
            table_.push_back(std::move(row));
        }
    }

    /// @brief 要素数を O(1) で返す。
    int size() const { return n_; }

    /// @brief [l, r) の集計結果を O(1) で返す。@pre 0 <= l < r <= size()。
    T prod(int l, int r) const {
        assert(0 <= l && l < r && r <= n_);
        int k = static_cast<int>(std::bit_width(static_cast<unsigned int>(r - l))) - 1;
        return op(table_[k][l], table_[k][r - (1 << k)]);
    }

private:
    int n_;
    std::vector<std::vector<T>> table_;
};
