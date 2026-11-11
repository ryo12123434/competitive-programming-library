#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief 最新時刻だけを更新し、全ての過去時刻を参照できる部分永続Union-Find。
 * @note 時刻0は初期状態。失敗mergeも時刻を進める。巻き戻し・過去からの分岐は不可。
 * union by sizeを使い、経路圧縮は行わない。履歴は厳密に増加する時刻順に保持する。
 * @par Complexity 構築O(N)、merge/leader/sameはO(log(N+1))、sizeはO(log(N+1)+log(Q+1))。
 * mergeの配列拡張は償却、保持空間O(N+Q)。Qはmerge回数（成功は高々N-1）。
 * @code
 * #include "library/data_structure/partially_persistent_union_find.hpp"
 * #include <cassert>
 * int main(){PartiallyPersistentUnionFind u(3);u.merge(0,1);auto t=u.time();u.merge(5,1,2);assert(u.size(t,0)==2&&u.size(5,0)==3&&!u.same(0,0,1));}
 * @endcode
 */
class PartiallyPersistentUnionFind {
public:
    using Time = std::int64_t;

private:
    Time now_ = 0;
    std::vector<int> parent_;
    std::vector<Time> joined_;
    std::vector<std::vector<std::pair<Time, int>>> history_;
    void check_time(Time t) const {
        if (t < 0 || t > now_)
            throw std::invalid_argument("PPUF: time outside the past");
    }

public:
    /// @brief n>=0頂点で構築。各頂点が大きさ1の独立成分。
    explicit PartiallyPersistentUnionFind(int n) {
        assert(n >= 0);
        parent_.assign(n, -1);
        joined_.assign(n, std::numeric_limits<Time>::max());
        history_.assign(n, {{0, 1}});
    }
    /// @brief 最新の更新時刻。構築直後は0。O(1)。
    Time time() const { return now_; }
    /// @brief 時刻tの代表。@pre v∈[0,N)。未来・負の時刻はinvalid_argument。
    int leader(Time t, int v) const {
        check_time(t);
        assert(0 <= v && v < static_cast<int>(parent_.size()));
        while (parent_[v] >= 0 && joined_[v] <= t)
            v = parent_[v];
        return v;
    }
    /// @brief 時刻tでの連結判定。頂点・時刻の前提条件はleaderと同じ。
    bool same(Time t, int a, int b) const { return leader(t, a) == leader(t, b); }
    /// @brief 時刻tの成分サイズ。代表を求めた後、サイズ履歴を二分探索する。
    int size(Time t, int v) const {
        int root = leader(t, v);
        const auto &h = history_[root];
        auto it = std::upper_bound(h.begin(), h.end(), t, [](Time x, const auto &e) { return x < e.first; });
        return std::prev(it)->second;
    }
    /// @brief 指定時刻tでmerge。tは現在より大きく、INT64_MAX未満。違反はinvalid_argument。
    /// @return 2成分を結合したならtrue。同一成分ならfalseでも時刻はtになる。
    bool merge(Time t, int a, int b) {
        if (t <= now_ || t == std::numeric_limits<Time>::max())
            throw std::invalid_argument("PPUF: merge time must strictly increase");
        int x = leader(now_, a), y = leader(now_, b);
        now_ = t;
        if (x == y)
            return false;
        if (history_[x].back().second < history_[y].back().second)
            std::swap(x, y);
        int size = history_[x].back().second + history_[y].back().second;
        parent_[y] = x;
        joined_[y] = t;
        history_[x].emplace_back(t, size);
        return true;
    }
    /// @brief 現在時刻+1でmergeする。上限に達するとoverflow_error。
    bool merge(int a, int b) {
        if (now_ >= std::numeric_limits<Time>::max() - 1)
            throw std::overflow_error("PPUF: time exhausted");
        return merge(now_ + 1, a, b);
    }
};
