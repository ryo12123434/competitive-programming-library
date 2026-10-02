#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_4_B"

#include <iostream>
#include "../../library/graph/topological_sort.hpp"

int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<std::vector<int>> graph(n);
    std::vector<int> indegree(n, 0);
    while (m--) {
        int u, v;
        std::cin >> u >> v;
        graph[u].push_back(v);
        ++indegree[v];
    }
    for (int v : topological_sort(graph, indegree, n)) std::cout << v << '\n';
}
