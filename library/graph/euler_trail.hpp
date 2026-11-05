#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

/// @brief オイラー路。vertices は辺数+1要素、edge_ids[i] が vertices[i] -> vertices[i+1]。
struct EulerTrailResult {
    std::vector<int> vertices, edge_ids;
};
/**
 * @brief Hierholzer 法で有向・無向オイラー路（閉路を含む）を求める。
 * @param n 頂点数。@param edges ID順の辺。無向辺も一度だけ入力する。
 * @param directed true は有向。@param start 指定すると、その頂点から出発できる場合だけ成功する。
 * @return 存在しないなら nullopt。辺なしなら指定点または0の1頂点。n=0なら空の経路。
 * @pre n>=0、辺数は INT_MAX 以下、端点・指定 start は [0,n)。n=0では start を指定しない。
 * @note 孤立点は無視するが、辺を持つ成分が分離していると失敗。自己ループ・平行辺も各IDを1回使う。
 * @par Complexity O(V+E) 時間・追加空間。再帰なし。
 * @code
 * #include "library/graph/euler_trail.hpp"
 * #include <cassert>
 * int main(){auto p=euler_trail(2,{{0,1},{1,0}},true);assert(p&&p->edge_ids.size()==2&&p->vertices.front()==p->vertices.back());}
 * @endcode
 */
inline std::optional<EulerTrailResult> euler_trail(int n, const std::vector<std::pair<int, int>> &edges,
                                                   bool directed = false,
                                                   std::optional<int> start = std::nullopt) {
    assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    assert(!start || (0 <= *start && *start < n));
    std::vector<std::vector<std::pair<int, int>>> graph(n);
    std::vector<long long> in(n), out(n);
    for (std::size_t id = 0; id < edges.size(); ++id) {
        auto [u, v] = edges[id];
        assert(0 <= u && u < n && 0 <= v && v < n);
        graph[u].push_back({v, static_cast<int>(id)});
        ++out[u];
        ++in[v];
        if (!directed) {
            graph[v].push_back({u, static_cast<int>(id)});
            ++out[v];
            ++in[u];
        }
    }
    if (edges.empty()) {
        EulerTrailResult result;
        if (n)
            result.vertices.push_back(start.value_or(0));
        return result;
    }
    int required = -1, odd = 0, positive = 0, negative = 0, first = -1;
    for (int v = 0; v < n; ++v) {
        if (out[v] && first == -1)
            first = v;
        if (directed) {
            auto diff = out[v] - in[v];
            if (diff == 1) {
                ++positive;
                required = v;
            } else if (diff == -1)
                ++negative;
            else if (diff != 0)
                return std::nullopt;
        } else if (out[v] % 2) {
            ++odd;
            if (required == -1)
                required = v;
        }
    }
    if (directed ? !((positive == 0 && negative == 0) || (positive == 1 && negative == 1))
                 : !(odd == 0 || odd == 2))
        return std::nullopt;
    int source = start.value_or(required != -1 ? required : first);
    if (directed && required != -1 && source != required)
        return std::nullopt;
    if (!directed && odd == 2 && out[source] % 2 == 0)
        return std::nullopt;
    if (!out[source])
        return std::nullopt;
    std::vector<unsigned char> used(edges.size());
    std::vector<std::size_t> next(n);
    std::vector<int> vertices{source}, incoming{-1};
    EulerTrailResult result;
    while (!vertices.empty()) {
        int v = vertices.back();
        auto &index = next[v];
        while (index < graph[v].size() && used[graph[v][index].second])
            ++index;
        if (index == graph[v].size()) {
            result.vertices.push_back(v);
            if (incoming.back() != -1)
                result.edge_ids.push_back(incoming.back());
            vertices.pop_back();
            incoming.pop_back();
        } else {
            auto [to, id] = graph[v][index++];
            used[id] = 1;
            vertices.push_back(to);
            incoming.push_back(id);
        }
    }
    if (result.edge_ids.size() != edges.size())
        return std::nullopt;
    std::reverse(result.vertices.begin(), result.vertices.end());
    std::reverse(result.edge_ids.begin(), result.edge_ids.end());
    return result;
}
