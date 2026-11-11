#pragma once

#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief 多項式の多点評価・補間で共有するProduct Tree。
 * @tparam Polynomial FormalPowerSeries<Mint>相当。係数体、積・剰余・和・ADL diffを提供する型。
 * @pre NTT可能な素数法modintなどの体。係数演算の法を構築後に変更しない。
 * @note FPSヘッダは利用側でincludeする。評価点の重複は評価では許すが、補間ではnullopt。
 * @par Complexity 点数N、入力次数D、乗算コストMとして構築O(M(N)log(N+1))、
 * 評価O(M(D+N)+M(N)log(N+1))、補間O(M(N)log(N+1))と逆元1回。
 * M(N)=O(N log N)なら通常の多点評価・補間はO(N log^2 N)。保持空間O(N log(N+1))。
 * @code
 * #include "library/math/formal_power_series.hpp"
 * #include "library/math/multipoint_evaluation.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using M=atcoder::modint998244353;using P=FormalPowerSeries<M>;
 * PolynomialProductTree<P> tree(std::vector<M>{0,1,2});auto y=tree.evaluate(P{1,2});
 * assert((y==std::vector<M>{1,3,5}));assert(tree.interpolate(y)==P({1,2,0}));}
 * @endcode
 */
template <class Polynomial> class PolynomialProductTree {
public:
    using Mint = typename Polynomial::value_type;
    /** @brief 評価点列から構築する。空列も許す。 @param xs 評価点。 */
    explicit PolynomialProductTree(const std::vector<Mint> &xs) : n_(xs.size()) {
        if (n_ > static_cast<std::size_t>(std::numeric_limits<int>::max()) / 2)
            throw std::length_error("PolynomialProductTree: too many points");
        while (base_ < n_)
            base_ *= 2;
        products_.resize(2 * base_);
        for (std::size_t i = 0; i < base_; ++i)
            products_[base_ + i] = i < n_ ? Polynomial{-xs[i], Mint(1)} : Polynomial{Mint(1)};
        for (std::size_t i = base_ - 1; i > 0; --i)
            products_[i] = products_[2 * i] * products_[2 * i + 1];
    }
    /** @brief 評価点数。O(1)。 */
    int size() const { return static_cast<int>(n_); }
    /** @brief 全ての(x-xs[i])の積。空列なら定数1。O(1)参照。 */
    const Polynomial &product() const { return products_[1]; }
    /** @brief 各xs[i]におけるfの値を入力順に返す。空多項式は0。 */
    std::vector<Mint> evaluate(const Polynomial &f) const {
        if (!n_)
            return {};
        std::vector<Polynomial> remainder(2 * base_);
        remainder[1] = f % products_[1];
        for (std::size_t i = 1; i < base_; ++i) {
            remainder[2 * i] = remainder[i] % products_[2 * i];
            remainder[2 * i + 1] = remainder[i] % products_[2 * i + 1];
            Polynomial{}.swap(remainder[i]);
        }
        std::vector<Mint> answer(n_);
        for (std::size_t i = 0; i < n_; ++i)
            if (!remainder[base_ + i].empty())
                answer[i] = remainder[base_ + i][0];
        return answer;
    }
    /**
     * @brief f(xs[i])=ys[i]を満たすN項の多項式。空入力なら空多項式。
     * @param ys 評価点と同じ長さ。異なる長さはinvalid_argument。
     * @return 一意な補間多項式。重複点は値が一致していてもnullopt。
     */
    std::optional<Polynomial> interpolate(const std::vector<Mint> &ys) const {
        if (ys.size() != n_)
            throw std::invalid_argument("PolynomialProductTree: value count mismatch");
        if (!n_)
            return Polynomial{};
        auto denominator = evaluate(diff(products_[1]));
        std::vector<Mint> prefix(n_ + 1, Mint(1));
        for (std::size_t i = 0; i < n_; ++i) {
            if (denominator[i] == Mint(0))
                return std::nullopt;
            prefix[i + 1] = prefix[i] * denominator[i];
        }
        auto inverse = Mint(1) / prefix[n_];
        std::vector<Polynomial> partial(2 * base_);
        for (std::size_t i = n_; i-- > 0;) {
            partial[base_ + i] = Polynomial{ys[i] * prefix[i] * inverse};
            inverse *= denominator[i];
        }
        for (std::size_t i = base_ - 1; i > 0; --i) {
            partial[i] = partial[2 * i] * products_[2 * i + 1] + partial[2 * i + 1] * products_[2 * i];
            Polynomial{}.swap(partial[2 * i]);
            Polynomial{}.swap(partial[2 * i + 1]);
        }
        partial[1].resize(n_);
        return std::move(partial[1]);
    }

private:
    std::size_t n_, base_ = 1;
    std::vector<Polynomial> products_;
};

/** @brief Product Treeを一度構築してfをxsの全点で評価する。規約・計算量はPolynomialProductTree参照。 */
template <class Polynomial>
std::vector<typename Polynomial::value_type>
multipoint_evaluation(const Polynomial &f, const std::vector<typename Polynomial::value_type> &xs) {
    return PolynomialProductTree<Polynomial>(xs).evaluate(f);
}
