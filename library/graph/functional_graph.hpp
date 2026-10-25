#pragma once

#include <cassert>
#include <limits>
#include <vector>

/**
 * @brief 各頂点からちょうど 1 本の辺が出るグラフを、閉路と流入木に分解する。
 * @note 頂点は 0-indexed。k 回遷移は既存の Doubling に同じ next を渡して求める。
 * @par Complexity
 * 構築 O(N) 時間・空間。各頂点の情報取得と cycles の参照は O(1)。
 * @code
 * #include "library/graph/functional_graph.hpp"
 * #include <cassert>
 * int main() {
 *     FunctionalGraph graph(std::vector<int>{1, 2, 1, 2, 4});
 *     assert(graph.cycles().size() == 2);
 *     assert(graph.on_cycle(1) && !graph.on_cycle(0));
 *     assert(graph.cycle_id(0) == graph.cycle_id(3));
 *     assert(graph.distance_to_cycle(0) == 1 && graph.cycle_entry(0) == 1);
 *     assert(graph.reachable_count(0) == 3); // 出発点自身も数える
 * }
 * @endcode
 */
class FunctionalGraph {
public:
    /**
     * @brief 遷移先配列から閉路・各頂点の情報を前計算する。
     * @param next next[v] は v の唯一の遷移先。空配列も可。
     * @pre N = next.size() は int に収まり、各 next[v] は [0, N)。-1 は不可。
     * @note 構築後に元の配列を変更しても結果は変わらない。
     * @par Complexity
     * O(N) 時間・空間。
     */
    explicit FunctionalGraph(const std::vector<int>& next) {
        assert(next.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const int n = static_cast<int>(next.size());
        std::vector<int> indegree(n), removed;
        removed.reserve(n);
        for (int to : next) {
            assert(0 <= to && to < n);
            ++indegree[to];
        }
        for (int v = 0; v < n; ++v) if (indegree[v] == 0) removed.push_back(v);
        for (std::size_t i = 0; i < removed.size(); ++i) {
            if (--indegree[next[removed[i]]] == 0) removed.push_back(next[removed[i]]);
        }
        cycle_id_.assign(n, -1);
        distance_.assign(n, 0);
        entry_.assign(n, -1);
        for (int v = 0; v < n; ++v) {
            if (indegree[v] == 0 || cycle_id_[v] != -1) continue;
            const int id = static_cast<int>(cycles_.size());
            cycles_.emplace_back();
            int u = v;
            do {
                cycles_.back().push_back(u);
                cycle_id_[u] = id;
                entry_[u] = u;
                u = next[u];
            } while (u != v);
        }
        for (auto it = removed.rbegin(); it != removed.rend(); ++it) {
            const int v = *it, to = next[v];
            cycle_id_[v] = cycle_id_[to];
            distance_[v] = distance_[to] + 1;
            entry_[v] = entry_[to];
        }
    }

    /// @brief 頂点数を O(1) で返す。
    int size() const { return static_cast<int>(cycle_id_.size()); }

    /**
     * @brief 全閉路の頂点列を O(1) で参照する。コピーはしない。
     * @return 各列は遷移順で、末尾から先頭へ戻る。先頭は閉路の最小頂点。
     * @note 閉路 ID は列の添字。閉路の最小頂点の昇順で採番する。空グラフでは空。
     */
    const std::vector<std::vector<int>>& cycles() const { return cycles_; }

    /// @brief v が閉路上なら true。O(1)。@param v 頂点番号。@pre v は [0, size())。
    bool on_cycle(int v) const { return distance_to_cycle(v) == 0; }

    /// @brief 到達する閉路の ID を O(1) で返す。@param v 頂点番号。@pre v は [0, size())。
    int cycle_id(int v) const { check_vertex(v); return cycle_id_[v]; }

    /// @brief 閉路に入るまでの辺数を O(1) で返す。閉路上は 0。@param v 頂点番号。@pre v は [0, size())。
    int distance_to_cycle(int v) const { check_vertex(v); return distance_[v]; }

    /// @brief 最初に到達する閉路上の頂点を O(1) で返す。閉路上なら v 自身。@param v 頂点番号。@pre v は [0, size())。
    int cycle_entry(int v) const { check_vertex(v); return entry_[v]; }

    /**
     * @brief 到達可能な異なる頂点数（v 自身を含む）を O(1) で返す。
     * @param v 頂点番号。@pre v は [0, size())。
     * @return 閉路までの距離 + 到達閉路の長さ。最大 size()。
     */
    int reachable_count(int v) const {
        check_vertex(v);
        return distance_[v] + static_cast<int>(cycles_[cycle_id_[v]].size());
    }

private:
    std::vector<std::vector<int>> cycles_;
    std::vector<int> cycle_id_, distance_, entry_;
    void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
};
