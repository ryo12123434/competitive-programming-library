#pragma once

#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 根付き木を各頂点 1 回の DFS 行きがけ順に一次元化し、部分木区間を求める。
 * @note 頂点・DFS 添字は 0-indexed。LCA 用の長さ 2N-1 の巡回列とは異なる。
 * @par Complexity
 * 構築 O(N) 時間・空間、各情報の取得 O(1)。再帰は使わない。
 * @code
 * #include "library/graph/euler_tour.hpp"
 * #include <cassert>
 * int main() {
 *     EulerTour tour(std::vector<std::vector<int>>{{1, 2}, {0, 3}, {0}, {1}}, 0);
 *     assert(tour.parent(0) == -1 && tour.depth(3) == 2);
 *     auto [l, r] = tour.subtree_range(1);
 *     assert(r - l == tour.subtree_size(1) && tour.vertex(l) == 1);
 *     assert((tour.order() == std::vector<int>{0, 1, 3, 2}));
 * }
 * @endcode
 */
class EulerTour {
public:
    /**
     * @brief 無向木を root を根として反復 DFS で前計算する。
     * @param tree 各辺を両方向に格納した隣接リスト。隣接頂点の格納順に訪問する。
     * @param root 根の頂点番号。既定値 0。
     * @pre 0 < N <= INT_MAX の連結な無向木。root と各頂点は [0, N)。
     * @par Complexity
     * O(N) 時間・空間。
     */
    explicit EulerTour(const std::vector<std::vector<int>>& tree, int root = 0) {
        assert(!tree.empty() && tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const int n = static_cast<int>(tree.size());
        assert(0 <= root && root < n);
        parent_.assign(n, -1);
        depth_.assign(n, -1);
        tin_.resize(n);
        subtree_size_.assign(n, 1);
        order_.reserve(n);
        depth_[root] = 0;
        std::vector<int> stack{root};
        while (!stack.empty()) {
            const int v = stack.back();
            stack.pop_back();
            tin_[v] = static_cast<int>(order_.size());
            order_.push_back(v);
            for (auto it = tree[v].rbegin(); it != tree[v].rend(); ++it) {
                const int to = *it;
                assert(0 <= to && to < n);
                if (to == parent_[v]) continue;
                assert(depth_[to] == -1);
                parent_[to] = v;
                depth_[to] = depth_[v] + 1;
                stack.push_back(to);
            }
        }
        assert(static_cast<int>(order_.size()) == n);
        for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
            const int v = *it;
            if (parent_[v] != -1) subtree_size_[parent_[v]] += subtree_size_[v];
        }
    }

    /// @brief 頂点数を O(1) で返す。
    int size() const { return static_cast<int>(order_.size()); }
    /// @brief v の親を O(1) で返す。根は -1。@param v 頂点番号。@pre v は [0, size())。
    int parent(int v) const { check_vertex(v); return parent_[v]; }
    /// @brief 根から v への辺数を O(1) で返す。@param v 頂点番号。@pre v は [0, size())。
    int depth(int v) const { check_vertex(v); return depth_[v]; }
    /// @brief v 自身を含む部分木の頂点数を O(1) で返す。@param v 頂点番号。@pre v は [0, size())。
    int subtree_size(int v) const { check_vertex(v); return subtree_size_[v]; }
    /// @brief v の DFS 添字（部分木の左端）を O(1) で返す。@param v 頂点番号。@pre v は [0, size())。
    int tin(int v) const { check_vertex(v); return tin_[v]; }
    /// @brief 部分木の右端（含まない）を O(1) で返す。@param v 頂点番号。@pre v は [0, size())。
    int tout(int v) const { check_vertex(v); return tin_[v] + subtree_size_[v]; }
    /**
     * @brief v の部分木に対応する半開区間を O(1) で返す。
     * @param v 頂点番号。@pre v は [0, size())。
     * @return [tin(v), tout(v))。order() のこの範囲に部分木の全頂点が並ぶ。
     */
    std::pair<int, int> subtree_range(int v) const { return {tin(v), tout(v)}; }
    /// @brief DFS 添字 index から頂点番号へ O(1) で変換する。@param index DFS 添字。@pre index は [0, size())。
    int vertex(int index) const { assert(0 <= index && index < size()); return order_[index]; }
    /// @brief N 頂点の DFS 訪問順序をコピーせず O(1) で参照する。
    const std::vector<int>& order() const { return order_; }

private:
    std::vector<int> parent_, depth_, tin_, subtree_size_, order_;
    void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
};
