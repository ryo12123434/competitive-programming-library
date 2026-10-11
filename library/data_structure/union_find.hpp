#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * @brief 素集合を管理する Union-Find。
 *
 * 経路圧縮と union by rank により素集合を管理する。
 *
 * @par Complexity
 * root / issame / unite / getsize / getedges は償却 O(alpha(N))。
 * @pre 頂点番号は [0, N)。辺数は int に収まること。
 *
 * @note parent / rank / size / edge_count は内部管理用。直接変更しないこと。保持空間 O(N)。
 *
 * @code
 * #include "library/data_structure/union_find.hpp"
 * #include <cassert>
 * int main() {
 *     UnionFind uf(3);
 *     assert(uf.unite(0, 1));
 *     assert(uf.issame(0, 1));
 *     assert(uf.getsize(0) == 2);
 * }
 * @endcode
 */
struct UnionFind {
    vector<int> parent, rank, size, edge_count;

    /**
     * @brief n 頂点をそれぞれ独立した集合として初期化する。
     * @param n 頂点数
     * @par Complexity
     * O(N)
     *
     * @pre n >= 0。
     */
    UnionFind(int n) : parent(n,-1), rank(n,0), size(n,1), edge_count(n,0) { }

    /**
     * @brief x が属する集合の代表元を返す。
     * @param x 頂点番号
     * @return x の代表元
     * @par Complexity
     * 償却 O(alpha(N))
     */
    int root(int x) {
        if(parent[x] == -1) return x;
        return parent[x] = root(parent[x]);
    }

    /**
     * @brief x と y が同じ集合に属するか判定する。
     * @param x 頂点番号
     * @param y 頂点番号
     * @return 同じ集合なら true
     * @par Complexity
     * 償却 O(alpha(N))
     */
    bool issame(int x, int y) {
        return root(x) == root(y);
    }

    /**
     * @brief 辺 (x, y) を追加し、x と y が属する集合を併合する。
     * @note 既に同じ集合の場合も、自己ループ・多重辺を含め辺数を 1 増やす。
     * @param x 頂点番号
     * @param y 頂点番号
     * @return 新たに併合した場合 true、既に同一集合なら false
     * @par Complexity
     * 償却 O(alpha(N))
     */
    bool unite(int x, int y) {
        int rx = root(x), ry = root(y);
        if(rx == ry) {
            ++edge_count[rx];
            return false;
        }
        if(rank[rx]<rank[ry]) swap(rx, ry);
        parent[ry] = rx;
        if(rank[rx] == rank[ry]) rank[rx]++;
        size[rx] += size[ry];
        edge_count[rx] += edge_count[ry] + 1;
        return true;
    }

    /**
     * @brief x が属する集合の要素数を返す。
     * @param x 頂点番号
     * @return 集合の要素数
     * @par Complexity
     * 償却 O(alpha(N))
     */
    int getsize(int x) {
        return size[root(x)];
    }

    /**
     * @brief x が属する集合の辺数を返す。償却 O(alpha(N))。
     * @param x 0-indexed の頂点番号。
     * @return x の連結成分へ追加された辺数（自己ループ・多重辺も含む）。
     */
    int getedges(int x) {
        return edge_count[root(x)];
    }
};
