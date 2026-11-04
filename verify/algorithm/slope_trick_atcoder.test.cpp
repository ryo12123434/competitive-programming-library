#define PROBLEM "https://atcoder.jp/contests/arc070/tasks/arc070_c"
#include "../../library/algorithm/slope_trick.hpp"
#include <iostream>
int main() {
    int n;
    std::cin >> n;
    SlopeTrick f;
    long long previous_width = 0;
    for (int i = 0; i < n; ++i) {
        long long l, r;
        std::cin >> l >> r;
        long long width = r - l;
        if (i)
            f.slide_min(-SlopeTrick::Wide(width), previous_width);
        f.add_abs(l);
        previous_width = width;
    }
    std::cout << static_cast<long long>(f.minimum()) << '\n';
}
