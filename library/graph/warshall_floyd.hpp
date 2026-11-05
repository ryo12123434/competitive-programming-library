#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

/**
 * @brief 全点対最短距離。到達不能・有限・負閉路による下限なしを区別し、有限経路を復元する。
 * @note 辺追加後に solve()。再実行・solve後の辺追加も可。負辺・平行辺・自己ループも扱う。
 * 内部距離は128bitで、負閉路による巨大な負値だけを飽和させる。有限最短距離は厳密。
 * @pre n>=0、端点は [0,n)。辺重みは long long。有限距離は (n-1)*最大絶対辺重みの範囲。
 * @par Complexity 初期化 O(N^2)、solve O(N^3) 時間・O(N^2) 空間。distance O(1)、path O(N)。
 * @code
 * #include "library/graph/warshall_floyd.hpp"
 * #include <cassert>
 * int main(){WarshallFloyd g(3);g.add_edge(0,1,-2);g.add_edge(1,2,5);g.solve();assert(g.distance(0,2).value==3&&g.path(0,2)->size()==3);}
 * @endcode
 */
class WarshallFloyd {
public:
    using Wide = __int128_t;
    enum class State { Unreachable, Finite, NegativeInfinity };
    /// @brief value は Finite の場合のみ有効。その他では0。distance()で取得する。
    struct Distance {
        State state;
        Wide value;
    };
    /// @brief n 頂点、辺なし。対角距離は0で初期化する。
    explicit WarshallFloyd(int n) : n_(n) {
        assert(n >= 0);
        base_.assign(n, std::vector<Wide>(n, inf_));
        for (int i = 0; i < n; ++i)
            base_[i][i] = 0;
    }
    /// @brief u->v に重み w の辺を追加。既存の辺より軽い場合だけ採用。O(1)。
    void add_edge(int u, int v, long long w) {
        check(u, v);
        base_[u][v] = std::min(base_[u][v], Wide(w));
        solved_ = false;
    }
    /// @brief 全距離と負閉路の影響を再計算する。O(N^3)。
    void solve() {
        dist_ = base_;
        next_.assign(n_, std::vector<int>(n_, -1));
        unbounded_.assign(n_, std::vector<bool>(n_));
        negative_ = false;
        for (int i = 0; i < n_; ++i)
            for (int j = 0; j < n_; ++j)
                if (dist_[i][j] != inf_)
                    next_[i][j] = j;
        for (int k = 0; k < n_; ++k)
            for (int i = 0; i < n_; ++i)
                if (dist_[i][k] != inf_)
                    for (int j = 0; j < n_; ++j)
                        if (dist_[k][j] != inf_) {
                            Wide candidate = std::max(-inf_, dist_[i][k] + dist_[k][j]);
                            if (candidate < dist_[i][j]) {
                                dist_[i][j] = candidate;
                                next_[i][j] = next_[i][k];
                            }
                        }
        for (int k = 0; k < n_; ++k)
            if (dist_[k][k] < 0) {
                negative_ = true;
                for (int i = 0; i < n_; ++i)
                    if (dist_[i][k] != inf_)
                        for (int j = 0; j < n_; ++j)
                            if (dist_[k][j] != inf_)
                                unbounded_[i][j] = true;
            }
        solved_ = true;
    }
    /// @brief グラフのどこかに負閉路があるか。@pre solve 済み。O(1)。
    bool has_negative_cycle() const {
        assert(solved_);
        return negative_;
    }
    /// @brief u->v の状態と有限距離を返す。@pre 最後の辺追加後に solve 済み。
    Distance distance(int u, int v) const {
        check(u, v);
        assert(solved_);
        if (unbounded_[u][v])
            return {State::NegativeInfinity, 0};
        if (dist_[u][v] == inf_)
            return {State::Unreachable, 0};
        return {State::Finite, dist_[u][v]};
    }
    /// @brief 有限最短経路の頂点列。到達不能・負閉路の影響下なら nullopt。u=v なら {u}。
    std::optional<std::vector<int>> path(int u, int v) const {
        if (distance(u, v).state != State::Finite)
            return std::nullopt;
        std::vector<int> result{u};
        while (u != v) {
            u = next_[u][v];
            result.push_back(u);
        }
        return result;
    }

private:
    static constexpr Wide inf_ = Wide(1) << 120;
    int n_;
    bool solved_ = false, negative_ = false;
    std::vector<std::vector<Wide>> base_, dist_;
    std::vector<std::vector<int>> next_;
    std::vector<std::vector<bool>> unbounded_;
    void check([[maybe_unused]] int u, [[maybe_unused]] int v) const {
        assert(0 <= u && u < n_ && 0 <= v && v < n_);
    }
};
