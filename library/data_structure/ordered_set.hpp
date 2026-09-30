#pragma once

#include <cassert>
#include <limits>
#include <utility>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

// GNU PBDS を用いた順序統計木。
// GNU++ 環境を前提とする。
//
// ordered_set<T>:
// - insert / erase / find: O(log N)
// - order_of_key(x): x 未満の要素数を O(log N) で取得
// - find_by_order(k): 0-indexed で k 番目の要素を O(log N) で取得
template<class T>
using ordered_set = __gnu_pbds::tree<
    T,
    __gnu_pbds::null_type,
    std::less<T>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update
>;

// 重複要素を許す順序統計木。
// 内部で (値, 一意なID) を保持して重複を区別する。
template<class T>
class OrderedMultiset {
private:
    using Id = long long;
    using Node = std::pair<T, Id>;
    using Tree = __gnu_pbds::tree<
        Node,
        __gnu_pbds::null_type,
        std::less<Node>,
        __gnu_pbds::rb_tree_tag,
        __gnu_pbds::tree_order_statistics_node_update
    >;

    Tree tree_;
    Id next_id_ = 0;

public:
    // x を1個追加する。
    // 計算量: O(log N)
    void insert(const T& x) {
        tree_.insert({x, next_id_++});
    }

    // x を1個だけ削除する。
    // x が存在しなければ何もしない。
    // 計算量: O(log N)
    bool erase_one(const T& x) {
        auto it = tree_.lower_bound({x, std::numeric_limits<Id>::min()});
        if (it == tree_.end() || it->first != x) return false;
        tree_.erase(it);
        return true;
    }

    // x 未満の要素数を返す。
    // 計算量: O(log N)
    int order_of_key(const T& x) const {
        return static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::min()})
        );
    }

    // 0-indexed で k 番目に小さい値を返す。
    // 前提: 0 <= k < size()
    // 計算量: O(log N)
    T find_by_order(int k) const {
        assert(0 <= k && k < size());
        return tree_.find_by_order(k)->first;
    }

    // x の個数を返す。
    // 計算量: O(log N)
    int count(const T& x) const {
        const int l = static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::min()})
        );
        const int r = static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::max()})
        );
        return r - l;
    }

    int size() const {
        return static_cast<int>(tree_.size());
    }

    bool empty() const {
        return tree_.empty();
    }

    void clear() {
        tree_.clear();
        next_id_ = 0;
    }
};
