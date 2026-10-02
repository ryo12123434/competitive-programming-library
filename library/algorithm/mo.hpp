#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

/**
 * @brief 静的配列の区間クエリをオフラインで処理する Mo's algorithm。
 * @note 配列と集計状態は呼び出し側が管理する。添字は 0-indexed、区間は [l, r)。
 * @note 左端のブロック順に並べ、右端はブロックごとに昇順・降順を交互にする。
 */
class Mo {
public:
    /// @brief 長さ n の配列用に構築する。O(1)。@pre n >= 0。
    explicit Mo(int n) : n_(n) { assert(n >= 0); }

    /**
     * @brief [l, r) を登録し、0-indexed の登録順 ID を返す。償却 O(1)。
     * @pre 0 <= l <= r <= n。クエリ数は int に収まること。
     * @note 空区間・重複クエリも登録できる。
     */
    int add_query(int l, int r) {
        assert(0 <= l && l <= r && r <= n_);
        assert(queries_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(queries_.size());
        queries_.push_back({l, r});
        return id;
    }

    /**
     * @brief 左右別の追加・削除関数で全クエリを処理する。
     * @param add_left 左端に追加する要素の添字を受け取る関数。
     * @param add_right 右端に追加する要素の添字を受け取る関数。
     * @param erase_left 左端から削除する要素の添字を受け取る関数。
     * @param erase_right 右端から削除する要素の添字を受け取る関数。
     * @param output 区間の集計が完成したときにクエリ ID を受け取る関数。
     * @pre 呼び出しごとに集計状態を空区間 [0, 0) に初期化すること。
     * @pre コールバック中にこの Mo のクエリを変更しないこと。
     * @note output の呼び出し順は登録順とは限らない。状態は最後の区間のまま残る。
     * @note クエリがなければコールバックは呼ばない。再実行時は状態を初期化すること。
     * @par 計算量 Q 個のクエリに対し O(Q log Q) の整列と O(N sqrt(Q) + Q) 回の区間操作。
     * コールバックの計算量は別途加算する。追加空間 O(Q)。
     */
    template <class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Output>
    void run(AddLeft add_left, AddRight add_right,
             EraseLeft erase_left, EraseRight erase_right, Output output) const {
        int q = static_cast<int>(queries_.size());
        if (q == 0) return;
        int block_size = std::max(1, static_cast<int>(n_ / std::sqrt(static_cast<double>(q))));
        std::vector<int> order(q);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            int block_a = queries_[a].l / block_size;
            int block_b = queries_[b].l / block_size;
            if (block_a != block_b) return block_a < block_b;
            return block_a % 2 ? queries_[a].r > queries_[b].r : queries_[a].r < queries_[b].r;
        });
        int l = 0, r = 0;
        for (int id : order) {
            const auto& query = queries_[id];
            while (l > query.l) add_left(--l);
            while (r < query.r) add_right(r++);
            while (l < query.l) erase_left(l++);
            while (r > query.r) erase_right(--r);
            output(id);
        }
    }

    /**
     * @brief 左右共通の add(index), erase(index) と output(query_id) で処理する。
     * @pre 集計状態は空区間に初期化済みで、要素の追加・削除が左右によらないこと。
     * @note 計算量・実行後の状態は左右別の run と同じ。
     */
    template <class Add, class Erase, class Output>
    void run(Add add, Erase erase, Output output) const {
        run(add, add, erase, erase, output);
    }

private:
    struct Query { int l, r; };
    int n_;
    std::vector<Query> queries_;
};
