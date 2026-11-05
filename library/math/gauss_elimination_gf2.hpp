#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

/// @brief GF(2)連立方程式の結果。解なしでも rank と free_variables は係数行列の値を返す。
struct GF2Solution {
    int rank = 0, free_variables = 0;
    bool solvable = true;
    std::vector<int> solution;
    std::vector<std::vector<int>> nullspace;
};
/**
 * @brief 動的な列数を持つGF(2)連立一次方程式。64bit固定のXorBasisと違い、方程式の解を復元する。
 * @note 変数は0-indexed。係数は64bitワードに詰め、演算はXOR。整数上の連立方程式ではない。
 * @par Complexity
 * M行N列・rank=r。solve は O(MN+Mr ceil(N/64)) 時間、O(M ceil(N/64)+N) 追加空間。
 * 解空間基底を要求すると更に O((N-r)N) 時間・空間。登録行の保持空間は O(M ceil(N/64)+M)。
 * @code
 * #include "library/math/gauss_elimination_gf2.hpp"
 * #include <cassert>
 * int main(){GF2LinearSystem system(3);system.add_equation({0,1},true);system.add_equation({1,2},false);auto r=system.solve(true);assert(r.solvable&&r.rank==2&&r.free_variables==1&&r.nullspace.size()==1);}
 * @endcode
 */
class GF2LinearSystem {
public:
    /// @brief variables列の空の方程式系。@pre variables>=0。
    explicit GF2LinearSystem(int variables) : n_(variables) {
        assert(variables >= 0);
        words_ = (static_cast<std::size_t>(variables) + 63) / 64;
    }
    /// @brief 係数1の変数添字と右辺を追加。重複添字は偶数回なら相殺。@pre 添字は[0,N)、行数はINT_MAX以下。
    void add_equation(const std::vector<int> &ones, bool rhs) {
        std::vector<std::uint64_t> row(words_);
        for (int i : ones) {
            assert(0 <= i && i < n_);
            row[i / 64] ^= std::uint64_t(1) << (i % 64);
        }
        add_row(row, rhs);
    }
    /// @brief ビット圧縮した係数行を追加。@pre ワード数はceil(N/64)。末尾の余分なビットは無視する。
    void add_row(std::vector<std::uint64_t> row, bool rhs) {
        assert(row.size() == words_ &&
               rows_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        if (n_ % 64)
            row.back() &= (std::uint64_t(1) << (n_ % 64)) - 1;
        rows_.push_back(std::move(row));
        rhs_.push_back(rhs);
    }
    /// @brief 解を1つ返す（自由変数を0に選ぶ）。include_basis=trueなら同次解空間の基底も返す。非破壊。
    GF2Solution solve(bool include_basis = false) const {
        auto a = rows_;
        auto b = rhs_;
        int m = static_cast<int>(a.size()), rank = 0;
        std::vector<int> pivots;
        for (int col = 0; col < n_ && rank < m; ++col) {
            int pivot = rank;
            while (pivot < m && !((a[pivot][col / 64] >> (col % 64)) & 1))
                ++pivot;
            if (pivot == m)
                continue;
            std::swap(a[rank], a[pivot]);
            std::swap(b[rank], b[pivot]);
            for (int i = 0; i < m; ++i)
                if (i != rank && ((a[i][col / 64] >> (col % 64)) & 1)) {
                    for (std::size_t w = col / 64; w < words_; ++w)
                        a[i][w] ^= a[rank][w];
                    b[i] ^= b[rank];
                }
            pivots.push_back(col);
            ++rank;
        }
        GF2Solution result;
        result.rank = rank;
        result.free_variables = n_ - rank;
        for (int i = rank; i < m; ++i)
            if (b[i]) {
                result.solvable = false;
                return result;
            }
        result.solution.assign(n_, 0);
        for (int i = 0; i < rank; ++i)
            result.solution[pivots[i]] = b[i];
        if (include_basis) {
            std::vector<bool> pivot(n_);
            for (int col : pivots)
                pivot[col] = true;
            for (int col = 0; col < n_; ++col)
                if (!pivot[col]) {
                    std::vector<int> v(n_);
                    v[col] = 1;
                    for (int i = 0; i < rank; ++i)
                        v[pivots[i]] = static_cast<int>((a[i][col / 64] >> (col % 64)) & 1);
                    result.nullspace.push_back(std::move(v));
                }
        }
        return result;
    }

private:
    int n_;
    std::size_t words_;
    std::vector<std::vector<std::uint64_t>> rows_;
    std::vector<unsigned char> rhs_;
};
