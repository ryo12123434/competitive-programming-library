#pragma once
#include <cassert>
#include <cstddef>
#include <queue>
#include <utility>
#include <vector>

/// @brief 補グラフBFSの距離・親。到達不能の距離は-1、根・到達不能の親も-1。
struct ComplementBFSResult {
    std::vector<int> distance, parent;
};
/**
 * @brief 無向グラフの補グラフを、隣接行列を作らず探索する。補グラフに自己ループはない。
 * @note 元の多重辺は隣接の重複として扱い、自己ループは無視。未訪問リストの残留走査は元の辺へ課金できる。
 * @par Complexity 構築・bfs・componentsはいずれもO(V+E)時間・空間。頂点は0-indexed、個数はint。
 * @code
 * #include "library/graph/complement_graph.hpp"
 * #include <cassert>
 * int main(){ComplementGraph g(3,{{0,1}});assert(g.bfs(0).distance[1]==2&&g.components().size()==1);}
 * @endcode
 */
class ComplementGraph {
    int n_;
    std::vector<std::vector<int>> graph_;
    std::pair<ComplementBFSResult, std::vector<std::vector<int>>> run(int source, bool all) const {
        ComplementBFSResult result;
        result.distance.assign(n_, -1);
        result.parent.assign(n_, -1);
        std::vector<std::vector<int>> groups;
        std::vector<int> next(static_cast<std::size_t>(n_) + 1), prev(next.size()), marked(n_, -1);
        int head = 0;
        for (int i = 0; i < n_; ++i) {
            next[i] = i + 1;
            prev[i] = i - 1;
        }
        next[n_] = n_;
        auto remove = [&](int v) {
            if (prev[v] < 0)
                head = next[v];
            else
                next[prev[v]] = next[v];
            prev[next[v]] = prev[v];
        };
        while (head < n_) {
            int root = all ? head : source;
            remove(root);
            result.distance[root] = 0;
            groups.emplace_back();
            std::queue<int> queue;
            queue.push(root);
            while (!queue.empty()) {
                int v = queue.front();
                queue.pop();
                groups.back().push_back(v);
                for (int to : graph_[v])
                    marked[to] = v;
                for (int to = head; to < n_;) {
                    int following = next[to];
                    if (marked[to] != v) {
                        remove(to);
                        result.distance[to] = result.distance[v] + 1;
                        result.parent[to] = v;
                        queue.push(to);
                    }
                    to = following;
                }
            }
            if (!all)
                break;
        }
        return {std::move(result), std::move(groups)};
    }

public:
    /// @brief n>=0、無向辺の両端は[0,n)。入力を変更しない。
    ComplementGraph(int n, const std::vector<std::pair<int, int>> &edges) : n_(n) {
        assert(n >= 0);
        graph_.resize(n);
        for (auto [u, v] : edges) {
            assert(0 <= u && u < n && 0 <= v && v < n);
            if (u != v) {
                graph_[u].push_back(v);
                graph_[v].push_back(u);
            }
        }
    }
    /// @brief sourceから補グラフ上のBFSを行う。@pre source∈[0,n)。
    ComplementBFSResult bfs(int source) const {
        assert(0 <= source && source < n_);
        return run(source, false).first;
    }
    /// @brief 補グラフの各連結成分の頂点列。空グラフなら空列。
    std::vector<std::vector<int>> components() const { return run(0, true).second; }
};
