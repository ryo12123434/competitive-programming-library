#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_11_C"

// 重み 1 の辺を judge 問題で検証する。重み 0・混在・経路復元は回帰テストで検証する。
#include <iostream>
#include "../../library/graph/zero_one_bfs.hpp"

int main() {
    int n;
    std::cin >> n;
    ZeroOneBFS graph(n);
    for (int i = 0; i < n; ++i) {
        int u, degree;
        std::cin >> u >> degree;
        while (degree--) {
            int v;
            std::cin >> v;
            graph.add_edge(u - 1, v - 1, 1);
        }
    }
    graph.solve(0);
    for (int v = 0; v < n; ++v) {
        int d = graph.get_dist(v);
        std::cout << v + 1 << ' ' << (d == ZeroOneBFS::INF ? -1 : d) << '\n';
    }
}
