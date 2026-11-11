#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

/**
 * @brief 必要なノードだけ確保する非永続の動的セグメント木。未設定点はe()。
 * @tparam S コピー可能なモノイド型。S op(S,S)は結合的、S e()は両側単位元。可換性不要。
 * @note 座標はsigned 64bitの[lower,upper)。両端が表現可能な範囲。upperそのものは点に含まない。
 * std::midpointで中点の差のオーバーフローを防ぐ。ノード再確保をまたぐ参照は保持しない。
 * @par Complexity U=upper-lowerとして構築O(1)、set/get/prodはO(log(U+1))、all_prodはO(1)。
 * Q回更新の空間O(Q log(U+1))、setはプール再確保を含め償却。演算・コピーO(1)を仮定する。
 * @code
 * #include "library/data_structure/dynamic_segment_tree.hpp"
 * #include <cassert>
 * long long sum(long long a,long long b){return a+b;}long long zero(){return 0;}
 * int main(){DynamicSegmentTree<long long,sum,zero>s(-1000000000000LL,1000000000000LL);s.set(-3,7);assert(s.prod(-5,0)==7&&s.get(10)==0);}
 * @endcode
 */
template <class S, S (*op)(S, S), S (*e)()> class DynamicSegmentTree {
    using C = std::int64_t;
    struct Node {
        S value;
        int left = -1, right = -1;
    };
    C lower_, upper_;
    int root_ = -1;
    std::vector<Node> pool_;
    S value(int p) const { return p < 0 ? e() : pool_[p].value; }
    int change(int p, C l, C r, C index, const S &x) {
        if (p < 0) {
            assert(pool_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
            p = static_cast<int>(pool_.size());
            pool_.push_back({e()});
        }
        if (l + 1 == r) {
            pool_[p].value = x;
            return p;
        }
        C m = std::midpoint(l, r);
        if (index < m) {
            int child = change(pool_[p].left, l, m, index, x);
            pool_[p].left = child;
        } else {
            int child = change(pool_[p].right, m, r, index, x);
            pool_[p].right = child;
        }
        pool_[p].value = op(value(pool_[p].left), value(pool_[p].right));
        return p;
    }
    S query(int p, C l, C r, C ql, C qr) const {
        if (p < 0 || qr <= l || r <= ql)
            return e();
        if (ql <= l && r <= qr)
            return pool_[p].value;
        C m = std::midpoint(l, r);
        return op(query(pool_[p].left, l, m, ql, qr), query(pool_[p].right, m, r, ql, qr));
    }

public:
    /// @brief 半開座標域で構築。@pre lower<=upper。空の域も許す。ノード数はintに収まること。
    DynamicSegmentTree(C lower, C upper) : lower_(lower), upper_(upper) { assert(lower <= upper); }
    /// @brief indexをxに上書きする。@pre lower<=index<upper。
    void set(C index, const S &x) {
        assert(lower_ <= index && index < upper_);
        root_ = change(root_, lower_, upper_, index, x);
    }
    /// @brief 半開区間[l,r)を座標の昇順に集約。空区間はe()。@pre lower<=l<=r<=upper。
    S prod(C l, C r) const {
        assert(lower_ <= l && l <= r && r <= upper_);
        return l == r ? e() : query(root_, lower_, upper_, l, r);
    }
    /// @brief 1点の値。未設定ならe()。@pre lower<=index<upper。O(log(U+1))。
    S get(C index) const {
        assert(lower_ <= index && index < upper_);
        return prod(index, index + 1);
    }
    /// @brief 全座標域の集約。空域・未設定ならe()。O(1)。
    S all_prod() const { return value(root_); }
    /// @brief 確保済みノード数。O(1)。queryでは増えない。
    std::size_t node_count() const { return pool_.size(); }
    /// @brief プールを事前確保する。O(既存ノード数)の移動が起きうる。
    void reserve_nodes(std::size_t count) { pool_.reserve(count); }
};
