#pragma once

#include "miller_rabin.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

/** @brief 素数法連立方程式の結果。solutionは自由変数0の特解、nullspaceは斉次解の基底。
 * 解なしならsolvable=falseかつ両配列は空。rankは係数行列の階数、free_variablesは変数数-rank。
 */
template <class Mint> struct ModLinearSolution {
    int rank = 0, free_variables = 0;
    bool solvable = true;
    std::vector<Mint> solution;
    std::vector<std::vector<Mint>> nullspace;
};

/**
 * @brief 素数法modintの連立一次方程式を掃き出し法で解く。
 * @tparam Mint 素数法のACL modint相当。mod()・四則演算・==を持つ体。
 * @pre 構築後にmod()を変更しないこと。合成数法は構築時invalid_argument。
 * @note 方程式0本・変数0個も許す。行列をコピーして解くため元の方程式は変化しない。
 * @par Complexity M式N変数でO(MN min(M,N))体演算、O(MN+N)作業空間。
 * 解空間基底を要求すると追加O((N-rank)N)時間・空間。逆元はpivotごとに1回。
 * @code
 * #include "library/math/gauss_elimination_mod.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using M=atcoder::modint998244353;ModLinearSystem<M> s(2);
 * s.add_equation({1,1},3);s.add_equation({1,-1},1);auto r=s.solve(true);
 * assert(r.solvable&&r.rank==2&&r.solution[0]==M(2)&&r.solution[1]==M(1));}
 * @endcode
 */
template <class Mint> class ModLinearSystem {
public:
    /** @brief 変数数を指定する。負数やINT_MAXはinvalid_argument。 */
    explicit ModLinearSystem(int variables) : n_(variables) {
        if (n_ < 0 || n_ == std::numeric_limits<int>::max() || !is_prime_u64(Mint::mod()))
            throw std::invalid_argument("ModLinearSystem: invalid dimension or non-prime modulus");
    }
    /** @brief 係数列と右辺を追加する。係数数不一致はinvalid_argument。O(N)。 */
    void add_equation(std::vector<Mint> coefficients, Mint rhs) {
        if (coefficients.size() != static_cast<std::size_t>(n_))
            throw std::invalid_argument("ModLinearSystem: coefficient count mismatch");
        coefficients.push_back(rhs);
        rows_.push_back(std::move(coefficients));
    }
    /** @brief 解の有無・階数・特解を求める。include_basis=trueなら斉次解基底も返す。 */
    ModLinearSolution<Mint> solve(bool include_basis = false) const {
        auto a = rows_;
        std::size_t row = 0;
        std::vector<int> pivot;
        for (int col = 0; col < n_ && row < a.size(); ++col) {
            auto k = row;
            while (k < a.size() && a[k][col] == Mint(0))
                ++k;
            if (k == a.size())
                continue;
            std::swap(a[k], a[row]);
            auto inverse = Mint(1) / a[row][col];
            for (int j = col; j <= n_; ++j)
                a[row][j] *= inverse;
            for (std::size_t i = 0; i < a.size(); ++i)
                if (i != row) {
                    auto factor = a[i][col];
                    if (factor != Mint(0))
                        for (int j = col; j <= n_; ++j)
                            a[i][j] -= factor * a[row][j];
                }
            pivot.push_back(col);
            ++row;
        }
        ModLinearSolution<Mint> result;
        result.rank = static_cast<int>(row);
        result.free_variables = n_ - result.rank;
        for (std::size_t i = row; i < a.size(); ++i)
            if (a[i][n_] != Mint(0)) {
                result.solvable = false;
                return result;
            }
        result.solution.resize(n_);
        for (std::size_t i = 0; i < row; ++i)
            result.solution[pivot[i]] = a[i][n_];
        if (include_basis)
            for (int j = 0, k = 0; j < n_; ++j) {
                if (k < result.rank && pivot[k] == j) {
                    ++k;
                    continue;
                }
                std::vector<Mint> basis(n_);
                basis[j] = 1;
                for (std::size_t i = 0; i < row; ++i)
                    basis[pivot[i]] = -a[i][j];
                result.nullspace.push_back(std::move(basis));
            }
        return result;
    }

private:
    int n_;
    std::vector<std::vector<Mint>> rows_;
};

/** @brief 素数法行列の階数。長方形を許す。O(MN min(M,N))体演算、O(MN)空間。空行列は0。 */
template <class Mint> int matrix_rank_mod(const std::vector<std::vector<Mint>> &a) {
    if (a.empty())
        return 0;
    if (a[0].size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("matrix_rank_mod: too many columns");
    ModLinearSystem<Mint> system(static_cast<int>(a[0].size()));
    for (const auto &row : a)
        system.add_equation(row, Mint(0));
    return system.solve().rank;
}

/** @brief 素数法の正方行列の行列式。0x0は1。非正方・非素数法はinvalid_argument。
 * @par Complexity O(N^3)体演算と高々N回の逆元、O(N^2)コピー空間。
 */
template <class Mint> Mint determinant_mod(std::vector<std::vector<Mint>> a) {
    if (!is_prime_u64(Mint::mod()))
        throw std::invalid_argument("determinant_mod: non-prime modulus");
    const std::size_t n = a.size();
    for (const auto &row : a)
        if (row.size() != n)
            throw std::invalid_argument("determinant_mod: not square");
    Mint result = 1;
    for (std::size_t col = 0; col < n; ++col) {
        auto k = col;
        while (k < n && a[k][col] == Mint(0))
            ++k;
        if (k == n)
            return Mint(0);
        if (k != col) {
            std::swap(a[k], a[col]);
            result = -result;
        }
        result *= a[col][col];
        auto inverse = Mint(1) / a[col][col];
        for (std::size_t i = col + 1; i < n; ++i) {
            auto factor = a[i][col] * inverse;
            for (std::size_t j = col + 1; j < n; ++j)
                a[i][j] -= factor * a[col][j];
        }
    }
    return result;
}
