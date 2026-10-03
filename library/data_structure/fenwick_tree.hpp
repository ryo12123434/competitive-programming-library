#pragma once

#include <cassert>
#include <vector>

/**
 * @brief Fenwick Tree (Binary Indexed Tree)。
 *
 * 点加算と累積和・区間和を管理する。
 *
 * @tparam T 管理する値の型
 * @pre T{} が加法単位元であり、T が += と減算をサポートすること。
 * @pre 集計途中も含め、加減算が T で表現できること。
 * @note 添字は 0-indexed、区間は半開区間 [l, r)。
 * @par Complexity
 * 初期化 O(N)、add / sum O(log N)、メモリ O(N)。
 */
template <class T>
class FenwickTree {
public:
    /**
     * @brief サイズ n の Fenwick Tree を構築する。
     * @param n 要素数
     * @pre n >= 0
     * @par Complexity
     * O(N)
     */
    explicit FenwickTree(int n) : n_(n) {
        assert(n >= 0);
        data_.resize(n);
    }

    /**
     * @brief p 番目の要素に x を加算する。
     * @param p 更新する添字
     * @param x 加算する値
     * @pre 0 <= p < size
     * @par Complexity
     * O(log N)
     */
    void add(int p, T x) {
        assert(0 <= p && p < n_);
        for (++p; p <= n_;) {
            data_[p - 1] += x;
            int step = p & -p;
            if (step > n_ - p) break;
            p += step;
        }
    }

    /**
     * @brief 区間 [0, r) の総和を返す。
     * @param r 区間の右端
     * @return [0, r) の総和
     * @pre 0 <= r <= size
     * @par Complexity
     * O(log N)
     */
    T sum(int r) const {
        assert(0 <= r && r <= n_);
        T result{};
        for (; r > 0; r -= r & -r) {
            result += data_[r - 1];
        }
        return result;
    }

    /**
     * @brief 区間 [l, r) の総和を返す。
     * @param l 区間の左端
     * @param r 区間の右端
     * @return [l, r) の総和
     * @pre 0 <= l <= r <= size
     * @par Complexity
     * O(log N)
     */
    T sum(int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        return sum(r) - sum(l);
    }

private:
    int n_;
    std::vector<T> data_;
};
