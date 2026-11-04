#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

/**
 * @brief 区間 chmin / chmax / 加算と和・最小値・最大値を扱う Segment Tree Beats。
 * @note 添字は 0-indexed、区間は [l,r)。空区間の和は 0、min/max は nullopt。
 * 値は long long、和は Sum（符号付き128bit）。極値の差も128bitで計算し、番兵値は使わない。
 * @pre N <= INT_MAX。各更新後の要素は long long に収まること。
 * 遅延加算の累積は符号付き128bitに収めること。区間和は N と値の制約から128bitに収まる。
 * @par Complexity
 * 構築 O(N)、空間 O(N)（非空配列で約 320N バイト）。クエリ O(log(N+1))。
 * chmin/chmax/add の混在は標準的な Beats の償却 O(log^2(N+1)) / 更新。単発更新の最悪は O(N)。
 * @code
 * #include "library/data_structure/segment_tree_beats.hpp"
 * #include <cassert>
 * int main() {
 *     SegmentTreeBeats seg(std::vector<long long>{1, 5, -2});
 *     seg.range_chmin(0, 3, 3); seg.range_chmax(0, 3, 0); seg.range_add(1, 3, 2);
 *     assert(seg.range_sum(0, 3) == 8 && seg.point_get(1) == 5);
 * }
 * @endcode
 */
class SegmentTreeBeats {
public:
    using Sum = __int128_t;
    /// @brief a をコピーして構築する。空配列も可。
    explicit SegmentTreeBeats(const std::vector<long long>& a) {
        assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        n_ = static_cast<int>(a.size());
        tree_.resize(4 * std::max<std::size_t>(1, a.size()));
        if (n_) build(1, 0, n_, a);
    }
    /// @brief 要素数。O(1)。
    int size() const { return n_; }
    /// @brief [l,r) の各値を min(value,x) にする。@pre 0<=l<=r<=size()。
    void range_chmin(int l, int r, long long x) { update(l, r, x, 0); }
    /// @brief [l,r) の各値を max(value,x) にする。@pre 0<=l<=r<=size()。
    void range_chmax(int l, int r, long long x) { update(l, r, x, 1); }
    /// @brief [l,r) に x を加算する。@pre 0<=l<=r<=size()、更新後の値は long long に収まる。
    void range_add(int l, int r, long long x) { update(l, r, x, 2); }
    /// @brief [l,r) の和。空区間なら 0。@pre 0<=l<=r<=size()。
    Sum range_sum(int l, int r) { check_range(l, r); return l == r ? 0 : query(1, 0, n_, l, r).sum; }
    /// @brief [l,r) の最小値。空区間なら nullopt。@pre 0<=l<=r<=size()。
    std::optional<long long> range_min(int l, int r) {
        check_range(l, r); if (l == r) return std::nullopt; return query(1, 0, n_, l, r).lo;
    }
    /// @brief [l,r) の最大値。空区間なら nullopt。@pre 0<=l<=r<=size()。
    std::optional<long long> range_max(int l, int r) {
        check_range(l, r); if (l == r) return std::nullopt; return query(1, 0, n_, l, r).hi;
    }
    /// @brief i 番目の値。@pre 0<=i<size()。O(log(N+1))。
    long long point_get(int i) { assert(0 <= i && i < n_); return *range_min(i, i + 1); }

private:
    struct Node {
        Sum sum = 0, add = 0;
        long long hi = 0, hi2 = 0, lo = 0, lo2 = 0;
        int nhi = 0, nlo = 0;
    };
    int n_;
    std::vector<Node> tree_;
    static long long narrow(Sum x) {
        assert(std::numeric_limits<long long>::lowest() <= x && x <= std::numeric_limits<long long>::max());
        return static_cast<long long>(x);
    }
    static Node combine(const Node& a, const Node& b) {
        Node c;
        c.sum = a.sum + b.sum;
        c.hi = std::max(a.hi, b.hi); c.lo = std::min(a.lo, b.lo);
        c.nhi = (a.hi == c.hi ? a.nhi : 0) + (b.hi == c.hi ? b.nhi : 0);
        c.nlo = (a.lo == c.lo ? a.nlo : 0) + (b.lo == c.lo ? b.nlo : 0);
        // 一様区間では次点を自身と等しくする。次点の有無は hi != lo で判別する。
        c.hi2 = c.lo; c.lo2 = c.hi;
        if (a.hi < c.hi) c.hi2 = std::max(c.hi2, a.hi);
        else if (a.lo != a.hi) c.hi2 = std::max(c.hi2, a.hi2);
        if (b.hi < c.hi) c.hi2 = std::max(c.hi2, b.hi);
        else if (b.lo != b.hi) c.hi2 = std::max(c.hi2, b.hi2);
        if (a.lo > c.lo) c.lo2 = std::min(c.lo2, a.lo);
        else if (a.lo != a.hi) c.lo2 = std::min(c.lo2, a.lo2);
        if (b.lo > c.lo) c.lo2 = std::min(c.lo2, b.lo);
        else if (b.lo != b.hi) c.lo2 = std::min(c.lo2, b.lo2);
        return c;
    }
    void build(std::size_t p, int l, int r, const std::vector<long long>& a) {
        if (r - l == 1) {
            auto& t = tree_[p]; t.sum = t.hi = t.hi2 = t.lo = t.lo2 = a[l]; t.nhi = t.nlo = 1; return;
        }
        int m = l + (r - l) / 2;
        build(p * 2, l, m, a); build(p * 2 + 1, m, r, a); pull(p);
    }
    void pull(std::size_t p) { tree_[p] = combine(tree_[p * 2], tree_[p * 2 + 1]); }
    void add(std::size_t p, int len, Sum x) {
        auto& t = tree_[p]; t.sum += x * len; t.add += x;
        t.hi = narrow(Sum(t.hi) + x); t.hi2 = narrow(Sum(t.hi2) + x);
        t.lo = narrow(Sum(t.lo) + x); t.lo2 = narrow(Sum(t.lo2) + x);
    }
    void chmin(std::size_t p, long long x) {
        auto& t = tree_[p]; if (t.hi <= x) return;
        t.sum += (Sum(x) - t.hi) * t.nhi;
        if (t.lo == t.hi) t.lo = t.lo2 = t.hi2 = x;
        else if (t.lo2 == t.hi) t.lo2 = x;
        t.hi = x;
    }
    void chmax(std::size_t p, long long x) {
        auto& t = tree_[p]; if (t.lo >= x) return;
        t.sum += (Sum(x) - t.lo) * t.nlo;
        if (t.hi == t.lo) t.hi = t.hi2 = t.lo2 = x;
        else if (t.hi2 == t.lo) t.hi2 = x;
        t.lo = x;
    }
    void push(std::size_t p, int l, int r) {
        int m = l + (r - l) / 2;
        propagate(p * 2, m - l, tree_[p]); propagate(p * 2 + 1, r - m, tree_[p]); tree_[p].add = 0;
    }
    void propagate(std::size_t p, int len, const Node& parent) {
        auto& t = tree_[p];
        if (parent.lo == parent.hi) {
            t = {}; t.sum = Sum(parent.lo) * len; t.lo = t.hi = t.lo2 = t.hi2 = parent.lo;
            t.nlo = t.nhi = len; return;
        }
        // add 後・clamp 前の値が64bitを越えても、最終値だけを narrow する。
        Sum hi = Sum(t.hi) + parent.add, hi2 = Sum(t.hi2) + parent.add;
        Sum lo = Sum(t.lo) + parent.add, lo2 = Sum(t.lo2) + parent.add;
        t.sum += parent.add * len; t.add += parent.add;
        if (hi > parent.hi) {
            t.sum += (Sum(parent.hi) - hi) * t.nhi;
            if (lo == hi) lo = lo2 = hi2 = parent.hi;
            else if (lo2 == hi) lo2 = parent.hi;
            hi = parent.hi;
        }
        if (lo < parent.lo) {
            t.sum += (Sum(parent.lo) - lo) * t.nlo;
            if (hi == lo) hi = hi2 = lo2 = parent.lo;
            else if (hi2 == lo) hi2 = parent.lo;
            lo = parent.lo;
        }
        t.hi = narrow(hi); t.hi2 = narrow(hi2); t.lo = narrow(lo); t.lo2 = narrow(lo2);
        if (lo == hi) { t.nhi = t.nlo = len; t.add = 0; }
    }
    void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
    void update(int l, int r, long long x, int kind) {
        check_range(l, r); if (l != r) update(1, 0, n_, l, r, x, kind);
    }
    void update(std::size_t p, int l, int r, int ql, int qr, long long x, int kind) {
        auto& t = tree_[p];
        if (r <= ql || qr <= l || (kind == 0 && t.hi <= x) || (kind == 1 && t.lo >= x)) return;
        if (ql <= l && r <= qr) {
            if (kind == 2) { add(p, r - l, x); return; }
            if (kind == 0 && (t.lo == t.hi || t.hi2 < x)) { chmin(p, x); return; }
            if (kind == 1 && (t.lo == t.hi || x < t.lo2)) { chmax(p, x); return; }
        }
        push(p, l, r);
        int m = l + (r - l) / 2;
        update(p * 2, l, m, ql, qr, x, kind); update(p * 2 + 1, m, r, ql, qr, x, kind); pull(p);
    }
    Node query(std::size_t p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return tree_[p];
        push(p, l, r);
        int m = l + (r - l) / 2;
        if (qr <= m) return query(p * 2, l, m, ql, qr);
        if (m <= ql) return query(p * 2 + 1, m, r, ql, qr);
        return combine(query(p * 2, l, m, ql, qr), query(p * 2 + 1, m, r, ql, qr));
    }
};
