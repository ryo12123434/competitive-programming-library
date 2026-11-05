#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

/**
 * @brief 密な二次元配列の点加算と半開矩形和を管理する Fenwick Tree。
 * @tparam T T{} が零元の可換加法群。+= と + / - を持ち、全中間演算がTに収まること。
 * @note 座標は (y,x)、矩形の引数順は PrefixSum2D と同じ (上端,下端,左端,右端)。
 * @par Complexity
 * 初期化・配列からの構築 O(HW) 時間・空間。add / sum は O(log(H+1) log(W+1))。
 * @code
 * #include "library/data_structure/fenwick_tree_2d.hpp"
 * #include <cassert>
 * int main(){FenwickTree2D<long long> bit(3,4);bit.add(1,2,5);assert(bit.sum(0,2,1,4)==5);}
 * @endcode
 */
template <class T> class FenwickTree2D {
public:
    /// @brief HxWの零配列を作る。@pre height,width>=0、H*Wがsize_tと確保可能なサイズに収まること。
    FenwickTree2D(int height, int width) : h_(height), w_(width) {
        assert(height >= 0 && width >= 0);
        assert(!width ||
               static_cast<std::size_t>(height) <= data_.max_size() / static_cast<std::size_t>(width));
        data_.resize(static_cast<std::size_t>(height) * width);
    }
    /// @brief 全行の幅が等しい配列からO(HW)で構築。空配列は0x0、空行はHx0。
    explicit FenwickTree2D(const std::vector<std::vector<T>> &a)
        : FenwickTree2D(dimension(a.size()), dimension(a.empty() ? 0 : a[0].size())) {
        for (int y = 0; y < h_; ++y) {
            assert(a[y].size() == static_cast<std::size_t>(w_));
            for (int x = 0; x < w_; ++x)
                at(y, x) = a[y][x];
        }
        for (int y = 0; y < h_; ++y) {
            int p = y + 1, step = p & -p;
            if (step <= h_ - p)
                for (int x = 0; x < w_; ++x)
                    at(p + step - 1, x) += at(y, x);
        }
        for (int x = 0; x < w_; ++x) {
            int p = x + 1, step = p & -p;
            if (step <= w_ - p)
                for (int y = 0; y < h_; ++y)
                    at(y, p + step - 1) += at(y, x);
        }
    }
    /// @brief 行数。O(1)。
    int height() const { return h_; }
    /// @brief 列数。O(1)。
    int width() const { return w_; }
    /// @brief (y,x)にdeltaを加算。@pre 0<=y<H、0<=x<W。
    void add(int y, int x, const T &delta) {
        assert(0 <= y && y < h_ && 0 <= x && x < w_);
        for (int i = y + 1;;) {
            for (int j = x + 1;;) {
                at(i - 1, j - 1) += delta;
                int step = j & -j;
                if (step > w_ - j)
                    break;
                j += step;
            }
            int step = i & -i;
            if (step > h_ - i)
                break;
            i += step;
        }
    }
    /// @brief [0,y) x [0,x) の和。@pre 0<=y<=H、0<=x<=W。辺長0ならT{}。
    T sum(int y, int x) const {
        assert(0 <= y && y <= h_ && 0 <= x && x <= w_);
        T result{};
        for (int i = y; i > 0; i -= i & -i)
            for (int j = x; j > 0; j -= j & -j)
                result += data_[static_cast<std::size_t>(i - 1) * w_ + j - 1];
        return result;
    }
    /// @brief [u,d) x [l,r)の和。@pre 0<=u<=d<=H、0<=l<=r<=W。空矩形ならT{}。
    T sum(int u, int d, int l, int r) const {
        assert(0 <= u && u <= d && d <= h_ && 0 <= l && l <= r && r <= w_);
        if (u == d || l == r)
            return T{};
        return sum(d, r) - sum(u, r) - sum(d, l) + sum(u, l);
    }

private:
    int h_, w_;
    std::vector<T> data_;
    static int dimension(std::size_t n) {
        assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        return static_cast<int>(n);
    }
    T &at(int y, int x) { return data_[static_cast<std::size_t>(y) * w_ + x]; }
};
