#pragma once

#include <cassert>
#include <vector>

/**
 * @brief 頂点番号で管理する静的双方向連結リスト。
 *
 * 各頂点の前後の頂点番号を配列で保持する。
 *
 * @note 頂点自体の追加・削除ではなく、既存頂点間の連結を管理する。
 * @pre 頂点番号は [0, n)。connect では自己接続や循環を作らないこと。
 */
struct Static_Doubly_LinkedList {
    int n;
    std::vector<int> nxt; // 次の要素 (p)
    std::vector<int> prv; // 前の要素 (q)

    /**
     * @brief n 頂点を互いに未接続の状態で初期化する。
     * @param n 頂点数
     * @pre n >= 0
     * @par Complexity
     * O(N)
     */
    Static_Doubly_LinkedList(int n) : n(n) {
        assert(n >= 0);
        nxt.assign(n, -1);
        prv.assign(n, -1);
    }

    /**
     * @brief x の直後に y を連結する。
     * @param x 前側の頂点
     * @param y 後側の頂点
     * @par Complexity
     * O(1)
     */
    void connect(int x, int y) {
        assert(0 <= x && x < n && 0 <= y && y < n && x != y);
        if(nxt[x] != -1) prv[nxt[x]] = -1;
        if(prv[y] != -1) nxt[prv[y]] = -1;
        nxt[x] = y;
        prv[y] = x;
    }

    /**
     * @brief x と y が直接連結されていれば解除する。
     * @param x 前側の頂点
     * @param y 後側の頂点
     * @par Complexity
     * O(1)
     */
    void disconnect(int x, int y) {
        assert(0 <= x && x < n && 0 <= y && y < n);
        if(nxt[x] == y) nxt[x] = -1;
        if(prv[y] == x) prv[y] = -1;
    }
    
    /**
     * @brief z を現在の位置から取り外し、x の直後に挿入する。
     * @param x 基準となる頂点
     * @param z 挿入する頂点
     * @note x == z または z が既に直後にある場合は何もしない。
     * @par Complexity
     * O(1)
     */
    void insert_after(int x, int z) {
        assert(0 <= x && x < n && 0 <= z && z < n);
        if (x == z || nxt[x] == z) return;
        detach(z);
        int y = nxt[x];
        connect(x, z);
        if(y != -1) connect(z, y);
    }

    /**
     * @brief z を現在の位置から取り外し、x の直前に挿入する。
     * @param x 基準となる頂点
     * @param z 挿入する頂点
     * @note x == z または z が既に直前にある場合は何もしない。
     * @par Complexity
     * O(1)
     */
    void insert_before(int x, int z) {
        assert(0 <= x && x < n && 0 <= z && z < n);
        if (x == z || prv[x] == z) return;
        detach(z);
        int y = prv[x];
        connect(z, x);
        if(y != -1) connect(y, z);
    }

    /**
     * @brief x が含まれる連結成分を先頭から順に取得する。
     * @param x 連結成分内の頂点
     * @return 先頭から末尾までの頂点列
     * @par Complexity
     * 連結成分の要素数を K として O(K)。
     */
    std::vector<int> get_path(int x) {
        assert(0 <= x && x < n);
        // 先頭まで遡る
        int head = x;
        while (prv[head] != -1) {
            head = prv[head];
        }

        // 先頭から順に末尾まで辿る
        std::vector<int> path;
        int cur = head;
        while (cur != -1) {
            path.push_back(cur);
            cur = nxt[cur];
        }
        return path;
    }

private:
    void detach(int z) {
        int before = prv[z], after = nxt[z];
        if (before != -1) nxt[before] = after;
        if (after != -1) prv[after] = before;
        prv[z] = nxt[z] = -1;
    }
};
