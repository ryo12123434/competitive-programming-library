#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * @brief Union-Find (Disjoint Set Union)。
 *
 * 経路圧縮と union by rank により素集合を管理する。
 *
 * @par Complexity
 * root / issame / unite / getsize は償却 O(alpha(N))。
 */
struct UnionFind {
    vector<int> parent, rank, size;

    /**
     * @brief n 頂点をそれぞれ独立した集合として初期化する。
     * @param n 頂点数
     * @par Complexity
     * O(N)
     */
    UnionFind(int n) : parent(n,-1), rank(n,0), size(n,1) { }

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
     * @brief x と y が属する集合を併合する。
     * @param x 頂点番号
     * @param y 頂点番号
     * @return 新たに併合した場合 true、既に同一集合なら false
     * @par Complexity
     * 償却 O(alpha(N))
     */
    bool unite(int x, int y) {
        int rx = root(x), ry = root(y);
        if(rx == ry) return false; 
        if(rank[rx]<rank[ry]) swap(rx, ry);
        parent[ry] = rx;
        if(rank[rx] == rank[ry]) rank[rx]++;
        size[rx] += size[ry];
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
};
