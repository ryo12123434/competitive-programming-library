#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 非負重みの無向木の直径と、その頂点列を反復探索 2 回で求める。
 * @tparam T 距離の型。T{} が 0、加算と大小比較が可能な型（既定 long long）。
 * @pre 各辺重みは非負で、探索中の距離の和が T に収まること。負辺には対応しない。
 * @note 非重み付き隣接リストは全辺重み 1 として扱う。最遠点が複数ならその一つを返す。
 * @par Complexity
 * 構築 O(N) 時間・空間、結果取得 O(1)。get_path はコピーせず参照する。
 * @code
 * #include "library/graph/tree_diameter.hpp"
 * #include <cassert>
 * int main() {
 *     TreeDiameter diameter(std::vector<std::vector<int>>{{1}, {0, 2}, {1}});
 *     assert(diameter.length() == 2 && diameter.get_path().size() == 3);
 *     using Diameter = TreeDiameter<long long>;
 *     std::vector<std::vector<Diameter::Edge>> tree{{{1, 7}}, {{0, 7}}};
 *     Diameter weighted(tree);
 *     auto [s, t] = weighted.endpoints();
 *     assert(weighted.length() == 7 && s != t);
 * }
 * @endcode
 */
template <class T = long long>
class TreeDiameter {
public:
    /**
     * @brief 行き先と非負重み。無向辺は両方向に同じ cost で格納する。
     * @code
     * #include "library/graph/tree_diameter.hpp"
     * #include <cassert>
     * int main() {
     *     TreeDiameter<>::Edge edge{1, 5};
     *     assert(edge.to == 1 && edge.cost == 5);
     * }
     * @endcode
     */
    struct Edge { int to; T cost; };

    /**
     * @brief 非重み付き無向木の直径を構築する。
     * @param tree 各辺を両方向に格納した隣接リスト。
     * @pre 0 < N <= INT_MAX の連結な無向木。各頂点は [0, N)。T は 1 から構築可能。
     * @par Complexity
     * O(N) 時間・空間。
     */
    explicit TreeDiameter(const std::vector<std::vector<int>>& tree) : TreeDiameter(with_unit_cost(tree)) {}

    /**
     * @brief 非負重み付き無向木の直径を構築する。
     * @param tree 各辺を両方向に同じ重みで格納した隣接リスト。
     * @pre 0 < N <= INT_MAX の連結な無向木。各頂点は [0, N)、全辺 cost >= 0。
     * @pre すべての単純パスの重みの和が T に収まること。
     * @note N == 1 では長さ 0・端点 (0,0)・経路 {0}。全辺 0 でも単一頂点の経路を返すことがある。
     * @par Complexity
     * O(N) 時間・空間。
     */
    explicit TreeDiameter(const std::vector<std::vector<Edge>>& tree) {
        assert(!tree.empty() && tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        std::vector<int> parent;
        const int start = farthest(tree, 0, parent).first;
        const auto [finish, distance] = farthest(tree, start, parent);
        length_ = distance;
        for (int v = finish; v != -1; v = parent[v]) path_.push_back(v);
        std::reverse(path_.begin(), path_.end());
    }

    /// @brief 直径の長さを O(1) で返す。非重み付きなら辺数。
    T length() const { return length_; }
    /// @brief 直径の端点を経路の順に O(1) で返す。
    std::pair<int, int> endpoints() const { return {path_.front(), path_.back()}; }
    /// @brief 両端点を含む直径の頂点列をコピーせず O(1) で参照する。
    const std::vector<int>& get_path() const { return path_; }

private:
    T length_{};
    std::vector<int> path_;

    static std::vector<std::vector<Edge>> with_unit_cost(const std::vector<std::vector<int>>& tree) {
        std::vector<std::vector<Edge>> result(tree.size());
        for (std::size_t v = 0; v < tree.size(); ++v) {
            result[v].reserve(tree[v].size());
            for (int to : tree[v]) result[v].push_back({to, T{1}});
        }
        return result;
    }

    static std::pair<int, T> farthest(const std::vector<std::vector<Edge>>& tree, int start,
                                      std::vector<int>& parent) {
        const int n = static_cast<int>(tree.size());
        parent.assign(n, -2);
        std::vector<T> distance(n, T{});
        std::vector<int> stack{start};
        parent[start] = -1;
        int finish = start, visited = 0;
        while (!stack.empty()) {
            const int v = stack.back();
            stack.pop_back();
            ++visited;
            if (distance[finish] < distance[v]) finish = v;
            for (const auto& edge : tree[v]) {
                assert(0 <= edge.to && edge.to < n && !(edge.cost < T{}));
                if (edge.to == parent[v]) continue;
                assert(parent[edge.to] == -2);
                parent[edge.to] = v;
                distance[edge.to] = distance[v] + edge.cost;
                stack.push_back(edge.to);
            }
        }
        assert(visited == n);
        return {finish, distance[finish]};
    }
};
