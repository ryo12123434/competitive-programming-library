#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 静的配列の結合的な演算を O(1) で集約する Disjoint Sparse Table。
 * @tparam T コピー可能な集約値。デフォルト構築可能でなくてもよい。
 * @tparam op T op(T left,T right)。結合律のみ必要で、冪等性・可換性・単位元は不要。
 * @note 0-indexed、区間は [l,r)。空配列も構築可だが、クエリは非空区間のみ。
 * 既存 SparseTable は冪等性が必要。こちらは和・行列積・関数合成にも使える。
 * @pre N<=INT_MAX、演算結果は T に収まること。構築後の値は更新できない。
 * @par Complexity
 * コピーと op が O(1) なら構築 O(N log(N+1)) 時間・空間、prod O(1)。
 * @code
 * #include "library/data_structure/disjoint_sparse_table.hpp"
 * #include <cassert>
 * long long sum(long long a,long long b){return a+b;}
 * int main() {
 *     DisjointSparseTable<long long,sum> table(std::vector<long long>{2,3,4});
 *     assert(table.prod(1,3)==7);
 * }
 * @endcode
 */
template <class T, T (*op)(T, T)>
class DisjointSparseTable {
public:
    /// @brief a をコピーして前処理する。
    explicit DisjointSparseTable(const std::vector<T>& a) : values_(a) {
        assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const auto n = a.size();
        for (std::size_t half = 1; half < n; half *= 2) {
            std::vector<T> row = a;
            for (std::size_t begin = 0; begin < n; begin += half * 2) {
                std::size_t mid = std::min(begin + half, n), end = std::min(begin + half * 2, n);
                for (std::size_t i = mid; i > begin + 1; --i) row[i - 2] = op(a[i - 2], row[i - 1]);
                for (std::size_t i = mid + 1; i < end; ++i) row[i] = op(row[i - 1], a[i]);
            }
            table_.push_back(std::move(row));
        }
    }
    /// @brief 要素数。O(1)。
    int size() const { return static_cast<int>(values_.size()); }
    /// @brief [l,r) を左から集約した値。@pre 0<=l<r<=size()。O(1)。
    T prod(int l, int r) const {
        assert(0 <= l && l < r && r <= size());
        if (r - l == 1) return values_[l];
        unsigned int bits = static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r - 1);
        auto k = std::bit_width(bits) - 1;
        return op(table_[k][l], table_[k][r - 1]);
    }
private:
    std::vector<T> values_;
    std::vector<std::vector<T>> table_;
};
