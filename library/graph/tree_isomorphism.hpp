#pragma once
#include <algorithm>
#include <cassert>
#include <map>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief ソートした子の型列を共有辞書でinternする、乱数・ハッシュ衝突のない木同型判定器。
 * @note IDは同じインスタンス内で比較する。他インスタンスや挿入順が異なる辞書のIDとは比較しない。
 * 空木のIDは0。根付き/根なし・中心1個/2個は異なるタグで管理する。ラベル・重みは扱わない。
 * @pre 入力は両方向に辺を格納した単純な連結無向木（空も可）。頂点数と生成ID数はintに収まる。
 * @par Complexity N頂点、辞書の型数Mに対しO(N log(N+1)+N log(M+N+1))時間。
 * 1回の作業空間O(N)、辞書保持はこれまで登録した木の頂点数合計に比例する空間。
 * @code
 * #include "library/graph/tree_isomorphism.hpp"
 * #include <cassert>
 * int main(){TreeIsomorphism ids;std::vector<std::vector<int>>a{{1},{0,2},{1}},b{{2},{2},{0,1}};assert(ids.unrooted_id(a)==ids.unrooted_id(b));assert(ids.rooted_id(a,1)==ids.rooted_id(b,2));}
 * @endcode
 */
class TreeIsomorphism {
    std::map<std::pair<int, std::vector<int>>, int> ids_;
    int intern(int tag, std::vector<int> children) {
        std::sort(children.begin(), children.end());
        int fresh = static_cast<int>(ids_.size()) + 1;
        return ids_.emplace(std::make_pair(tag, std::move(children)), fresh).first->second;
    }
    int encode(const std::vector<std::vector<int>> &tree, int root, int blocked) {
        int n = static_cast<int>(tree.size());
        std::vector<int> parent(n, -1), order{root}, type(n);
        parent[root] = blocked;
        for (std::size_t i = 0; i < order.size(); ++i) {
            int v = order[i];
            for (int to : tree[v]) {
                assert(0 <= to && to < n);
                if (to != parent[v]) {
                    parent[to] = v;
                    order.push_back(to);
                }
            }
        }
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            int v = *it;
            std::vector<int> children;
            for (int to : tree[v])
                if (to != parent[v])
                    children.push_back(type[to]);
            type[v] = intern(0, std::move(children));
        }
        return type[root];
    }

public:
    /// @brief rootを根にした型IDを返す。@pre 空木以外はroot∈[0,N)。
    int rooted_id(const std::vector<std::vector<int>> &tree, int root = 0) {
        if (tree.empty())
            return 0;
        assert(0 <= root && root < static_cast<int>(tree.size()));
        return encode(tree, root, -1);
    }
    /// @brief 葉の刈り込みで中心を求め、根なし木の型IDを返す。空木は0。
    int unrooted_id(const std::vector<std::vector<int>> &tree) {
        int n = static_cast<int>(tree.size());
        if (!n)
            return 0;
        std::vector<int> degree(n);
        std::queue<int> queue;
        for (int v = 0; v < n; ++v) {
            degree[v] = static_cast<int>(tree[v].size());
            if (degree[v] <= 1)
                queue.push(v);
        }
        int remaining = n;
        while (remaining > 2) {
            int count = static_cast<int>(queue.size());
            assert(count > 0);
            remaining -= count;
            while (count--) {
                int v = queue.front();
                queue.pop();
                for (int to : tree[v])
                    if (--degree[to] == 1)
                        queue.push(to);
            }
        }
        int a = queue.front();
        queue.pop();
        if (queue.empty())
            return intern(1, {encode(tree, a, -1)});
        int b = queue.front();
        return intern(2, {encode(tree, a, b), encode(tree, b, a)});
    }
};
