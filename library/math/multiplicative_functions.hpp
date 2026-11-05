#pragma once
#include "linear_sieve.hpp"
#include "pollard_rho.hpp"
#include <cassert>
#include <cstddef>
#include <vector>

/**
 * @brief LinearSieveを再利用し、[0,n]のEuler phiとMobius muを一括計算する。
 * @pre n>=0。phi[0]=mu[0]=0、phi[1]=mu[1]=1（存在する場合）。
 * @par Complexity O(n) 時間・保持/作業空間。phi[x]とmu[x]の参照は O(1)。
 * @code
 * #include "library/math/multiplicative_functions.hpp"
 * #include <cassert>
 * int main(){MultiplicativeFunctions f(10);assert(f.phi[10]==4&&f.mu[6]==1&&f.mu[4]==0);}
 * @endcode
 */
struct MultiplicativeFunctions {
    std::vector<int> phi, mu;
    explicit MultiplicativeFunctions(int n) {
        assert(n >= 0);
        LinearSieve sieve(n);
        phi.resize(static_cast<std::size_t>(n) + 1);
        mu.resize(phi.size());
        if (n >= 1)
            phi[1] = mu[1] = 1;
        for (std::size_t x = 2; x < phi.size(); ++x) {
            int p = sieve.lpf[x], y = static_cast<int>(x / p);
            if (y % p == 0) {
                phi[x] = phi[y] * p;
                mu[x] = 0;
            } else {
                phi[x] = phi[y] * (p - 1);
                mu[x] = -mu[y];
            }
        }
    }
};
/// @brief 単一整数のEuler phi。n>=1（0はinvalid_argument）、phi(1)=1。
/// @par Complexity Pollard Rhoの分解時間 + O(log n) 時間、O(log n) 空間。積はn以下。
inline std::uint64_t euler_phi_u64(std::uint64_t n) {
    auto result = n;
    for (auto [p, exponent] : factorize_u64_counts(n)) {
        (void)exponent;
        result = result / p * (p - 1);
    }
    return result;
}
/// @brief 単一整数のMobius mu（-1,0,+1）。n>=1、mu(1)=1。例外・計算量はeuler_phi_u64と同じ。
inline int mobius_u64(std::uint64_t n) {
    int result = 1;
    for (auto [p, exponent] : factorize_u64_counts(n)) {
        (void)p;
        if (exponent > 1)
            return 0;
        result = -result;
    }
    return result;
}
