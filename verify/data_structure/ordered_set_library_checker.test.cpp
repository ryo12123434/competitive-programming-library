#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"

// Problem: Library Checker - Ordered Set
// Target: library/data_structure/ordered_set.hpp

#include <iostream>

#include "../../library/data_structure/ordered_set.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    ordered_set<long long> st;
    for (int i = 0; i < n; ++i) {
        long long x;
        std::cin >> x;
        st.insert(x);
    }

    while (q--) {
        int type;
        long long x;
        std::cin >> type >> x;

        if (type == 0) {
            st.insert(x);
        } else if (type == 1) {
            st.erase(x);
        } else if (type == 2) {
            if (x <= 0 || static_cast<long long>(st.size()) < x) {
                std::cout << -1 << '\n';
            } else {
                std::cout << *st.find_by_order(static_cast<std::size_t>(x - 1)) << '\n';
            }
        } else if (type == 3) {
            std::cout << st.order_of_key(x + 1) << '\n';
        } else if (type == 4) {
            const auto k = st.order_of_key(x + 1);
            if (k == 0) {
                std::cout << -1 << '\n';
            } else {
                std::cout << *st.find_by_order(k - 1) << '\n';
            }
        } else {
            const auto k = st.order_of_key(x);
            if (k == st.size()) {
                std::cout << -1 << '\n';
            } else {
                std::cout << *st.find_by_order(k) << '\n';
            }
        }
    }
}
