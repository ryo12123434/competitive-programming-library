#pragma once
#include <algorithm>
#include <cassert>
#include <tuple>
#include <utility>
#include <vector>

/**
 * @brief 反復探索で無向木を重心分解する。parentは重心木の親、levelは分解レベル（根0）。
 * @note ancestors[v]は(重心祖先,元の木での距離)を重心木の根からvまで並べた列。
 * 最近のマーク頂点には、更新でbest[c]=min(best[c],d)、取得でmin(best[c]+d)を使える。
 * @pre treeは両方向を格納した単純な連結無向木。空木は許す。頂点数はintに収まる。
 * @par Complexity 構築O(N log(N+1))時間・保持空間、作業空間O(N)。祖先数O(log(N+1))。
 * @code
 * #include "library/graph/centroid_decomposition.hpp"
 * #include <cassert>
 * int main(){CentroidDecomposition c({{1},{0,2},{1}});assert(c.parent[1]==-1&&c.ancestors[0].front()==std::make_pair(1,1));}
 * @endcode
 */
struct CentroidDecomposition {
    std::vector<int> parent, level;
    std::vector<std::vector<std::pair<int, int>>> ancestors;
    explicit CentroidDecomposition(const std::vector<std::vector<int>> &tree) {
        int n = static_cast<int>(tree.size());
        parent.assign(n, -1);
        level.resize(n);
        ancestors.resize(n);
        if (!n)
            return;
        std::vector<bool> removed(n);
        std::vector<int> from(n), size(n);
        std::vector<std::tuple<int, int, int>> tasks{{0, -1, 0}};
        while (!tasks.empty()) {
            auto [root, p, l] = tasks.back();
            tasks.pop_back();
            std::vector<int> order{root};
            from[root] = -1;
            for (std::size_t i = 0; i < order.size(); ++i) {
                int v = order[i];
                for (int to : tree[v]) {
                    assert(0 <= to && to < n);
                    if (!removed[to] && to != from[v]) {
                        from[to] = v;
                        order.push_back(to);
                    }
                }
            }
            for (auto it = order.rbegin(); it != order.rend(); ++it) {
                int v = *it;
                size[v] = 1;
                for (int to : tree[v])
                    if (!removed[to] && from[to] == v)
                        size[v] += size[to];
            }
            int centroid = -1, count = static_cast<int>(order.size());
            for (int v : order) {
                int largest = count - size[v];
                for (int to : tree[v])
                    if (!removed[to] && from[to] == v)
                        largest = std::max(largest, size[to]);
                if (largest <= count / 2) {
                    centroid = v;
                    break;
                }
            }
            assert(centroid >= 0);
            parent[centroid] = p;
            level[centroid] = l;
            std::vector<std::tuple<int, int, int>> stack{{centroid, -1, 0}};
            while (!stack.empty()) {
                auto [v, prev, d] = stack.back();
                stack.pop_back();
                ancestors[v].emplace_back(centroid, d);
                for (int to : tree[v])
                    if (!removed[to] && to != prev)
                        stack.emplace_back(to, v, d + 1);
            }
            removed[centroid] = true;
            for (int to : tree[centroid])
                if (!removed[to])
                    tasks.emplace_back(to, centroid, l + 1);
        }
    }
};
