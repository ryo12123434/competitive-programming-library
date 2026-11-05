#pragma once
#include <algorithm>
#include <cassert>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief Hopcroft–Karpによる二部グラフ最大マッチング。左右の頂点番号は独立に0-indexed。
 * @note 多重辺・孤立点・空の側も許す。再帰を使わない。solve後にmatch_left/rightを参照する。
 * ACL最大流でも解けるが、容量ネットワークを作らず二部専用に処理し、最小頂点被覆も返す。
 * @par Complexity solveは初期化O(V)に加え O(E*sqrt(V+1)) 時間、O(V+E)空間。
 * 孤立点は各増加段階の走査から除く。構築O(V)、add_edgeは償却O(1)。
 * @code
 * #include "library/graph/bipartite_matching.hpp"
 * #include <cassert>
 * int main(){BipartiteMatching g(2,2);g.add_edge(0,1);g.add_edge(1,0);assert(g.solve()==2);auto [l,r]=g.minimum_vertex_cover();assert(l.size()+r.size()==2);}
 * @endcode
 */
class BipartiteMatching {
    int left_, right_;
    std::vector<std::vector<int>> graph_;
    bool built_ = false;

public:
    std::vector<int> match_left, match_right;
    /// @brief 左n_left・右n_right頂点で構築。@pre 両方非負、和はintに収まる。
    BipartiteMatching(int n_left, int n_right) : left_(n_left), right_(n_right) {
        assert(n_left >= 0 && n_right >= 0);
        graph_.resize(left_);
    }
    /// @brief 左uと右vに辺を追加。結果を無効化する。@pre u∈[0,n_left),v∈[0,n_right)。
    void add_edge(int u, int v) {
        assert(0 <= u && u < left_ && 0 <= v && v < right_);
        graph_[u].push_back(v);
        built_ = false;
    }
    /// @brief 毎回マッチングを初期化し、最大サイズを返す。未マッチ頂点の相手は-1。
    int solve() {
        match_left.assign(left_, -1);
        match_right.assign(right_, -1);
        std::vector<int> distance(left_), next(left_);
        std::vector<int> active;
        for (int v = 0; v < left_; ++v)
            if (!graph_[v].empty())
                active.push_back(v);
        int total = 0;
        while (true) {
            for (int v : active)
                distance[v] = -1;
            std::queue<int> queue;
            for (int v : active)
                if (match_left[v] < 0) {
                    distance[v] = 0;
                    queue.push(v);
                }
            int shortest = -1;
            while (!queue.empty()) {
                int v = queue.front();
                queue.pop();
                if (shortest >= 0 && distance[v] + 1 > shortest)
                    continue;
                for (int r : graph_[v]) {
                    int to = match_right[r];
                    if (to < 0)
                        shortest = distance[v] + 1;
                    else if (distance[to] < 0 && (shortest < 0 || distance[v] + 1 < shortest)) {
                        distance[to] = distance[v] + 1;
                        queue.push(to);
                    }
                }
            }
            if (shortest < 0)
                break;
            for (int v : active)
                next[v] = 0;
            for (int root : active)
                if (match_left[root] < 0 && distance[root] == 0) {
                    std::vector<int> path{root}, via;
                    while (!path.empty()) {
                        int v = path.back();
                        bool descended = false, augmented = false;
                        while (next[v] < static_cast<int>(graph_[v].size())) {
                            int r = graph_[v][next[v]++], to = match_right[r];
                            if (to < 0 && distance[v] + 1 == shortest) {
                                match_left[v] = r;
                                match_right[r] = v;
                                for (int i = static_cast<int>(via.size()) - 1; i >= 0; --i) {
                                    match_left[path[i]] = via[i];
                                    match_right[via[i]] = path[i];
                                }
                                ++total;
                                augmented = true;
                                break;
                            }
                            if (to >= 0 && distance[to] == distance[v] + 1 && distance[to] < shortest) {
                                path.push_back(to);
                                via.push_back(r);
                                descended = true;
                                break;
                            }
                        }
                        if (augmented)
                            break;
                        if (descended)
                            continue;
                        distance[v] = -1;
                        path.pop_back();
                        if (!via.empty())
                            via.pop_back();
                    }
                }
        }
        built_ = true;
        return total;
    }
    /// @brief 最大マッチングの(左,右)列。O(V)時間・出力空間。@pre 最後の追加後solve済み。
    std::vector<std::pair<int, int>> matching() const {
        assert(built_);
        std::vector<std::pair<int, int>> result;
        for (int v = 0; v < left_; ++v)
            if (match_left[v] >= 0)
                result.emplace_back(v, match_left[v]);
        return result;
    }
    /// @brief 最小頂点被覆の(左頂点列,右頂点列)。O(V+E)時間・O(V)空間。@pre solve済み。
    std::pair<std::vector<int>, std::vector<int>> minimum_vertex_cover() const {
        assert(built_);
        std::vector<bool> left(left_), right(right_);
        std::queue<int> queue;
        for (int v = 0; v < left_; ++v)
            if (match_left[v] < 0) {
                left[v] = true;
                queue.push(v);
            }
        while (!queue.empty()) {
            int v = queue.front();
            queue.pop();
            for (int r : graph_[v])
                if (r != match_left[v] && !right[r]) {
                    right[r] = true;
                    int to = match_right[r];
                    if (to >= 0 && !left[to]) {
                        left[to] = true;
                        queue.push(to);
                    }
                }
        }
        std::pair<std::vector<int>, std::vector<int>> result;
        for (int v = 0; v < left_; ++v)
            if (!left[v])
                result.first.push_back(v);
        for (int v = 0; v < right_; ++v)
            if (right[v])
                result.second.push_back(v);
        return result;
    }
};
