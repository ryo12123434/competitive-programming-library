#pragma once

#include "euler_tour.hpp"
#include <vector>

/**
 * @brief DSU on Tree（sack 法）で各部分木の情報をコールバックへ渡す。反復処理で深い木も扱う。
 * @tparam Add void add(int vertex)。頂点の情報を作業状態に追加する。
 * @tparam Remove void remove(int vertex)。追加の逆操作。状態は頂点集合だけに依存し、順序に依存しないこと。
 * @tparam Answer void answer(int vertex)。状態が vertex の部分木全体のとき結果を保存する。
 * @param tree 両方向に辺を格納した連結な無向木。空なら何もしない。
 * @param add 頂点追加。@param remove 頂点削除。@param answer 回答保存。@param root 根（既定0）。
 * @pre 非空なら N<=INT_MAX、頂点・root は [0,N)。初期作業状態は空、answer は状態を変更しないこと。
 * @note 終了時にも作業状態を空に戻す。コールバックの呼び出し順序自体は保証しない。
 * @par Complexity
 * O(N log(N+1)) 回の add/remove、N 回の answer。準備・保持空間 O(N)。再帰なし。
 * コールバックが O(1) なら全体 O(N log(N+1))。
 * @code
 * #include "library/graph/dsu_on_tree.hpp"
 * #include <cassert>
 * int main() {
 *     std::vector<std::vector<int>> tree{{1,2},{0},{0}};
 *     std::vector<int> color{1,1,2}, count(3), result(3); int distinct=0;
 *     dsu_on_tree(tree, [&](int v){ if(count[color[v]]++==0)++distinct; },
 *         [&](int v){ if(--count[color[v]]==0)--distinct; }, [&](int v){ result[v]=distinct; });
 *     assert(result[0]==2 && distinct==0);
 * }
 * @endcode
 */
template <class Add, class Remove, class Answer>
void dsu_on_tree(const std::vector<std::vector<int>>& tree, Add add, Remove remove, Answer answer, int root = 0) {
    if (tree.empty()) return;
    EulerTour tour(tree, root);
    std::vector<int> heavy(tree.size(), -1);
    for (int v : tour.order()) for (int to : tree[v]) {
        if (tour.parent(to) == v && (heavy[v] == -1 || tour.subtree_size(heavy[v]) < tour.subtree_size(to))) heavy[v] = to;
    }
    struct Frame { int v; bool keep, finish; };
    std::vector<Frame> stack{{root, false, false}};
    while (!stack.empty()) {
        auto [v, keep, finish] = stack.back(); stack.pop_back();
        if (!finish) {
            stack.push_back({v, keep, true});
            if (heavy[v] != -1) stack.push_back({heavy[v], true, false});
            for (int to : tree[v]) if (tour.parent(to) == v && to != heavy[v]) stack.push_back({to, false, false});
        } else {
            for (int to : tree[v]) if (tour.parent(to) == v && to != heavy[v]) {
                for (int i = tour.tin(to); i < tour.tout(to); ++i) add(tour.vertex(i));
            }
            add(v); answer(v);
            if (!keep) for (int i = tour.tin(v); i < tour.tout(v); ++i) remove(tour.vertex(i));
        }
    }
}
