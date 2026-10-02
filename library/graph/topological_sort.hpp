#pragma once

#include <cassert>
#include <queue>
#include <vector>

/**
 * @brief Kahn 法でトポロジカル順序を求める。indegree は更新される。
 * @pre G.size() == indegree.size() == V、入次数が正しく、頂点番号は [0, V)。
 * @note 返された頂点数が V 未満なら、有向閉路が存在する。
 * @par 計算量 O(V + E) 時間、O(V) 追加空間。
 */
inline std::vector<int> topological_sort(const std::vector<std::vector<int>>& G,
                                         std::vector<int>& indegree, int V) {
    assert(V >= 0 && static_cast<int>(G.size()) == V && static_cast<int>(indegree.size()) == V);
    std::queue<int> que;
    for (int v = 0; v < V; ++v) if (indegree[v] == 0) que.push(v);
    std::vector<int> order;
    while (!que.empty()) {
        int v = que.front();
        que.pop();
        order.push_back(v);
        for (int u : G[v]) if (--indegree[u] == 0) que.push(u);
    }
    return order;
}
