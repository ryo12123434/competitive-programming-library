#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

/// @brief 無向Steiner Tree用の非負辺。辺IDは入力配列の添字。自己ループ・多重辺も許す。
struct SteinerTreeEdge {
    int u, v;
    long long cost;
};

/// @brief Steiner部分グラフの最小費用と元の辺ID。重複辺IDは含めない。
struct SteinerTreeResult {
    __int128_t cost = 0;
    std::vector<int> edge_ids;
};
namespace cpl::steiner_detail {
inline std::optional<SteinerTreeResult> solve(int n, const std::vector<SteinerTreeEdge> &edges,
                                              std::vector<int> terminals, bool restore) {
    assert(n >= 0);
    for ([[maybe_unused]] int v : terminals)
        assert(0 <= v && v < n);
    struct Edge {
        int to, id;
        long long cost;
    };
    std::vector<std::vector<Edge>> graph(n);
    for (int id = 0; id < static_cast<int>(edges.size()); ++id) {
        auto [u, v, w] = edges[id];
        assert(0 <= u && u < n && 0 <= v && v < n && w >= 0);
        graph[u].push_back({v, id, w});
        graph[v].push_back({u, id, w});
    }
    std::sort(terminals.begin(), terminals.end());
    terminals.erase(std::unique(terminals.begin(), terminals.end()), terminals.end());
    if (terminals.size() <= 1)
        return SteinerTreeResult{};
    if (terminals.size() >= std::numeric_limits<std::size_t>::digits)
        throw std::length_error("Steiner: too many terminals");
    std::size_t states = std::size_t(1) << terminals.size();
    using W = __int128_t;
    if (states > std::vector<W>().max_size() / static_cast<std::size_t>(n))
        throw std::length_error("Steiner: DP too large");
    const W inf = W(1) << 120;
    std::vector<W> dp(states * n, inf);
    std::vector<int> previous;
    std::vector<std::size_t> split;
    if (restore) {
        previous.assign(dp.size(), -1);
        split.resize(dp.size());
    }
    auto index = [&](std::size_t mask, int v) { return mask * n + v; };
    auto at = [&](std::size_t mask, int v) -> W & { return dp[index(mask, v)]; };
    for (std::size_t i = 0; i < terminals.size(); ++i)
        at(std::size_t(1) << i, terminals[i]) = 0;
    using Item = std::pair<W, int>;
    for (std::size_t mask = 1; mask < states; ++mask) {
        for (std::size_t sub = (mask - 1) & mask; sub; sub = (sub - 1) & mask) {
            auto other = mask ^ sub;
            if (sub > other)
                continue;
            for (int v = 0; v < n; ++v)
                if (at(sub, v) != inf && at(other, v) != inf) {
                    W candidate = at(sub, v) + at(other, v);
                    if (candidate < at(mask, v)) {
                        at(mask, v) = candidate;
                        if (restore) {
                            split[index(mask, v)] = sub;
                            previous[index(mask, v)] = -1;
                        }
                    }
                }
        }
        std::set<Item> heap;
        for (int v = 0; v < n; ++v)
            if (at(mask, v) != inf)
                heap.emplace(at(mask, v), v);
        while (!heap.empty()) {
            auto [d, v] = *heap.begin();
            heap.erase(heap.begin());
            for (auto [to, id, w] : graph[v])
                if (d + w < at(mask, to)) {
                    heap.erase({at(mask, to), to});
                    at(mask, to) = d + w;
                    heap.emplace(d + w, to);
                    if (restore) {
                        previous[index(mask, to)] = id;
                        split[index(mask, to)] = 0;
                    }
                }
        }
    }
    W answer = inf;
    int root = -1;
    for (int v = 0; v < n; ++v)
        if (at(states - 1, v) < answer) {
            answer = at(states - 1, v);
            root = v;
        }
    if (root < 0)
        return std::nullopt;
    SteinerTreeResult result;
    result.cost = answer;
    if (restore) {
        std::vector<bool> used(edges.size()), seen(dp.size());
        std::vector<std::pair<std::size_t, int>> stack{{states - 1, root}};
        while (!stack.empty()) {
            auto [mask, v] = stack.back();
            stack.pop_back();
            auto p = index(mask, v);
            if (seen[p])
                continue;
            seen[p] = true;
            if (split[p]) {
                stack.emplace_back(split[p], v);
                stack.emplace_back(mask ^ split[p], v);
            } else if (previous[p] >= 0) {
                int id = previous[p];
                if (!used[id]) {
                    used[id] = true;
                    result.edge_ids.push_back(id);
                }
                auto e = edges[id];
                stack.emplace_back(mask, e.u == v ? e.v : e.u);
            }
        }
    }
    return result;
}
} // namespace cpl::steiner_detail
/**
 * @brief 指定頂点を全て結ぶ無向Steiner Treeの最小費用。非連結で不可能ならnullopt。
 * @param n 頂点数（非負int）。頂点とterminalは0-indexed。
 * @param edges 非負long long重みの無向辺。内部距離・返り値はsigned 128bit。
 * @param terminals 重複可。空・1頂点は費用0。異なるterminal数Kは小さい場合に使う。
 * @throws std::length_error 2^K Vが配列として表せない場合。実際に確保可能なメモリも必要。
 * @par Complexity O(3^K V + 2^K(V+E)log(V+1))時間、O(2^K V+V+E)空間。
 * 距離付き集合でdecrease-keyを行い、多重辺でも候補数をV以下に保つ。
 * @note 各maskで部分集合結合後に多始点Dijkstraを行う。既存Dijkstraは単始点・long longなので内部で128bit版を使う。
 * @code
 * #include "library/graph/steiner_tree.hpp"
 * #include <cassert>
 * int main(){auto c=steiner_tree(4,{{0,3,1},{1,3,1},{2,3,1}},{0,1,2});assert(c&&*c==3);}
 * @endcode
 */

inline std::optional<__int128_t> steiner_tree(int n, const std::vector<SteinerTreeEdge> &edges,
                                              std::vector<int> terminals) {
    auto result = cpl::steiner_detail::solve(n, edges, std::move(terminals), false);
    if (!result)
        return std::nullopt;
    return result->cost;
}
/// @brief 元の辺IDも復元する版。引数・計算量はsteiner_treeと同じで、復元用にO(2^K V)領域を追加する。
/// @note 非負辺なので零重み閉路を含む可能性はあるが、連結な最小費用部分グラフを返す。
inline std::optional<SteinerTreeResult>
steiner_tree_with_edges(int n, const std::vector<SteinerTreeEdge> &edges, std::vector<int> terminals) {
    return cpl::steiner_detail::solve(n, edges, std::move(terminals), true);
}
