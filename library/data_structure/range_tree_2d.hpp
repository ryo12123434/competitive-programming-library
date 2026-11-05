#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief 静的な二次元点集合の矩形内点数を、xソート＋yのMerge Sort Treeで数える。
 * @tparam Coordinate < で一貫して比較できる座標型。long longなど。NaNは不可。
 * @note 重複点は個別に数える。構築後の追加・更新は不可。座標に加減算を行わない。
 * @par Complexity N点なら構築 O(N log(N+1)) 時間・空間、count O(log^2(N+1))。
 * @code
 * #include "library/data_structure/range_tree_2d.hpp"
 * #include <cassert>
 * int main(){RangeTree2D<> tree({{0,1},{0,1},{3,-1}});assert(tree.count(-1,2,0,2)==2);}
 * @endcode
 */
template <class Coordinate = long long> class RangeTree2D {
public:
    /// @brief (x,y)の点集合をコピーして構築。@pre 点数はINT_MAX以下。空集合も可。
    explicit RangeTree2D(std::vector<std::pair<Coordinate, Coordinate>> points) {
        assert(points.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        std::sort(points.begin(), points.end());
        while (base_ < points.size())
            base_ *= 2;
        tree_.resize(base_ * 2);
        xs_.reserve(points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            xs_.push_back(points[i].first);
            tree_[base_ + i].push_back(points[i].second);
        }
        for (std::size_t p = base_; p-- > 1;) {
            tree_[p].reserve(tree_[p * 2].size() + tree_[p * 2 + 1].size());
            std::merge(tree_[p * 2].begin(), tree_[p * 2].end(), tree_[p * 2 + 1].begin(),
                       tree_[p * 2 + 1].end(), std::back_inserter(tree_[p]));
        }
    }
    /// @brief 点数（重複も含む）。O(1)。
    int size() const { return static_cast<int>(xs_.size()); }
    /// @brief [x_lower,x_upper) x [y_lower,y_upper)内の点数。空矩形なら0。
    /// @pre lower<=upper（両軸）。上端も座標型で表現すること。最大値+1が必要なら広い座標型を使う。
    int count(const Coordinate &x_lower, const Coordinate &x_upper, const Coordinate &y_lower,
              const Coordinate &y_upper) const {
        assert(!(x_upper < x_lower) && !(y_upper < y_lower));
        std::size_t l = base_ + static_cast<std::size_t>(std::lower_bound(xs_.begin(), xs_.end(), x_lower) -
                                                         xs_.begin()),
                    r = base_ + static_cast<std::size_t>(std::lower_bound(xs_.begin(), xs_.end(), x_upper) -
                                                         xs_.begin());
        int result = 0;
        auto count_y = [&](std::size_t p) {
            return static_cast<int>(std::lower_bound(tree_[p].begin(), tree_[p].end(), y_upper) -
                                    std::lower_bound(tree_[p].begin(), tree_[p].end(), y_lower));
        };
        while (l < r) {
            if (l & 1)
                result += count_y(l++);
            if (r & 1)
                result += count_y(--r);
            l /= 2;
            r /= 2;
        }
        return result;
    }

private:
    std::size_t base_ = 1;
    std::vector<Coordinate> xs_;
    std::vector<std::vector<Coordinate>> tree_;
};
