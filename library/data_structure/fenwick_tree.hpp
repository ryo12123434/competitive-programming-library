#pragma once

#include <cassert>
#include <vector>

// Fenwick Tree (Binary Indexed Tree)
// 概要: 点への加算と、累積和・区間和を管理する。
// 計算量: 初期化 O(N)、add / sum O(log N)、メモリ O(N)。
// 添字: 0-indexed。区間は半開区間 [l, r)。
// 前提条件: T{} が加法単位元であり、T が += と減算をサポートすること。
// 使用方法: FenwickTree<T> fw(n); とし、fw.add(p, x) で p 番目に x を加算する。
// 注意事項: 0 <= n、0 <= p < n、0 <= l <= r <= n を満たすこと。値は T の範囲内で計算すること。
template <class T>
class FenwickTree {
public:
    explicit FenwickTree(int n) : n_(n) {
        assert(n >= 0);
        data_.resize(n);
    }

    void add(int p, T x) {
        assert(0 <= p && p < n_);
        for (++p; p <= n_; p += p & -p) {
            data_[p - 1] += x;
        }
    }

    T sum(int r) const {
        assert(0 <= r && r <= n_);
        T result{};
        for (; r > 0; r -= r & -r) {
            result += data_[r - 1];
        }
        return result;
    }

    T sum(int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        return sum(r) - sum(l);
    }

private:
    int n_;
    std::vector<T> data_;
};
