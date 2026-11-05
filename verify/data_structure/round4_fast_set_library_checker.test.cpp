#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include "../../library/data_structure/fast_set.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::string s;
    std::cin >> n >> q >> s;
    FastSet set(s);
    while (q--) {
        int t, x;
        std::cin >> t >> x;
        if (t == 0)
            set.insert(x);
        if (t == 1)
            set.erase(x);
        if (t == 2)
            std::cout << set.contains(x) << "\n";
        if (t == 3)
            std::cout << set.next(x).value_or(-1) << "\n";
        if (t == 4)
            std::cout << set.prev(x).value_or(-1) << "\n";
    }
}
