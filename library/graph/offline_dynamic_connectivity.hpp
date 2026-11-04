#pragma once

#include "../data_structure/rollback_union_find.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>
#include <vector>

/**
 * @brief 時間軸セグ木と RollbackUnionFind による無向グラフのオフライン動的連結性。
 * @note 操作を登録して solve()。同じ辺の多重度を数え、最後の1本が消えるまで連結を維持する。
 * 自己ループも可。失敗した remove_edge は何も変更せず時刻も進めない。
 * 各成功操作・クエリが1時刻。クエリはそれ以前の追加・削除を反映する。solve は再実行・追加登録可能。
 * @pre N>=0、端点は [0,N)。操作数・同じ辺の多重度・クエリ数は int に収まること。
 * @par Complexity
 * Q 操作、生存区間 U<=Q。登録 O(log(Q+1)) / 辺操作、クエリ登録償却 O(1)。
 * solve は O((U log(Q+1)+Q) log(N+1)) 時間、保持・作業空間 O(N+Q+U log(Q+1))。
 * 木の再帰は時間軸の O(log Q) 段だけで、グラフの深さには依存しない。
 * @code
 * #include "library/graph/offline_dynamic_connectivity.hpp"
 * #include <cassert>
 * int main() {
 *     OfflineDynamicConnectivity graph(3); graph.add_edge(0,1);
 *     int a=graph.query_same(0,1); graph.remove_edge(1,0);
 *     int b=graph.query_components(); auto answers=graph.solve();
 *     assert(answers[a]==1 && answers[b]==3);
 * }
 * @endcode
 */
class OfflineDynamicConnectivity {
public:
    /// @brief n 頂点・辺なしで構築。@pre n>=0。
    explicit OfflineDynamicConnectivity(int n) : n_(n) { assert(n >= 0); }
    /// @brief 無向辺を1本追加する。端点の順序は問わない。
    void add_edge(int u, int v) {
        auto edge = normalize(u, v); int time = next_time();
        auto& active = active_[edge];
        assert(active.count < std::numeric_limits<int>::max());
        if (active.count++ == 0) active.begin = time;
    }
    /// @brief 無向辺を1本削除。存在しない辺なら false、成功なら true。
    bool remove_edge(int u, int v) {
        auto edge = normalize(u, v); auto it = active_.find(edge);
        if (it == active_.end()) return false;
        int time = next_time();
        if (--it->second.count == 0) {
            intervals_.push_back({it->second.begin, time, edge}); active_.erase(it);
        }
        return true;
    }
    /// @brief 現在の u,v の連結性を問い合わせ、回答 ID を返す。回答は 0 / 1。
    int query_same(int u, int v) {
        normalize(u, v); return add_query({0, u, v, -1});
    }
    /// @brief 現在の連結成分数を問い合わせ、回答 ID を返す。N=0 なら回答 0。
    int query_components() { return add_query({1, 0, 0, -1}); }
    /// @brief 登録した全クエリの答えを ID 順に返す。登録内容は変更しない。
    std::vector<int> solve() const {
        std::vector<int> answers(queries_);
        if (events_.empty()) return answers;
        const int q = static_cast<int>(events_.size());
        std::size_t base = 1; while (base < events_.size()) base *= 2;
        std::vector<std::vector<std::pair<int, int>>> segments(base * 2);
        auto insert = [&](int l, int r, std::pair<int, int> edge) {
            for (std::size_t a = base + l, b = base + r; a < b; a /= 2, b /= 2) {
                if (a & 1) segments[a++].push_back(edge);
                if (b & 1) segments[--b].push_back(edge);
            }
        };
        for (const auto& interval : intervals_) insert(interval.l, interval.r, interval.edge);
        for (const auto& [edge, active] : active_) insert(active.begin, q, edge);
        RollbackUnionFind uf(n_);
        auto visit = [&](auto&& self, std::size_t p) -> void {
            auto token = uf.snapshot();
            for (auto [u, v] : segments[p]) uf.merge(u, v);
            if (p < base) { self(self, p * 2); self(self, p * 2 + 1); }
            else if (p - base < events_.size()) {
                auto event = events_[p - base];
                if (event.kind == 0) answers[event.id] = uf.same(event.u, event.v);
                if (event.kind == 1) answers[event.id] = uf.components();
            }
            [[maybe_unused]] bool restored = uf.rollback(token); assert(restored);
        };
        visit(visit, 1); return answers;
    }
private:
    struct Event { int kind = -1, u = 0, v = 0, id = -1; };
    struct Active { int count = 0, begin = 0; };
    struct Interval { int l, r; std::pair<int, int> edge; };
    int n_, queries_ = 0;
    std::vector<Event> events_;
    std::vector<Interval> intervals_;
    std::map<std::pair<int, int>, Active> active_;
    std::pair<int, int> normalize(int u, int v) const {
        assert(0 <= u && u < n_ && 0 <= v && v < n_); return std::minmax(u, v);
    }
    int next_time() {
        assert(events_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int time = static_cast<int>(events_.size()); events_.push_back({}); return time;
    }
    int add_query(Event event) {
        event.id = queries_++; int time = next_time(); events_[time] = event; return event.id;
    }
};
