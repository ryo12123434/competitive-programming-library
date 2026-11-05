#pragma once
#include "fenwick_tree.hpp"
#include <cassert>

/**
 * @brief 2本の既存FenwickTreeで区間加算・区間和を扱う。
 * @tparam T 可換加法群の型。T{}が零元、+=・減算・intとの乗算(T*int)を持つこと。
 * @pre 重み付き差分・prefixの積など中間演算も含めてTに収まること。必要なら128bitやmodintを指定する。
 * @note 0-indexed、半開区間。要素ごとの値ではなく差分と添字倍の差分を保持する。
 * @par Complexity 初期化 O(N) 時間・空間、add/sum O(log(N+1))。
 * @code
 * #include "library/data_structure/range_add_fenwick_tree.hpp"
 * #include <cassert>
 * int main(){RangeAddFenwickTree<long long> bit(4);bit.add(1,4,3);bit.add(2,3,-1);assert(bit.sum(0,4)==8&&bit.sum(2,3)==2);}
 * @endcode
 */
template <class T> class RangeAddFenwickTree {
public:
    /// @brief n要素を0で初期化。@pre n>=0。
    explicit RangeAddFenwickTree(int n) : n_(n), difference_(n), weighted_(n) {}
    /// @brief 要素数。O(1)。
    int size() const { return n_; }
    /// @brief [l,r)にvalueを加算。空区間は何もしない。@pre 0<=l<=r<=N。
    void add(int l, int r, T value) {
        assert(0 <= l && l <= r && r <= n_);
        if (l == r)
            return;
        difference_.add(l, value);
        weighted_.add(l, value * l);
        if (r < n_) {
            difference_.add(r, T{} - value);
            weighted_.add(r, (T{} - value) * r);
        }
    }
    /// @brief [0,r)の和。@pre 0<=r<=N。
    T sum(int r) const {
        assert(0 <= r && r <= n_);
        return difference_.sum(r) * r - weighted_.sum(r);
    }
    /// @brief [l,r)の和。@pre 0<=l<=r<=N。空区間はT{}。
    T sum(int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        return sum(r) - sum(l);
    }

private:
    int n_;
    FenwickTree<T> difference_, weighted_;
};
