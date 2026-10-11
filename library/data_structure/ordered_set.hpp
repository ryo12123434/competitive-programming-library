#pragma once

#include <cassert>
#include <functional>
#include <limits>
#include <utility>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

/**
 * @brief GNU PBDS を用いた順序統計集合。
 *
 * insert / erase / find に加え、order_of_key と find_by_order を利用できる。
 *
 * @tparam T 要素の型
 * @note GNU++ 環境を前提とする。
 * @par Complexity
 * 各主要操作 O(log N)。
 *
 * @pre T の < が狭義弱順序を満たすこと。重複値は一つだけ保持する。
 * @note order_of_key(x) は x 未満の要素数。find_by_order(k) は 0-indexed の k 番目のイテレータで、範囲外なら end()。
 * @note 空集合の構築 O(1)、保持空間 O(N)。
 *
 * @code
 * #include "library/data_structure/ordered_set.hpp"
 * #include <cassert>
 * int main() {
 *     ordered_set<int> values;
 *     values.insert(7);
 *     assert(values.order_of_key(7) == 0);
 *     assert(*values.find_by_order(0) == 7);
 * }
 * @endcode
 */
template<class T>
using ordered_set = __gnu_pbds::tree<
    T,
    __gnu_pbds::null_type,
    std::less<T>,
    __gnu_pbds::rb_tree_tag,
    __gnu_pbds::tree_order_statistics_node_update
>;

/**
 * @brief 重複要素を許す順序統計集合。
 *
 * 内部で (値, 一意な ID) を保持して重複を区別する。
 *
 * @tparam T 要素の型
 * @note GNU++ 環境を前提とする。
 * @pre 要素数は int に、clear 間の挿入回数は long long に収まること。
 *
 * @pre T の < は狭義弱順序で、== / != と整合すること。
 * @note find_by_order は ordered_set と異なり、イテレータではなく値を返す。
 * @par Complexity
 * 空集合の構築 O(1)、保持空間 O(N)、主要操作 O(log N)。
 *
 * @code
 * #include "library/data_structure/ordered_set.hpp"
 * #include <cassert>
 * int main() {
 *     OrderedMultiset<int> values;
 *     values.insert(7);
 *     values.insert(7);
 *     assert(values.count(7) == 2);
 *     assert(values.find_by_order(1) == 7);
 * }
 * @endcode
 */
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
    /**
     * @brief x を 1 個追加する。
     * @param x 追加する値
     * @par Complexity
     * O(log N)
     */
    void insert(const T& x) {
        assert(next_id_ < std::numeric_limits<Id>::max());
        tree_.insert({x, next_id_++});
    }

    /**
     * @brief x を 1 個だけ削除する。
     * @param x 削除する値
     * @return 削除した場合 true、存在しなかった場合 false
     * @par Complexity
     * O(log N)
     */
    bool erase_one(const T& x) {
        auto it = tree_.lower_bound({x, std::numeric_limits<Id>::min()});
        if (it == tree_.end() || it->first != x) return false;
        tree_.erase(it);
        return true;
    }

    /**
     * @brief x 未満の要素数を返す。
     * @param x 境界値
     * @return x 未満の要素数
     * @par Complexity
     * O(log N)
     */
    int order_of_key(const T& x) const {
        return static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::min()})
        );
    }

    /**
     * @brief 0-indexed で k 番目に小さい値を返す。
     * @param k 順位
     * @return k 番目に小さい値
     * @pre 0 <= k < size()
     * @par Complexity
     * O(log N)
     */
    T find_by_order(int k) const {
        assert(0 <= k && k < size());
        return tree_.find_by_order(k)->first;
    }

    /**
     * @brief x の個数を返す。
     * @param x 数える値
     * @return x の個数
     * @par Complexity
     * O(log N)
     */
    int count(const T& x) const {
        const int l = static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::min()})
        );
        const int r = static_cast<int>(
            tree_.order_of_key({x, std::numeric_limits<Id>::max()})
        );
        return r - l;
    }

    /**
     * @brief 重複を含めた要素数を返す。
     * @return 現在の要素数。
     * @par Complexity
     * O(1)。
     */
    int size() const {
        return static_cast<int>(tree_.size());
    }

    /**
     * @brief 空集合か判定する。
     * @return 要素がなければ true。
     * @par Complexity
     * O(1)。
     */
    bool empty() const {
        return tree_.empty();
    }

    /**
     * @brief 全要素と内部の挿入 ID を消去する。
     * @par Complexity
     * O(N)。
     */
    void clear() {
        tree_.clear();
        next_id_ = 0;
    }
};
