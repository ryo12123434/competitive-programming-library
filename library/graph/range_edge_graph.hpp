#pragma once
#include "dijkstra.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

/**
 * @brief 点・半開区間間の非負有向辺を、上下2本のセグ木で疎なグラフへ展開する。
 * @note 元の頂点IDは[0,n)。区間→区間は専用中継頂点1個とO(log n)辺を使う。
 * @pre n>=0、全展開頂点数がintに収まり、有限距離はDijkstra::INF未満であること。
 * @par Complexity 構築O(N)時間・空間。各区間辺追加O(log(N+1))、点辺は償却O(1)。
 * Q回の区間追加でO(N+Q log(N+1))辺・空間。最短路は既存Dijkstraの計算量。
 * @code
 * #include "library/graph/range_edge_graph.hpp"
 * #include <cassert>
 * int main(){RangeEdgeGraph g(4);g.add_range_to_range(0,2,2,4,7);auto d=g.distances(0);assert(d[2]==7&&d[3]==7);}
 * @endcode
 */
class RangeEdgeGraph {
    int n_;
    Dijkstra graph_;
    std::vector<int> up_, down_;
    static int checked(int n) {
        if (n < 0 || n > std::numeric_limits<int>::max() / 3)
            throw std::length_error("RangeEdgeGraph: invalid size");
        return n;
    }
    int vertex() {
        assert(graph_.V < std::numeric_limits<int>::max());
        graph_.G.emplace_back();
        return graph_.V++;
    }
    void build(std::size_t p, int l, int r) {
        if (r - l == 1) {
            up_[p] = down_[p] = l;
            return;
        }
        int m = l + (r - l) / 2;
        build(2 * p, l, m);
        build(2 * p + 1, m, r);
        up_[p] = vertex();
        down_[p] = vertex();
        for (auto c : {2 * p, 2 * p + 1}) {
            graph_.add_edge(up_[c], up_[p], 0);
            graph_.add_edge(down_[p], down_[c], 0);
        }
    }
    template <class F> void cover(int ql, int qr, F f) const {
        auto walk = [&](auto &&self, std::size_t p, int l, int r) -> void {
            if (qr <= l || r <= ql)
                return;
            if (ql <= l && r <= qr) {
                f(p);
                return;
            }
            int m = l + (r - l) / 2;
            self(self, 2 * p, l, m);
            self(self, 2 * p + 1, m, r);
        };
        assert(0 <= ql && ql <= qr && qr <= n_);
        if (ql < qr)
            walk(walk, 1, 0, n_);
    }

public:
    explicit RangeEdgeGraph(int n) : n_(checked(n)), graph_(n_) {
        up_.resize(static_cast<std::size_t>(n_) * 4);
        down_.resize(up_.size());
        if (n_)
            build(1, 0, n_);
    }
    /// @brief 元の点uからvへの辺。重みは非負。
    void add_edge(int u, int v, long long cost) {
        assert(0 <= u && u < n_ && 0 <= v && v < n_ && cost >= 0);
        graph_.add_edge(u, v, cost);
    }
    /// @brief 点uから区間[l,r)の全頂点へcostの辺を追加。空区間は何もしない。
    void add_point_to_range(int u, int l, int r, long long cost) {
        assert(0 <= u && u < n_ && cost >= 0);
        cover(l, r, [&](auto p) { graph_.add_edge(u, down_[p], cost); });
    }
    /// @brief 区間[l,r)の全頂点から点vへcostの辺を追加。
    void add_range_to_point(int l, int r, int v, long long cost) {
        assert(0 <= v && v < n_ && cost >= 0);
        cover(l, r, [&](auto p) { graph_.add_edge(up_[p], v, cost); });
    }
    /// @brief [l1,r1)から[l2,r2)への全辺。どちらかが空なら何もしない。
    void add_range_to_range(int l1, int r1, int l2, int r2, long long cost) {
        assert(0 <= l1 && l1 <= r1 && r1 <= n_ && 0 <= l2 && l2 <= r2 && r2 <= n_ && cost >= 0);
        if (l1 == r1 || l2 == r2)
            return;
        int mid = vertex();
        cover(l1, r1, [&](auto p) { graph_.add_edge(up_[p], mid, 0); });
        cover(l2, r2, [&](auto p) { graph_.add_edge(mid, down_[p], cost); });
    }
    /// @brief 元のsourceから、元の全n頂点への距離。到達不能はDijkstra::INF。
    std::vector<long long> distances(int source) {
        assert(0 <= source && source < n_);
        graph_.solve_dijk(source);
        return {graph_.dis.begin(), graph_.dis.begin() + n_};
    }
    /// @brief 展開グラフの読み取り専用参照。元頂点は先頭n個。構築後の解析用。
    const Dijkstra &expanded_graph() const { return graph_; }
};
