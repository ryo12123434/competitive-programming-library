#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_5_B"

#include <iostream>
#include "../../library/data_structure/imos_2d.hpp"

int main() {
    int n;
    std::cin >> n;
    Imos2D<int> imos(1000, 1000);
    while (n--) {
        int x1, y1, x2, y2;
        std::cin >> x1 >> y1 >> x2 >> y2;
        imos.add(y1, y2, x1, x2);
    }
    imos.build();
    int answer = 0;
    for (int y = 0; y < 1000; ++y)
        for (int x = 0; x < 1000; ++x) answer = std::max(answer, imos.get(y, x));
    std::cout << answer << '\n';
}
