#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief 区間作用・一点取得専用のDual Segment Tree。区間集約が不要ならACL lazy_segtreeより設定が少ない。
 * @tparam S 点の値型。Fは作用型。S mapping(F,S)、F composition(F,F)、F id()を指定する。
 * @pre composition(f,g)は「gの後にf」。結合律・作用の整合性・idの単位元則を満たすこと。
 * 可換性・区間モノイドは不要。全中間演算が各型の範囲に収まること。
 * @note applyの前に両端の祖先を伝播し、非可換な作用の時系列を保つ。get/setも遅延を伝播する。
 * @par Complexity 構築O(N)時間・空間。apply/get/setはO(log(N+1))時間・O(1)追加空間。
 * @code
 * #include "library/data_structure/dual_segment_tree.hpp"
 * #include <cassert>
 * struct F{long long a,b;};long long mapping(F f,long long x){return f.a*x+f.b;}
 * F composition(F f,F g){return {f.a*g.a,f.a*g.b+f.b};}F id(){return {1,0};}
 * int main(){DualSegmentTree<long long,F,mapping,composition,id>d(std::vector<long long>{1,2});d.apply(0,2,{2,1});d.apply(1,2,{3,4});assert(d.get(1)==19);}
 * @endcode
 */
template <class S, class F, S (*mapping)(F, S), F (*composition)(F, F), F (*id)()> class DualSegmentTree {
    int n_, height_ = 0;
    std::size_t size_ = 1;
    std::vector<S> values_;
    std::vector<F> lazy_;
    void all_apply(std::size_t p, F f) {
        if (p < size_)
            lazy_[p] = composition(f, lazy_[p]);
        else if (p - size_ < values_.size())
            values_[p - size_] = mapping(f, values_[p - size_]);
    }
    void push(std::size_t p) {
        all_apply(2 * p, lazy_[p]);
        all_apply(2 * p + 1, lazy_[p]);
        lazy_[p] = id();
    }
    void push_path(std::size_t p) {
        for (int h = height_; h; --h)
            push(p >> h);
    }

public:
    explicit DualSegmentTree(std::vector<S> values) : values_(std::move(values)) {
        if (values_.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("DualSegmentTree: too many points");
        n_ = static_cast<int>(values_.size());
        while (size_ < values_.size()) {
            size_ *= 2;
            ++height_;
        }
        lazy_.assign(size_, id());
    }
    int size() const { return n_; }
    /// @brief 半開区間[l,r)へ作用fを適用。空なら何もしない。@pre 0<=l<=r<=N。
    void apply(int l, int r, F f) {
        assert(0 <= l && l <= r && r <= n_);
        if (l == r)
            return;
        std::size_t left = size_ + l, right = size_ + r;
        push_path(left);
        push_path(right - 1);
        while (left < right) {
            if (left & 1)
                all_apply(left++, f);
            if (right & 1)
                all_apply(--right, f);
            left >>= 1;
            right >>= 1;
        }
    }
    /// @brief 点iの現在値。@pre i∈[0,N)。
    S get(int i) {
        assert(0 <= i && i < n_);
        push_path(size_ + i);
        return values_[i];
    }
    /// @brief 過去の作用を反映してから点iを上書き。@pre i∈[0,N)。
    void set(int i, const S &value) {
        assert(0 <= i && i < n_);
        push_path(size_ + i);
        values_[i] = value;
    }
};
