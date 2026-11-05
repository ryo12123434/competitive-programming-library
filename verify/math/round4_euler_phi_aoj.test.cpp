#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/NTL_1_D"
#include "../../library/math/multiplicative_functions.hpp"
#include <iostream>

int main() {
    std::uint64_t n;
    std::cin >> n;
    std::cout << euler_phi_u64(n) << '\n';
}
