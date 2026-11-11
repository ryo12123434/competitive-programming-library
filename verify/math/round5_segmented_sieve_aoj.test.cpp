#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_1_C"
#include "../../library/math/segmented_sieve.hpp"
#include <iostream>
int main() {
    int n, count = 0;
    std::cin >> n;
    while (n--) {
        std::int64_t x;
        std::cin >> x;
        count += !segmented_sieve(x, x).empty();
    }
    std::cout << count << '\n';
}
