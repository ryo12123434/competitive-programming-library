#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ITP1_7_C"

#include <iostream>
#include <vector>
#include "../../library/data_structure/prefix_sum_2d.hpp"

int main() {
    int h, w;
    std::cin >> h >> w;
    std::vector<std::vector<long long>> values(h, std::vector<long long>(w));
    for (auto& row : values) for (auto& value : row) std::cin >> value;
    const PrefixSum2D<long long> sum(values);
    for (int y = 0; y <= h; ++y) {
        for (int x = 0; x <= w; ++x) {
            const int u = y == h ? 0 : y, d = y == h ? h : y + 1;
            const int l = x == w ? 0 : x, r = x == w ? w : x + 1;
            if (x != 0) std::cout << ' ';
            std::cout << sum.query(u, d, l, r);
        }
        std::cout << '\n';
    }
}
