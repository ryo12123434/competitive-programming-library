#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

/**
 * @brief [0,n)の整数集合を64分木の階層ビット集合で管理する。
 * @note std::setと異なり整数の固定宇宙が必要。小さい宇宙を密に使う場合に省メモリ。
 * next/prevは境界を「含む」。範囲外の境界は宇宙へ切り詰め、見つからなければnullopt。
 * @par Complexity 構築O(ceil(n/64)+log(n+1))時間・空間（64bit語数）、文字列構築O(n)。
 * insert/erase/next/prev/anyは O(log_64(n+1)+1)、contains/sizeはO(1)。
 * @code
 * #include "library/data_structure/fast_set.hpp"
 * #include <cassert>
 * int main(){FastSet s(100);s.insert(63);s.insert(64);assert(s.next(1)==63&&s.prev(100)==64&&s.any(63,64));}
 * @endcode
 */
class FastSet {
    using U = std::uint64_t;
    int n_, count_ = 0;
    std::vector<std::vector<U>> levels_;
    void build_upper() {
        for (std::size_t h = 1; h < levels_.size(); ++h)
            for (std::size_t i = 0; i < levels_[h - 1].size(); ++i)
                if (levels_[h - 1][i])
                    levels_[h][i / 64] |= U(1) << (i % 64);
    }

public:
    /// @brief n>=0の宇宙を空または全要素で初期化する。
    explicit FastSet(int n = 0, bool filled = false) : n_(n) {
        assert(n >= 0);
        std::size_t width = n;
        while (width) {
            width = (width + 63) / 64;
            levels_.emplace_back(width, 0);
            if (width == 1)
                break;
        }
        if (filled && n) {
            std::fill(levels_[0].begin(), levels_[0].end(), ~U(0));
            if (n % 64)
                levels_[0].back() >>= 64 - n % 64;
            count_ = n;
            build_upper();
        }
    }
    /// @brief '0'/'1'列から初期化。i番が'1'ならiを含む。@pre 長さはintに収まる。
    explicit FastSet(std::string_view bits) : FastSet(static_cast<int>(bits.size())) {
        for (int i = 0; i < n_; ++i) {
            assert(bits[i] == '0' || bits[i] == '1');
            if (bits[i] == '1') {
                levels_[0][i / 64] |= U(1) << (i % 64);
                ++count_;
            }
        }
        build_upper();
    }
    /// @brief 集合の要素数。O(1)。
    int size() const { return count_; }
    /// @brief 空集合か。O(1)。
    bool empty() const { return count_ == 0; }
    /// @brief 含むかを返す。宇宙外はfalse。
    bool contains(int x) const { return 0 <= x && x < n_ && (levels_[0][x / 64] >> (x % 64) & 1); }
    /// @brief xを追加し、変化したかを返す。@pre x∈[0,n)。
    bool insert(int x) {
        assert(0 <= x && x < n_);
        if (contains(x))
            return false;
        ++count_;
        std::size_t index = x;
        for (auto &level : levels_) {
            auto word = index / 64;
            U old = level[word];
            level[word] |= U(1) << (index % 64);
            if (old)
                break;
            index = word;
        }
        return true;
    }
    /// @brief xを削除し、変化したかを返す。@pre x∈[0,n)。不存在ならfalse。
    bool erase(int x) {
        assert(0 <= x && x < n_);
        if (!contains(x))
            return false;
        --count_;
        std::size_t index = x;
        for (auto &level : levels_) {
            auto word = index / 64;
            level[word] &= ~(U(1) << (index % 64));
            if (level[word])
                break;
            index = word;
        }
        return true;
    }
    /// @brief x以上の最小要素。x<0も可。見つからなければnullopt。
    std::optional<int> next(int x) const {
        if (x < 0)
            x = 0;
        if (x >= n_)
            return std::nullopt;
        std::size_t index = x;
        for (std::size_t h = 0; h < levels_.size(); ++h) {
            if (index / 64 >= levels_[h].size())
                return std::nullopt;
            U word = levels_[h][index / 64] >> (index % 64);
            if (word) {
                index += std::countr_zero(word);
                for (std::size_t down = h; down > 0; --down)
                    index = index * 64 + std::countr_zero(levels_[down - 1][index]);
                return static_cast<int>(index);
            }
            index = index / 64 + 1;
        }
        return std::nullopt;
    }
    /// @brief x以下の最大要素。x>=nも可。見つからなければnullopt。
    std::optional<int> prev(int x) const {
        if (x >= n_)
            x = n_ - 1;
        if (x < 0)
            return std::nullopt;
        std::size_t index = x;
        for (std::size_t h = 0; h < levels_.size(); ++h) {
            U word = levels_[h][index / 64] & (~U(0) >> (63 - index % 64));
            if (word) {
                index = index / 64 * 64 + 63 - std::countl_zero(word);
                for (std::size_t down = h; down > 0; --down)
                    index = index * 64 + 63 - std::countl_zero(levels_[down - 1][index]);
                return static_cast<int>(index);
            }
            if (index / 64 == 0)
                return std::nullopt;
            index = index / 64 - 1;
        }
        return std::nullopt;
    }
    /// @brief 最小要素。空ならnullopt。計算量はnextと同じ。
    std::optional<int> min() const { return next(0); }
    /// @brief 最大要素。空ならnullopt。計算量はprevと同じ。
    std::optional<int> max() const { return prev(n_ - 1); }
    /// @brief 半開区間[l,r)に要素があるか。@pre 0<=l<=r<=n。
    bool any(int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        auto x = next(l);
        return x && *x < r;
    }
};
