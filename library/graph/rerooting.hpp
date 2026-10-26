#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

/**
 * @brief Rerooting の有向隣接辺。to は行き先、id は transfer に渡す利用者指定の辺ID。
 * @note 無向辺は両方向に格納する。両方向で異なる id も使用できる。構築例は rerooting を参照。
 */
struct RerootingEdge { int to, id; };

/**
 * @brief 全頂点を根とした木DPを反復探索で求める。可換性は不要。
 * @tparam State DPの値型。コピー可能であること。
 * @param tree 各無向辺を両方向に格納した木。N==0 は空の結果を返す。
 * @param merge State(State,State)。結合律を満たし、identity を両側の単位元とする。
 * @param identity 集約の単位元。葉の空集約もこの値。
 * @param finalize State(State aggregate,int vertex)。頂点の重み等を加えてDPを確定する純粋関数。
 * @param transfer State(State dp,int from,int to,int edge_id)。from側の確定DPを to への寄与へ写す純粋関数。
 * @return 各 v を根とした確定DP。隣接部分木の寄与は常に tree[v] の格納順で結合する。
 * 方向 v→p のDPは辺を切った v 側で、p の隣接スロットだけを除き、残りの順序を保ったもの。
 * @pre N<=INT_MAX の連結な無向木。各 to は [0,N)。各演算・中間値が State に収まること。
 * @par Complexity
 * 状態コピー・各コールバックが O(1) のとき O(N) 時間・空間。文字列等では実際の演算量が加わる。
 * suffix と走査中の prefix で各隣接辺を除く集約を計算し、再帰は使わない。
 * @code
 * #include "library/graph/rerooting.hpp"
 * #include <cassert>
 * struct S { long long sum, count; };
 * int main() {
 *     std::vector<std::vector<RerootingEdge>> tree{{{1,0}},{{0,0},{2,1}},{{1,1}}};
 *     auto result = rerooting<S>(tree,
 *         [](S a,S b){return S{a.sum+b.sum,a.count+b.count};}, S{0,0},
 *         [](S a,int){return S{a.sum,a.count+1};},
 *         [](S a,int,int,int){return S{a.sum+a.count,a.count};});
 *     assert(result[0].sum==3 && result[1].sum==2); // 距離の総和
 * }
 * @endcode
 */
template<class State, class Merge, class Finalize, class Transfer>
std::vector<State> rerooting(const std::vector<std::vector<RerootingEdge>>& tree,
                            Merge merge, State identity, Finalize finalize, Transfer transfer) {
    assert(tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int n = static_cast<int>(tree.size());
    if (n == 0) return {};
    std::vector<int> parent(n, -2), order, parent_slot(n, -1), child_slot(n, -1);
    std::vector<std::vector<State>> incoming;
    incoming.reserve(n);
    for (const auto& edges : tree) incoming.emplace_back(edges.size(), identity);
    order.reserve(n); order.push_back(0); parent[0] = -1;
    for (std::size_t i = 0; i < order.size(); ++i) {
        const int v = order[i];
        assert(tree[v].size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        for (int slot = 0; slot < static_cast<int>(tree[v].size()); ++slot) {
            const int to = tree[v][slot].to;
            assert(0 <= to && to < n);
            if (to == parent[v]) { parent_slot[v] = slot; continue; }
            assert(parent[to] == -2);
            parent[to] = v; child_slot[to] = slot; order.push_back(to);
        }
        assert(v == 0 || parent_slot[v] != -1);
    }
    assert(static_cast<int>(order.size()) == n);
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        const int v = *it;
        State aggregate = identity;
        for (int i = 0; i < static_cast<int>(tree[v].size()); ++i)
            if (i != parent_slot[v]) aggregate = merge(aggregate, incoming[v][i]);
        if (parent[v] != -1)
            incoming[parent[v]][child_slot[v]] = transfer(finalize(aggregate,v),v,parent[v],tree[v][parent_slot[v]].id);
    }
    std::vector<State> result(n, identity);
    for (int v : order) {
        const auto degree = tree[v].size();
        std::vector<State> suffix(degree+1, identity);
        for (auto i = degree; i > 0; --i) suffix[i-1] = merge(incoming[v][i-1],suffix[i]);
        result[v] = finalize(suffix[0], v);
        State prefix = identity;
        for (std::size_t i = 0; i < degree; ++i) {
            const auto edge = tree[v][i];
            if (edge.to != parent[v]) {
                State without = finalize(merge(prefix,suffix[i+1]),v);
                incoming[edge.to][parent_slot[edge.to]] = transfer(without,v,edge.to,edge.id);
            }
            prefix = merge(prefix,incoming[v][i]);
        }
    }
    return result;
}
