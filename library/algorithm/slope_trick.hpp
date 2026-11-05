#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 整数の折点・整数傾きを持ち、下に有界な凸区分線形関数を管理する Slope Trick。
 * @note 初期関数は f(x)=0。単位の片側ヒンジ・絶対値を加算する。重み付きヒンジ・関数同士のmergeは扱わない。
 * 座標・オフセット・関数値は Wide（符号付き128bit）。全中間演算をその範囲に収めること。
 * @par Complexity
 * 折点数 K。ヒンジ・絶対値追加 O(log(K+1))、定数加算・shift・slide_min・minimum・argmin O(1)。
 * prefix_min/suffix_min は削除する折点数に比例する時間、eval は O(K log(K+1)) 時間・O(K) 追加空間。
 * 保持空間 O(K)。eval は確認用で、コンテストの大量クエリには使わないこと。
 * @code
 * #include "library/algorithm/slope_trick.hpp"
 * #include <cassert>
 * int main(){SlopeTrick f;f.add_abs(2);f.add_abs(6);assert(f.minimum()==4&&f.eval(3)==4);f.slide_min(-1,1);assert(f.eval(1)==4);}
 * @endcode
 */
class SlopeTrick {
public:
    using Wide = __int128_t;
    /// @brief f(x) に定数を加算する。
    void add_constant(Wide value) { minimum_ += value; }
    /// @brief max(0,x-a) を加算する。
    void add_x_minus_a(Wide a) {
        if (!left_.empty() && left_.top() + offset_left_ > a) {
            Wide p = left_.top() + offset_left_;
            left_.pop();
            minimum_ += p - a;
            left_.push(a - offset_left_);
            right_.push(p - offset_right_);
        } else
            right_.push(a - offset_right_);
    }
    /// @brief max(0,a-x) を加算する。
    void add_a_minus_x(Wide a) {
        if (!right_.empty() && right_.top() + offset_right_ < a) {
            Wide p = right_.top() + offset_right_;
            right_.pop();
            minimum_ += a - p;
            right_.push(a - offset_right_);
            left_.push(p - offset_left_);
        } else
            left_.push(a - offset_left_);
    }
    /// @brief |x-a| を加算する。
    void add_abs(Wide a) {
        add_x_minus_a(a);
        add_a_minus_x(a);
    }
    /// @brief 現在の最小関数値。
    Wide minimum() const { return minimum_; }
    /// @brief 最小値を取る閉区間。first=nullopt は -∞、second=nullopt は +∞。
    std::pair<std::optional<Wide>, std::optional<Wide>> argmin() const {
        std::optional<Wide> l, r;
        if (!left_.empty())
            l = left_.top() + offset_left_;
        if (!right_.empty())
            r = right_.top() + offset_right_;
        return {l, r};
    }
    /// @brief f_new(x)=f_old(x-delta) とする平行移動。
    void shift(Wide delta) { slide_min(delta, delta); }
    /// @brief f_new(x)=min_{x-right<=y<=x-left} f_old(y)。@pre left<=right。
    void slide_min(Wide left, Wide right) {
        assert(left <= right);
        offset_left_ += left;
        offset_right_ += right;
    }
    /// @brief f_new(x)=min_{y<=x}f_old(y)。右側の折点をすべて捨てる。
    void prefix_min() { decltype(right_){}.swap(right_); }
    /// @brief f_new(x)=min_{y>=x}f_old(y)。左側の折点をすべて捨てる。
    void suffix_min() { decltype(left_){}.swap(left_); }
    /// @brief f(x) を非破壊で評価。折点ヒープをコピーするため O(K log(K+1))。
    Wide eval(Wide x) const {
        Wide result = minimum_;
        auto l = left_;
        auto r = right_;
        while (!l.empty()) {
            result += std::max(Wide(0), l.top() + offset_left_ - x);
            l.pop();
        }
        while (!r.empty()) {
            result += std::max(Wide(0), x - r.top() - offset_right_);
            r.pop();
        }
        return result;
    }

private:
    std::priority_queue<Wide> left_;
    std::priority_queue<Wide, std::vector<Wide>, std::greater<Wide>> right_;
    Wide minimum_ = 0, offset_left_ = 0, offset_right_ = 0;
};
