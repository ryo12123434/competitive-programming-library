#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_5_D"

#include <iostream>
#include "../../library/algorithm/inversion_number.hpp"

int main() {
    int n;
    std::cin >> n;
    std::vector<long long> a(n);
    for (auto& x : a) std::cin >> x;
    std::cout << count_inversions(a) << '\n';
}
