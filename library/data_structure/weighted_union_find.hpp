#pragma once

#include <vector>
#include <algorithm>

/**
 * @brief ポテンシャル差を管理する Weighted Union-Find。
 *
 * @tparam Abel 加減算と単項マイナスをサポートする可換群の値型
 * @pre 頂点番号は [0, N)。中間値も含めポテンシャルの演算が Abel で表現できること。
 * @par Complexity
 * root / issame / unite / get_size / get_weight / diff は償却 O(alpha(N))。
 *
 * @note Abel は 0 から変換可能で、0 は加法単位元。+=、-=、単項 - の演算は可換加法群を成すこと。
 * @note 各頂点の絶対ポテンシャルは決定しない。保持空間 O(N)。
 *
 * @code
 * #include "library/data_structure/weighted_union_find.hpp"
 * #include <cassert>
 * int main() {
 *     WeightedUnionFind<long long> uf(3);
 *     uf.unite(0, 1, 4);
 *     uf.unite(1, 2, -1);
 *     assert(uf.diff(0, 2) == 3);
 * }
 * @endcode
 */
template<class Abel>
struct WeightedUnionFind {
    std::vector<int> parent, rank, size;
    std::vector<Abel> diff_weight;

    /**
     * @brief n 頂点をそれぞれ独立した集合として初期化する。
     * @param n 頂点数
     * @par Complexity
     * O(N)
     *
     * @pre n >= 0。
     */
    WeightedUnionFind(int n)
        : parent(n, -1), rank(n, 0), size(n, 1), diff_weight(n, 0) {}

    /**
     * @brief x が属する集合の代表元を返す。
     * @param x 頂点番号
     * @return x の代表元
     * @par Complexity
     * 償却 O(alpha(N))
     */
    int root(int x) {
        if (parent[x] == -1) return x;
        int r = root(parent[x]);
        diff_weight[x] += diff_weight[parent[x]];
        return parent[x] = r;
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
     * @brief potential(y) - potential(x) = w となるように集合を併合する。
     * @param x 頂点番号
     * @param y 頂点番号
     * @param w x から y へのポテンシャル差
     * @return 新たに併合した場合 true、既に同一集合なら false
     * @par Complexity
     * 償却 O(alpha(N))
     *
     * @note 同一集合なら w の整合性は検査せず false を返す。矛盾判定は issame(x,y) と diff(x,y) == w で行う。
     */
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

    /**
     * @brief x が属する集合の要素数を返す。
     * @param x 頂点番号
     * @return 集合の要素数
     * @par Complexity
     * 償却 O(alpha(N))
     */
    int get_size(int x) {
        return size[root(x)];
    }

    /**
     * @brief x の代表元を基準とした x のポテンシャルを返す。
     * @param x 頂点番号
     * @return x のポテンシャル
     * @par Complexity
     * 償却 O(alpha(N))
     */
    Abel get_weight(int x) {
        root(x);
        return diff_weight[x];
    }

    /**
     * @brief potential(y) - potential(x) を返す。
     * @param x 頂点番号
     * @param y 頂点番号
     * @return x から y へのポテンシャル差
     * @pre x と y が同じ集合に属すること。
     * @par Complexity
     * 償却 O(alpha(N))
     */
    Abel diff(int x, int y) {
        return get_weight(y) - get_weight(x);
    }
};
