#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

/**
 * @brief Kitamasa法でa[n]を求める。a[t]=sum_j coefficient[j]*a[t-1-j]という「遅れ順」の規約。
 * @tparam T 0,1,+,-,*を持つ可換環。整数型は中間値も範囲内に収める。modintを推奨する。
 * @param initial a[0..K)。coefficientと同じ長さ。K=0は零列として全nで0を返す。
 * @param coefficient K個の係数。Kと2Kはintに収まること。
 * @param n unsigned 64bit全域の項番号。n<Kなら初期値を返す。
 * @par Complexity O(K^2 log(n+1))時間、O(K)追加空間。
 * @code
 * #include "library/math/linear_recurrence.hpp"
 * #include <cassert>
 * int main(){assert(linear_recurrence(std::vector<long long>{0,1},{1,1},10)==55);}
 * @endcode
 */
template <class T>
T linear_recurrence(const std::vector<T> &initial, const std::vector<T> &coefficient, std::uint64_t n) {
    assert(initial.size() == coefficient.size());
    int k = static_cast<int>(initial.size());
    if (!k)
        return T(0);
    if (n < initial.size())
        return initial[static_cast<std::size_t>(n)];
    auto multiply = [&](const std::vector<T> &a, const std::vector<T> &b) {
        std::vector<T> product(2 * k - 1, T(0));
        for (int i = 0; i < k; ++i)
            for (int j = 0; j < k; ++j)
                product[i + j] += a[i] * b[j];
        for (int degree = 2 * k - 2; degree >= k; --degree)
            for (int j = 1; j <= k; ++j)
                product[degree - j] += product[degree] * coefficient[j - 1];
        product.resize(k);
        return product;
    };
    std::vector<T> answer(k, T(0)), power(k, T(0));
    answer[0] = T(1);
    if (k == 1)
        power[0] = coefficient[0];
    else
        power[1] = T(1);
    for (; n; n >>= 1) {
        if (n & 1)
            answer = multiply(answer, power);
        power = multiply(power, power);
    }
    T value = T(0);
    for (int i = 0; i < k; ++i)
        value += answer[i] * initial[i];
    return value;
}
