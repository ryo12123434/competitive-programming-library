#pragma once

#include <vector>
#include <algorithm>

template<class Abel>
struct WeightedUnionFind {
    std::vector<int> parent, rank, size;
    std::vector<Abel> diff_weight;

    // 初期化：親、高さ、頂点数、親ノードとのポテンシャル差
    WeightedUnionFind(int n)
        : parent(n, -1), rank(n, 0), size(n, 1), diff_weight(n, 0) {}

    // 根を求める
    int root(int x) {
        if (parent[x] == -1) return x;
        int r = root(parent[x]);
        diff_weight[x] += diff_weight[parent[x]];
        return parent[x] = r;
    }

    // x と y が同じグループかどうか
    bool issame(int x, int y) {
        return root(x) == root(y);
    }

    // potential(y) - potential(x) = w となるように併合
    bool unite(int x, int y, Abel w) {
        w += get_weight(x);
        w -= get_weight(y);

        int rx = root(x), ry = root(y);
        if (rx == ry) return false;

        if (rank[rx] < rank[ry]) {
            std::swap(rx, ry);
            w = -w;
        }

        if (rank[rx] == rank[ry]) rank[rx]++;
        parent[ry] = rx;
        diff_weight[ry] = w;
        size[rx] += size[ry];
        return true;
    }

    // 頂点数を取得
    int get_size(int x) {
        return size[root(x)];
    }

    // 根から x までのポテンシャルを取得
    Abel get_weight(int x) {
        root(x);
        return diff_weight[x];
    }

    // potential(y) - potential(x) を取得
    Abel diff(int x, int y) {
        return get_weight(y) - get_weight(x);
    }
};
