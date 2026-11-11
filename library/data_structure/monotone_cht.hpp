#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

/**
 * @brief 傾きが単調な直線を追加するConvex Hull Trick。値はsigned 128bit、空ならnullopt。
 * @tparam Max falseは最小値（傾き非増加）、trueは最大値（傾き非減少）。
 * @pre 全追加の|傾き|,|切片|<=2^62-1。xはsigned 64bit全域。この範囲で積比較・評価を128bit保証する。
 * @note 同じ傾きは良い切片だけ残す。Li Chao Treeと異なり傾き順序が必要だが、座標の事前登録は不要。
 * @par Complexity 追加は償却O(1)、query O(log L)、query_monotoneは追加も含む列全体で償却O(1)。
 * Lは追加本数、保持空間O(L)。最悪1回の追加/単調クエリはO(L)。
 * @code
 * #include "library/data_structure/monotone_cht.hpp"
 * #include <cassert>
 * int main(){MonotoneCHT<> h;h.add_line(2,1);h.add_line(-1,4);assert(h.query(5)==-1);assert(h.query_monotone(5)==-1);}
 * @endcode
 */
template <bool Max = false> class MonotoneCHT {
    using W = __int128_t;
    struct Line {
        long long a, b;
        W value(long long x) const { return W(a) * x + b; }
    };
    std::vector<Line> lines_;
    std::size_t head_ = 0;
    std::optional<long long> last_slope_, last_x_;
    static bool redundant(Line a, Line b, Line c) {
        return (W(b.b) - a.b) * (W(b.a) - c.a) >= (W(c.b) - b.b) * (W(a.a) - b.a);
    }
    static W output(W x) {
        if constexpr (Max)
            return -x;
        else
            return x;
    }

public:
    /// @brief 空の集合を生成する。O(1)。
    MonotoneCHT() = default;
    /// @brief 直線a*x+bを追加。傾き順序と数値制約はクラスの前提条件に従う。
    void add_line(long long a, long long b) {
        [[maybe_unused]] constexpr long long bound = (1LL << 62) - 1;
        assert(-bound <= a && a <= bound && -bound <= b && b <= bound);
        if constexpr (Max) {
            a = -a;
            b = -b;
        }
        assert(!last_slope_ || a <= *last_slope_);
        last_slope_ = a;
        if (!lines_.empty() && lines_.back().a == a) {
            if (lines_.back().b <= b)
                return;
            lines_.pop_back();
        }
        Line line{a, b};
        while (lines_.size() >= 2 && redundant(lines_[lines_.size() - 2], lines_.back(), line))
            lines_.pop_back();
        lines_.push_back(line);
        head_ = std::min(head_, lines_.size() - 1);
    }
    /// @brief 任意順のxの最適値。O(log L)時間・O(1)追加空間。
    std::optional<W> query(long long x) const {
        if (lines_.empty())
            return std::nullopt;
        std::size_t l = 0, r = lines_.size() - 1;
        while (l < r) {
            auto m = l + (r - l) / 2;
            if (lines_[m + 1].value(x) <= lines_[m].value(x))
                l = m + 1;
            else
                r = m;
        }
        return output(lines_[l].value(x));
    }
    /// @brief xが非減少のクエリ。通常queryとの混在可。追加も含む操作列で償却O(1)。
    std::optional<W> query_monotone(long long x) {
        assert(!last_x_ || *last_x_ <= x);
        last_x_ = x;
        if (lines_.empty())
            return std::nullopt;
        while (head_ + 1 < lines_.size() && lines_[head_ + 1].value(x) <= lines_[head_].value(x))
            ++head_;
        return output(lines_[head_].value(x));
    }
};
