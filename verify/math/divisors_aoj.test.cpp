#define PROBLEM "http://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ITP1_3_D"

// Problem: AOJ ITP1_3_D - How Many Divisors?
// Target: library/math/divisors.hpp

#include <algorithm>
#include <iostream>

#include "../../library/math/divisors.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    long long a, b, c;
    std::cin >> a >> b >> c;

    const auto divisors = calc_divisors(c);
    const auto first = std::lower_bound(divisors.begin(), divisors.end(), a);
    const auto last = std::upper_bound(divisors.begin(), divisors.end(), b);

    std::cout << last - first << '\n';
}
