#pragma once

#include "multipoint_evaluation.hpp"

/**
 * @brief 相異なるxs[i]とys[i]からN項の補間多項式を求める。
 * @tparam Polynomial FormalPowerSeries<Mint>相当の多項式型。明示指定する。
 * @param xs 評価点。重複は値によらずnullopt。
 * @param ys 対応する値。長さ不一致はinvalid_argument。
 * @pre 係数は体。ACL畳み込みの法・長さ制約を満たすこと。
 * @return 補間結果。空入力は空多項式。
 * @par Complexity O(N log^2(N+1))時間と体の逆元1回、O(N log(N+1))空間（NTT利用時）。
 * @code
 * #include "library/math/formal_power_series.hpp"
 * #include "library/math/polynomial_interpolation.hpp"
 * #include <atcoder/modint>
 * #include <cassert>
 * int main(){using P=FormalPowerSeries<atcoder::modint998244353>;
 * auto f=polynomial_interpolation<P>({0,1},{2,5});assert(f==P({2,3}));}
 * @endcode
 */
template <class Polynomial>
std::optional<Polynomial> polynomial_interpolation(const std::vector<typename Polynomial::value_type> &xs,
                                                   const std::vector<typename Polynomial::value_type> &ys) {
    return PolynomialProductTree<Polynomial>(xs).interpolate(ys);
}
