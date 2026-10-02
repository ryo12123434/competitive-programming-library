#pragma once

#include <bit>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 無向木の最小共通祖先を二分累乗で求める。
 * @note 頂点は 0-indexed。構築は反復 DFS で行う。
 * @par 計算量 構築 O(N log N) 時間・空間、祖先・LCA・距離のクエリ O(log N)。
 */
class LCA {
public:
    /**
     * @brief tree を root を根とする木として前計算する。
     * @pre tree は N > 0 の連結な無向木で、各辺を両方向に格納すること。
     * @pre N は int に収まり、頂点と root は [0, N)。
     */
    explicit LCA(const std::vector<std::vector<int>>& tree, int root = 0) {
        assert(!tree.empty() && tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int n = static_cast<int>(tree.size());
        assert(0 <= root && root < n);
        int levels = static_cast<int>(std::bit_width(static_cast<unsigned int>(n)));
        parent_.assign(levels, std::vector<int>(n, -1));
        depth_.assign(n, -1);
        depth_[root] = 0;
        std::vector<int> stack{root};
        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            for (int to : tree[v]) {
                assert(0 <= to && to < n);
                if (to == parent_[0][v]) continue;
                assert(depth_[to] == -1);
                depth_[to] = depth_[v] + 1;
                parent_[0][to] = v;
                stack.push_back(to);
            }
        }
        for (int d : depth_) assert(d != -1);
        for (int k = 1; k < levels; ++k) {
            for (int v = 0; v < n; ++v) {
                int mid = parent_[k - 1][v];
                if (mid != -1) parent_[k][v] = parent_[k - 1][mid];
            }
        }
    }

    /// @brief v の深さを O(1) で返す。根の深さは 0。@pre v は [0, N)。
    int depth(int v) const {
        assert(0 <= v && v < static_cast<int>(depth_.size()));
        return depth_[v];
    }

    /// @brief v の親を O(1) で返す。根の親は -1。@pre v は [0, N)。
    int parent(int v) const {
        assert(0 <= v && v < static_cast<int>(depth_.size()));
        return parent_[0][v];
    }

    /// @brief v の k 個上の祖先を返す。根より上なら -1。@pre v は [0, N)。
    int kth_ancestor(int v, unsigned long long k) const {
        assert(0 <= v && v < static_cast<int>(depth_.size()));
        if (k > static_cast<unsigned long long>(depth_[v])) return -1;
        for (int bit = 0; k != 0; ++bit, k >>= 1) {
            if (k & 1ULL) v = parent_[bit][v];
        }
        return v;
    }

    /// @brief u, v の最小共通祖先を返す。@pre u, v は [0, N)。
    int query(int u, int v) const {
        assert(0 <= u && u < static_cast<int>(depth_.size()));
        assert(0 <= v && v < static_cast<int>(depth_.size()));
        if (depth_[u] < depth_[v]) std::swap(u, v);
        u = kth_ancestor(u, depth_[u] - depth_[v]);
        if (u == v) return u;
        for (int k = static_cast<int>(parent_.size()) - 1; k >= 0; --k) {
            if (parent_[k][u] != parent_[k][v]) {
                u = parent_[k][u];
                v = parent_[k][v];
            }
        }
        return parent_[0][u];
    }

    /// @brief u と v の距離（辺数）を返す。@pre u, v は [0, N)。
    int distance(int u, int v) const {
        int w = query(u, v);
        return (depth_[u] - depth_[w]) + (depth_[v] - depth_[w]);
    }

private:
    std::vector<std::vector<int>> parent_;
    std::vector<int> depth_;
};
