#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 経路コピーで過去の配列を保持する、モノイドの永続セグメント木。
 * @tparam S コピー可能な集約型。S op(S,S) は結合的、S e() は両側単位元。可換性は不要。
 * @tparam op 左区間・右区間の集約をこの順に結合する関数。
 * @tparam e 空区間の値を返す関数。
 * @note 初期バージョンは 0。set は元を変えず新しい ID を返す。ID はこの個体内だけで有効。
 * @pre 要素数・バージョン数・ノード数は int に収まること。演算結果は S の表現範囲に収める。
 * @par Complexity
 * S のコピー・演算が O(1) なら構築 O(N)、set/prod/get O(log(N+1))、all_prod O(1)。
 * ノードプール O(N+U log(N+1))、バージョンの根 O(U)。U は更新回数。set はプール拡張を含め償却計算量。
 * @code
 * #include "library/data_structure/persistent_segment_tree.hpp"
 * #include <cassert>
 * long long sum(long long a, long long b) { return a+b; }
 * long long zero() { return 0; }
 * int main() {
 *     PersistentSegmentTree<long long, sum, zero> seg(std::vector<long long>{1,2});
 *     int v1 = seg.set(0, 1, 8), v2 = seg.set(0, 0, 3);
 *     assert(seg.all_prod(0)==3 && seg.prod(v1,0,2)==9 && seg.get(v2,0)==3);
 * }
 * @endcode
 */
template <class S, S (*op)(S, S), S (*e)()>
class PersistentSegmentTree {
public:
    /// @brief a からバージョン 0 を構築。空配列も可。
    explicit PersistentSegmentTree(const std::vector<S>& a) {
        assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        n_ = static_cast<int>(a.size());
        roots_.push_back(n_ ? build(0, n_, a) : -1);
    }
    /// @brief n 要素を e() で初期化。@pre n>=0。
    explicit PersistentSegmentTree(int n) : PersistentSegmentTree(initial(n)) {}
    /// @brief 要素数。O(1)。
    int size() const { return n_; }
    /// @brief 初期版も含むバージョン数。O(1)。
    int versions() const { return static_cast<int>(roots_.size()); }
    /// @brief version の index だけを value に変えた新しいバージョン ID を返す。
    /// @pre version は [0,versions())、index は [0,size())。
    int set(int version, int index, const S& value) {
        check_version(version); assert(0 <= index && index < n_);
        assert(roots_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const int root = change(roots_[version], 0, n_, index, value);
        roots_.push_back(root); return versions() - 1;
    }
    /// @brief 過去版の [l,r) を左から集約。空区間は e()。
    /// @pre version は [0,versions())、0<=l<=r<=size()。
    S prod(int version, int l, int r) const {
        check_version(version); assert(0 <= l && l <= r && r <= n_);
        return l == r ? e() : query(roots_[version], 0, n_, l, r);
    }
    /// @brief 過去版の index 番目の値。@pre 有効な version と 0<=index<size()。
    S get(int version, int index) const { assert(0 <= index && index < n_); return prod(version, index, index + 1); }
    /// @brief 過去版全体の集約。空配列は e()。@pre 有効な version。
    S all_prod(int version) const { check_version(version); return n_ ? pool_[roots_[version]].value : e(); }
private:
    struct Node { S value; int left, right; };
    int n_;
    std::vector<Node> pool_;
    std::vector<int> roots_;
    static std::vector<S> initial(int n) { assert(n >= 0); return std::vector<S>(n, e()); }
    void check_version([[maybe_unused]] int v) const { assert(0 <= v && v < versions()); }
    int append(Node node) {
        assert(pool_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(pool_.size()); pool_.push_back(std::move(node)); return id;
    }
    int build(int l, int r, const std::vector<S>& a) {
        if (r - l == 1) return append({a[l], -1, -1});
        int m = l + (r - l) / 2, left = build(l, m, a), right = build(m, r, a);
        return append({op(pool_[left].value, pool_[right].value), left, right});
    }
    int change(int p, int l, int r, int index, const S& value) {
        if (r - l == 1) return append({value, -1, -1});
        Node node = pool_[p]; // 再帰中の vector 再確保をまたいで参照を保持しない。
        int m = l + (r - l) / 2;
        if (index < m) node.left = change(node.left, l, m, index, value);
        else node.right = change(node.right, m, r, index, value);
        node.value = op(pool_[node.left].value, pool_[node.right].value);
        return append(std::move(node));
    }
    S query(int p, int l, int r, int ql, int qr) const {
        if (ql <= l && r <= qr) return pool_[p].value;
        int m = l + (r - l) / 2;
        if (qr <= m) return query(pool_[p].left, l, m, ql, qr);
        if (m <= ql) return query(pool_[p].right, m, r, ql, qr);
        return op(query(pool_[p].left, l, m, ql, qr), query(pool_[p].right, m, r, ql, qr));
    }
};
