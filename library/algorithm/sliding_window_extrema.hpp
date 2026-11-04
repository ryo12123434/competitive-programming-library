#pragma once
#include <cassert>
#include <cstddef>
#include <deque>
#include <functional>
#include <limits>
#include <vector>

/// @brief 固定窓の極値。indices[i] と values[i] は窓 [i,i+width) の元添字・値。
template <class T> struct SlidingWindowExtrema {
    std::vector<int> indices;
    std::vector<T> values;
};
/**
 * @brief 単調dequeで固定長の各窓の最小値または最大値を求める。
 * @param a 入力列。@param width 窓幅。0・配列長超過なら空の結果。
 * @param compare less は最小、greater は最大。比較同値なら最も左の添字を選ぶ。
 * @pre a.size()<=INT_MAX。T はコピー可能、compare は狭義弱順序。
 * @par Complexity コピー・比較 O(1) なら O(N) 時間、補助 O(width)、結果 O(N) 空間。
 * @code
 * #include "library/algorithm/sliding_window_extrema.hpp"
 * #include <cassert>
 * int main(){auto r=sliding_window_extrema(std::vector<int>{2,1,1},2);assert((r.indices==std::vector<int>{1,1}));}
 * @endcode
 */
template <class T, class Compare = std::less<T>>
SlidingWindowExtrema<T> sliding_window_extrema(const std::vector<T> &a, std::size_t width,
                                               Compare compare = {}) {
    assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    SlidingWindowExtrema<T> result;
    if (width == 0 || width > a.size())
        return result;
    std::deque<int> queue;
    int w = static_cast<int>(width), n = static_cast<int>(a.size());
    for (int i = 0; i < n; ++i) {
        while (!queue.empty() && queue.front() <= i - w)
            queue.pop_front();
        while (!queue.empty() && compare(a[i], a[queue.back()]))
            queue.pop_back();
        queue.push_back(i);
        if (i + 1 >= w) {
            result.indices.push_back(queue.front());
            result.values.push_back(a[queue.front()]);
        }
    }
    return result;
}
/// @brief 最小値版。仕様・計算量は sliding_window_extrema と同じ。
template <class T> SlidingWindowExtrema<T> sliding_window_min(const std::vector<T> &a, std::size_t width) {
    return sliding_window_extrema(a, width);
}
/// @brief 最大値版。仕様・計算量は sliding_window_extrema と同じ。
template <class T> SlidingWindowExtrema<T> sliding_window_max(const std::vector<T> &a, std::size_t width) {
    return sliding_window_extrema(a, width, std::greater<T>{});
}
