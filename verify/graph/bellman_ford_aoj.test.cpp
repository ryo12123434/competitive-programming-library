#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_B"

#include <iostream>
#include "../../library/graph/bellman_ford.hpp"

int main() {
    int n, m, s;
    std::cin >> n >> m >> s;
    Edges edges(m);
    for (auto& e : edges) std::cin >> e.from >> e.to >> e.cost;
    std::vector<long long> dis;
    if (bellman_ford(edges, n, s, dis)) {
        std::cout << "NEGATIVE CYCLE\n";
        return 0;
    }
    for (long long d : dis) {
        if (d == BELLMAN_FORD_INF) std::cout << "INF\n";
        else std::cout << d << '\n';
    }
}
