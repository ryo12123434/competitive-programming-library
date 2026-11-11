#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief オフライン登録座標だけを持つ疎な2次元Fenwick Tree。座標順は(y,x)。
 * @tparam T T{}が零元の可換加法群。全中間値が型の範囲に収まること。
 * @tparam Coord 大小比較可能な座標型（既定long long）。座標値そのものの加減算はしない。
 * @note 密なFenwickTree2DのO(HW)領域と異なり、P個の登録点にO(P log(P+1))領域を使う。
 * クエリ境界は未登録でもよい。addは事前登録点のみ。重複登録は1点にまとめる。
 * @par Complexity 構築O(P log^2(P+1))時間、保持O(P log(P+1))。更新・矩形和O(log^2(P+1))。
 * @code
 * #include "library/data_structure/sparse_fenwick_tree_2d.hpp"
 * #include <cassert>
 * int main(){SparseFenwickTree2D<long long>b({{100,-3},{1000000,7}});b.add(100,-3,5);assert(b.sum(0,101,-5,0)==5);}
 * @endcode
 */
template <class T, class Coord = long long> class SparseFenwickTree2D {
    std::vector<std::pair<Coord, Coord>> points_;
    std::vector<Coord> ys_;
    std::vector<std::vector<Coord>> xs_;
    std::vector<std::vector<T>> bit_;

public:
    /// @brief 全ての更新候補点を登録して零初期化。空も可。点数・圧縮座標数はintに収まること。
    explicit SparseFenwickTree2D(std::vector<std::pair<Coord, Coord>> points) : points_(std::move(points)) {
        std::sort(points_.begin(), points_.end());
        points_.erase(std::unique(points_.begin(), points_.end()), points_.end());
        for (auto &p : points_)
            ys_.push_back(p.first);
        ys_.erase(std::unique(ys_.begin(), ys_.end()), ys_.end());
        xs_.resize(ys_.size() + 1);
        bit_.resize(xs_.size());
        for (auto [y, x] : points_) {
            std::size_t i = std::lower_bound(ys_.begin(), ys_.end(), y) - ys_.begin() + 1;
            for (; i < xs_.size(); i += i & (~i + 1))
                xs_[i].push_back(x);
        }
        for (std::size_t i = 1; i < xs_.size(); ++i) {
            auto &v = xs_[i];
            std::sort(v.begin(), v.end());
            v.erase(std::unique(v.begin(), v.end()), v.end());
            bit_[i].resize(v.size() + 1);
        }
    }
    /// @brief 登録した(y,x)へdeltaを加算。未登録点はinvalid_argument（NDEBUGでも検査する）。
    void add(Coord y, Coord x, const T &delta) {
        if (!std::binary_search(points_.begin(), points_.end(), std::pair<Coord, Coord>{y, x}))
            throw std::invalid_argument("SparseFenwick2D: unregistered point");
        std::size_t i = std::lower_bound(ys_.begin(), ys_.end(), y) - ys_.begin() + 1;
        for (; i < xs_.size(); i += i & (~i + 1)) {
            std::size_t j = std::lower_bound(xs_[i].begin(), xs_[i].end(), x) - xs_[i].begin() + 1;
            for (; j < bit_[i].size(); j += j & (~j + 1))
                bit_[i][j] += delta;
        }
    }
    /// @brief y'<yかつx'<xの登録点の総和。任意の境界値を使える。
    T sum(Coord y, Coord x) const {
        T answer{};
        std::size_t i = std::lower_bound(ys_.begin(), ys_.end(), y) - ys_.begin();
        for (; i; i -= i & (~i + 1)) {
            std::size_t j = std::lower_bound(xs_[i].begin(), xs_[i].end(), x) - xs_[i].begin();
            for (; j; j -= j & (~j + 1))
                answer += bit_[i][j];
        }
        return answer;
    }
    /// @brief 半開矩形[y1,y2)×[x1,x2)の和。空矩形はT{}。
    T sum(Coord y1, Coord y2, Coord x1, Coord x2) const {
        assert(y1 <= y2 && x1 <= x2);
        if (y1 == y2 || x1 == x2)
            return T{};
        return sum(y2, x2) - sum(y1, x2) - sum(y2, x1) + sum(y1, x1);
    }
};
