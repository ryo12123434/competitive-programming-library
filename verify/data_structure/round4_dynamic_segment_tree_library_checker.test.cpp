#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include "../../library/data_structure/dynamic_segment_tree.hpp"
#include <iostream>
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    DynamicSegmentTree<long long, op, e> seg(0, n);
    for (int i = 0; i < n; ++i) {
        long long x;
        std::cin >> x;
        seg.set(i, x);
    }
    while (q--) {
        int type;
        long long x, y;
        std::cin >> type >> x >> y;
        if (type == 0)
            seg.set(x, seg.get(x) + y);
        else
            std::cout << seg.prod(x, y) << "\n";
    }
}
