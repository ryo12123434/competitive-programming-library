#pragma once

#include "lca.hpp"
#include "euler_tour.hpp"
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

/**
 * @brief 仮想木の結果。vertices[i] は元の頂点番号、parent[i] は仮想木内の親の添字。
 * @note 根は先頭で parent=-1・distance=0。他は親への元の木の距離（辺数）を distance に持つ。
 * 頂点は DFS 順、元の指定点と必要な LCA の和集合。空集合なら全配列が空。
 * @code
 * #include "library/graph/virtual_tree.hpp"
 * int main() { VirtualTreeResult result; (void)result; }
 * @endcode
 */
struct VirtualTreeResult { std::vector<int> vertices, parent, distance; };

/**
 * @brief 指定頂点集合と必要な LCA だけを含む仮想木を構築する。
 * @note 既存 LCA と EulerTour を所有するので元の隣接リストの寿命に依存しない。
 * 元の木の重みなし距離を扱う。指定頂点の重複は除く。
 * @par Complexity
 * 前処理 O(N log N) 時間・空間。build は O(K log K+K log N) 時間、追加空間 O(K)。
 * 二分累乗 LCA の O(log N) を含む。返り値の頂点数は K>0 なら高々 2K-1（重複除去後）。
 * @code
 * #include "library/graph/virtual_tree.hpp"
 * #include <cassert>
 * int main() {
 *     VirtualTree tree(std::vector<std::vector<int>>{{1,2},{0},{0}});
 *     auto result=tree.build({1,2});
 *     assert((result.vertices==std::vector<int>{0,1,2}));
 *     assert(result.parent[2]==0 && result.distance[2]==1);
 * }
 * @endcode
 */
class VirtualTree {
public:
    /// @brief 無向木を root で根付けして前処理。@pre 0<N<=INT_MAX、連結な木、root は [0,N)。
    explicit VirtualTree(const std::vector<std::vector<int>>& tree, int root = 0) : lca_(tree, root), tour_(tree, root) {}
    /// @brief 指定頂点とその LCA から仮想木を作る。@pre 各頂点は元の木の [0,N)。
    VirtualTreeResult build(std::vector<int> vertices) const {
        for ([[maybe_unused]] int v : vertices) assert(0 <= v && v < tour_.size());
        auto less = [&](int a, int b) { return tour_.tin(a) < tour_.tin(b); };
        std::sort(vertices.begin(), vertices.end(), less);
        vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
        const auto k = vertices.size();
        for (std::size_t i = 1; i < k; ++i) vertices.push_back(lca_.query(vertices[i - 1], vertices[i]));
        std::sort(vertices.begin(), vertices.end(), less);
        vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
        VirtualTreeResult result; result.vertices = std::move(vertices);
        result.parent.assign(result.vertices.size(), -1); result.distance.assign(result.vertices.size(), 0);
        std::vector<int> stack;
        for (std::size_t i = 0; i < result.vertices.size(); ++i) {
            int v = result.vertices[i];
            while (!stack.empty() && tour_.tout(result.vertices[stack.back()]) <= tour_.tin(v)) stack.pop_back();
            if (!stack.empty()) {
                result.parent[i] = stack.back();
                result.distance[i] = lca_.depth(v) - lca_.depth(result.vertices[stack.back()]);
            }
            stack.push_back(static_cast<int>(i));
        }
        return result;
    }
private:
    LCA lca_;
    EulerTour tour_;
};
