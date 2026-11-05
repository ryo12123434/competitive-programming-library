#pragma once
#include "biconnected_components.hpp"
#include <vector>

/**
 * @brief 二重頂点連結成分と関節点のBlock-Cut森。非連結・孤立点を許す。
 * @param b build済みのBiconnectedComponents。自己ループの所属規約も引き継ぐ。
 * @pre 結果のブロック数と関節点数の和はintに収まること。
 * @note [0,block_count)はブロック、後続は関節点。vertex_node[v]は元頂点の対応ノード。
 * 非関節点は所属ブロック、関節点は専用ノードに対応する。articulation_vertex[node]は元の関節点、ブロックなら-1。
 * @par Complexity O(V+E)時間・空間。元グラフが木でも通常の頂点隣接リストとはノードが異なる。
 * @code
 * #include "library/graph/block_cut_tree.hpp"
 * #include <cassert>
 * int main(){BiconnectedComponents b(3);b.add_edge(0,1);b.add_edge(1,2);b.build();BlockCutTree t(b);assert(t.block_count==2&&t.tree[t.vertex_node[1]].size()==2);}
 * @endcode
 */
struct BlockCutTree {
    int block_count;
    std::vector<std::vector<int>> tree;
    std::vector<int> vertex_node, articulation_vertex;
    explicit BlockCutTree(const BiconnectedComponents &b)
        : block_count(static_cast<int>(b.components.size())) {
        int n = b.lowlink.V;
        vertex_node.assign(n, -1);
        int total = block_count + static_cast<int>(b.articulation_points.size());
        tree.resize(total);
        articulation_vertex.assign(total, -1);
        for (int i = 0; i < static_cast<int>(b.articulation_points.size()); ++i) {
            int v = b.articulation_points[i], node = block_count + i;
            vertex_node[v] = node;
            articulation_vertex[node] = v;
        }
        for (int id = 0; id < block_count; ++id)
            for (int v : b.components[id]) {
                if (vertex_node[v] >= block_count) {
                    int node = vertex_node[v];
                    tree[id].push_back(node);
                    tree[node].push_back(id);
                } else
                    vertex_node[v] = id;
            }
    }
};
