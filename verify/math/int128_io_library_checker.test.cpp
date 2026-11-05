#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb"

#include "../../library/math/int128_io.hpp"
#include "../../library/math/safe_arithmetic.hpp"
#include <iostream>

// 128bitの読み書きと検査付き加算を、通常の一括入出力でも使用する。
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int cases;
    if (!(std::cin >> cases)) return 1;
    while (cases--) {
        __int128_t a = 0, b = 0;
        if (!read_int128(std::cin, a) || !read_int128(std::cin, b)) return 1;
        auto sum = checked_add(a, b);
        if (!sum) return 1;
        print_int128(std::cout, *sum) << '\n';
    }
}
