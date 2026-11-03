#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 無向木を Heavy-Light Decomposition で一次元化する。
 * @note 頂点は 0-indexed。反復探索のみを用い、LCA・EulerTour・セグ木に依存しない。
 * @note 辺の値は、根から見て深い方の頂点の tin に置く。根の位置には辺の値を置かない。
 * @par Complexity
 * 構築 O(N) 時間・空間。lca と path_segments は O(log N)、その他の取得は O(1)。
 * @code
 * #include "library/graph/hld.hpp"
 * #include <atcoder/segtree>
 * #include <cassert>
 * long long sum_op(long long a, long long b) { return a + b; }
 * long long sum_e() { return 0; }
 * int main() {
 *     std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
 *     HLD hld(tree, 0);
 *     atcoder::segtree<long long, sum_op, sum_e> seg(hld.size());
 *     for (int v = 0; v < hld.size(); ++v) seg.set(hld.tin(v), v + 1);
 *     long long answer = 0;
 *     for (auto part : hld.path_segments(3, 2)) answer += seg.prod(part.l, part.r);
 *     assert(answer == 10); // 可換な和では reversed を無視できる
 *     auto [l, r] = hld.subtree_range(1);
 *     assert(seg.prod(l, r) == 6 && hld.lca(3, 2) == 0);
 * }
 * @endcode
 */
class HLD {
public:
    /**
     * @brief 経路の一区間。[l,r) を reversed なら r-1,...,l、そうでなければ l,...,r-1 の順で通る。
     * @note 非可換演算では、セグ木の各ノードに正順・逆順の集約を保持し、reversed に応じて選ぶ。
     * 集約済みの値を単に反転する方法では一般の非可換演算を処理できない。
     * @code
     * #include "library/graph/hld.hpp"
     * #include <cassert>
     * int main() {
     *     HLD::PathSegment part{2, 5, true}; // 位置 4,3,2 を順に通る
     *     assert(part.r - part.l == 3 && part.reversed);
     * }
     * @endcode
     */
    struct PathSegment { int l, r; bool reversed; };

    /**
     * @brief root を根とし、最大部分木の子を heavy child として構築する。
     * @param tree 各辺を両方向に格納した無向木の隣接リスト。
     * @param root 根（既定値 0）。
     * @pre 0 < N <= INT_MAX の連結な無向木で、root と各隣接頂点は [0,N)。
     * @note heavy child の部分木サイズが同じなら隣接リストで先の子を選ぶ。
     * 一次元化は heavy child を先に訪問する DFS 順で、EulerTour の訪問順とは一致するとは限らない。
     * @par Complexity
     * O(N) 時間・空間。元の隣接リストを変更しない。
     */
    explicit HLD(const std::vector<std::vector<int>>& tree, int root = 0) {
        assert(!tree.empty() && tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const int n = static_cast<int>(tree.size());
        assert(0 <= root && root < n);
        parent_.assign(n, -1);
        depth_.assign(n, -1);
        subtree_size_.assign(n, 1);
        heavy_.assign(n, -1);
        head_.resize(n);
        tin_.resize(n);
        order_.reserve(n);
        std::vector<int> stack{root};
        depth_[root] = 0;
        while (!stack.empty()) {
            const int v = stack.back();
            stack.pop_back();
            order_.push_back(v);
            for (int to : tree[v]) {
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
            for (int to : tree[v]) {
                if (to == parent_[v]) continue;
                subtree_size_[v] += subtree_size_[to];
                if (heavy_[v] == -1 || subtree_size_[heavy_[v]] < subtree_size_[to]) heavy_[v] = to;
            }
        }
        order_.clear();
        stack.push_back(root);
        head_[root] = root;
        while (!stack.empty()) {
            const int v = stack.back();
            stack.pop_back();
            tin_[v] = static_cast<int>(order_.size());
            order_.push_back(v);
            for (auto it = tree[v].rbegin(); it != tree[v].rend(); ++it) {
                const int to = *it;
                if (to == parent_[v] || to == heavy_[v]) continue;
                head_[to] = to;
                stack.push_back(to);
            }
            if (heavy_[v] != -1) {
                head_[heavy_[v]] = head_[v];
                stack.push_back(heavy_[v]);
            }
        }
    }

    /// @brief 頂点数を O(1) で返す。
    int size() const { return static_cast<int>(order_.size()); }
    /// @brief v の親（根なら -1）を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int parent(int v) const { check_vertex(v); return parent_[v]; }
    /// @brief v の深さ（根は 0）を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int depth(int v) const { check_vertex(v); return depth_[v]; }
    /// @brief v 自身を含む部分木サイズを O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int subtree_size(int v) const { check_vertex(v); return subtree_size_[v]; }
    /// @brief heavy child（葉なら -1）を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int heavy_child(int v) const { check_vertex(v); return heavy_[v]; }
    /// @brief v が属する heavy path の先頭を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int head(int v) const { check_vertex(v); return head_[v]; }
    /// @brief v の一次元化位置を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int tin(int v) const { check_vertex(v); return tin_[v]; }
    /// @brief v の部分木の右端（含まない）を O(1) で返す。@param v 頂点番号。@pre v は [0,size())。
    int tout(int v) const { check_vertex(v); return tin_[v] + subtree_size_[v]; }
    /// @brief 一次元化位置から頂点番号へ O(1) で戻す。@param index 位置。@pre index は [0,size())。
    int vertex(int index) const { assert(0 <= index && index < size()); return order_[index]; }
    /// @brief heavy child 優先の DFS 順をコピーせず O(1) で参照する。
    const std::vector<int>& order() const { return order_; }

    /**
     * @brief 部分木を表す半開区間を O(1) で返す。
     * @param v 部分木の根。@pre v は [0,size())。
     * @param edge true なら部分木内の辺だけを扱い、v と親を結ぶ辺を除く。
     * @return 頂点なら [tin(v),tout(v))、辺なら [tin(v)+1,tout(v))。葉の辺区間は空。
     */
    std::pair<int, int> subtree_range(int v, bool edge = false) const {
        return {tin(v) + static_cast<int>(edge), tout(v)};
    }

    /**
     * @brief 指定した根に対する u と v の最小共通祖先を返す。
     * @param u 一方の頂点。@param v 他方の頂点。@pre u,v は [0,size())。
     * @par Complexity
     * O(log N) 時間、追加空間 O(1)。
     */
    int lca(int u, int v) const {
        check_vertex(u); check_vertex(v);
        while (head_[u] != head_[v]) {
            if (depth_[head_[u]] < depth_[head_[v]]) std::swap(u, v);
            u = parent_[head_[u]];
        }
        return depth_[u] < depth_[v] ? u : v;
    }

    /**
     * @brief u から v へ進む順に、空でない区間とその向きを返す。
     * @param u 出発頂点。@param v 到着頂点。@pre u,v は [0,size())。
     * @param edge false は両端点と LCA を含む頂点列、true は LCA を除いた辺の格納位置列。
     * @return u→v 順の区間列。各区間内は reversed に従って読む。u==v の辺クエリは空列。
     * @note 非可換演算ではこの列の順に正順・逆順の区間集約を結合する。
     * @par Complexity
     * O(log N) 時間・追加空間、区間数 O(log N)。N==1 は O(1)。
     */
    std::vector<PathSegment> path_segments(int u, int v, bool edge = false) const {
        check_vertex(u); check_vertex(v);
        std::vector<PathSegment> left, right;
        while (head_[u] != head_[v]) {
            if (depth_[head_[u]] >= depth_[head_[v]]) {
                left.push_back({tin_[head_[u]], tin_[u] + 1, true});
                u = parent_[head_[u]];
            } else {
                right.push_back({tin_[head_[v]], tin_[v] + 1, false});
                v = parent_[head_[v]];
            }
        }
        const int l = std::min(tin_[u], tin_[v]) + static_cast<int>(edge);
        const int r = std::max(tin_[u], tin_[v]) + 1;
        if (l < r) left.push_back({l, r, depth_[u] >= depth_[v]});
        left.insert(left.end(), right.rbegin(), right.rend());
        return left;
    }

private:
    std::vector<int> parent_, depth_, subtree_size_, heavy_, head_, tin_, order_;
    void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
};
