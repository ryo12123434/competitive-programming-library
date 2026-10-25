#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"

#include <atcoder/modint>
#include <atcoder/segtree>
#include <iostream>
#include <vector>
#include "../../library/graph/hld.hpp"

using Mint = atcoder::modint998244353;
struct Affine { Mint a, b; };
// f の後に g を作用させる。経路順の結合。
Affine combine(Affine f, Affine g) { return {g.a * f.a, g.a * f.b + g.b}; }
struct Node { Affine forward, backward; };
Node op(Node l, Node r) { return {combine(l.forward, r.forward), combine(r.backward, l.backward)}; }
Node e() { return {{1, 0}, {1, 0}}; }

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<Affine> functions(n);
    for (auto& f : functions) {
        int a, b;
        std::cin >> a >> b;
        f = {a, b};
    }
    std::vector<std::vector<int>> tree(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        std::cin >> u >> v;
        tree[u].push_back(v); tree[v].push_back(u);
    }
    const HLD hld(tree);
    std::vector<Node> ordered(n);
    for (int v = 0; v < n; ++v) ordered[hld.tin(v)] = {functions[v], functions[v]};
    atcoder::segtree<Node, op, e> seg(ordered);
    while (q--) {
        int type;
        std::cin >> type;
        if (type == 0) {
            int v, a, b;
            std::cin >> v >> a >> b;
            seg.set(hld.tin(v), {{a, b}, {a, b}});
        } else {
            int u, v, x;
            std::cin >> u >> v >> x;
            Affine result{1, 0};
            for (auto part : hld.path_segments(u, v)) {
                const Node node = seg.prod(part.l, part.r);
                result = combine(result, part.reversed ? node.backward : node.forward);
            }
            std::cout << (result.a * x + result.b).val() << '\n';
        }
    }
}
