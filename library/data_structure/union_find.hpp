#pragma once

#include <bits/stdc++.h>
using namespace std;

struct UnionFind {
    vector<int> parent, rank, size;

    //初期化　根付き木の親、高さ、頂点数
    UnionFind(int n) : parent(n,-1), rank(n,0), size(n,1) { }

    //根を求める
    int root(int x) {
        if(parent[x] == -1) return x;
        return parent[x] = root(parent[x]);
    }

    //xとyが同じグループかどうか
    bool issame(int x, int y) {
        return root(x) == root(y);
    }

    //xとyを含むグループを合体
    bool unite(int x, int y) {
        int rx = root(x), ry = root(y);
        if(rx == ry) return false; 
        if(rank[rx]<rank[ry]) swap(rx, ry);
        parent[ry] = rx;
        if(rank[rx] == rank[ry]) rank[rx]++;
        size[rx] += size[ry];
        return true;
    }

    // 頂点数を取得
    int getsize(int x) {
        return size[root(x)];
    }
};
