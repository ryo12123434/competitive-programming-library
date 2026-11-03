#include <algorithm>
#include <bit>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>
#include <atcoder/modint>
#include <atcoder/segtree>
#include "library/graph/hld.hpp"
#include "library/data_structure/li_chao_tree.hpp"

namespace {
void require(bool value) { if (!value) std::abort(); }
using Tree = std::vector<std::vector<int>>;
using Mint = atcoder::modint998244353;
struct Affine { Mint a, b; };
Affine combine(Affine f, Affine g) { return {g.a * f.a, g.a * f.b + g.b}; }
struct Node { Affine forward, backward; long long sum; };
Node op(Node l, Node r) {
    return {combine(l.forward, r.forward), combine(r.backward, l.backward), l.sum + r.sum};
}
Node e() { return {{1, 0}, {1, 0}, 0}; }
std::mt19937_64 rng(20261025);

// HLD の探索と独立に、各始点から BFS して経路を復元する。
std::vector<int> bfs_parent(const Tree& tree, int start) {
    std::vector<int> parent(tree.size(), -2);
    parent[start] = -1;
    std::queue<int> queue;
    queue.push(start);
    while (!queue.empty()) {
        const int v = queue.front(); queue.pop();
        for (int to : tree[v]) if (parent[to] == -2) {
            parent[to] = v;
            queue.push(to);
        }
    }
    return parent;
}

void check_tree(const Tree& tree, int root) {
    const int n = static_cast<int>(tree.size());
    const HLD hld(tree, root);
    const auto parent = bfs_parent(tree, root);
    std::vector<int> depth(n), size(n), seen(n);
    for (int v = 0; v < n; ++v) {
        for (int u = v; parent[u] != -1; u = parent[u]) ++depth[v];
        for (int u = v; u != -1; u = parent[u]) ++size[u];
    }
    require(hld.size() == n && hld.tin(root) == 0);
    for (int v = 0; v < n; ++v) {
        require(hld.parent(v) == parent[v] && hld.depth(v) == depth[v]);
        require(hld.subtree_size(v) == size[v]);
        require(0 <= hld.tin(v) && hld.tin(v) < n && !seen[hld.tin(v)]++);
        require(hld.vertex(hld.tin(v)) == v && hld.order()[hld.tin(v)] == v);
        const int heavy = hld.heavy_child(v);
        int expected_heavy = -1;
        for (int to : tree[v]) if (parent[to] == v) {
            if (expected_heavy == -1 || size[expected_heavy] < size[to]) expected_heavy = to;
            if (to != heavy) require(2 * size[to] < size[v]);
        }
        require(heavy == expected_heavy);
        if (heavy != -1) require(hld.head(heavy) == hld.head(v) && hld.tin(heavy) == hld.tin(v) + 1);
        if (parent[v] == -1 || hld.heavy_child(parent[v]) != v) require(hld.head(v) == v);
        for (bool edge : {false, true}) {
            const auto [l, r] = hld.subtree_range(v, edge);
            require(l == hld.tin(v) + edge && r == hld.tout(v) && r - l == size[v] - edge);
            for (int u = 0; u < n; ++u) {
                int p = u;
                while (p != -1 && p != v) p = parent[p];
                require((p == v && (!edge || u != v)) == (l <= hld.tin(u) && hld.tin(u) < r));
            }
        }
    }
    std::vector<Affine> functions(n);
    std::vector<long long> values(n);
    atcoder::segtree<Node, op, e> seg(n);
    auto update = [&](int v) {
        // a==0 も含め、一般に逆関数を持たない写像を用いる。
        functions[v] = {rng() % 5, rng() % 11};
        values[v] = static_cast<int>(rng() % 41) - 20;
        seg.set(hld.tin(v), {functions[v], functions[v], values[v]});
    };
    for (int v = 0; v < n; ++v) update(v);
    for (int u = 0; u < n; ++u) {
        update(u); // 頂点・辺の値の変更も区間データ構造へ反映する。
        const auto path_parent = bfs_parent(tree, u);
        for (int v = 0; v < n; ++v) {
            std::vector<int> path;
            for (int w = v; w != -1; w = path_parent[w]) path.push_back(w);
            std::reverse(path.begin(), path.end());
            int a = u, b = v;
            while (depth[a] > depth[b]) a = parent[a];
            while (depth[b] > depth[a]) b = parent[b];
            while (a != b) { a = parent[a]; b = parent[b]; }
            require(hld.lca(u, v) == a);
            for (bool edge : {false, true}) {
                std::vector<int> expected;
                if (!edge) expected = path;
                else for (std::size_t i = 1; i < path.size(); ++i) {
                    const int x = path[i - 1], y = path[i];
                    expected.push_back(depth[x] > depth[y] ? x : y);
                }
                const auto parts = hld.path_segments(u, v, edge);
                require(parts.size() <= 2U * static_cast<std::size_t>(std::bit_width(static_cast<unsigned>(n))));
                std::vector<int> actual;
                long long sum = 0, expected_sum = 0;
                Affine result{1, 0};
                for (auto part : parts) {
                    require(0 <= part.l && part.l < part.r && part.r <= n);
                    const Node node = seg.prod(part.l, part.r);
                    result = combine(result, part.reversed ? node.backward : node.forward);
                    sum += node.sum;
                    if (part.reversed) for (int i = part.r; i-- > part.l;) actual.push_back(hld.vertex(i));
                    else for (int i = part.l; i < part.r; ++i) actual.push_back(hld.vertex(i));
                }
                require(actual == expected);
                Mint value = 7;
                for (int w : expected) {
                    value = functions[w].a * value + functions[w].b;
                    expected_sum += values[w];
                }
                require(result.a * 7 + result.b == value && sum == expected_sum);
                value = 0;
                for (int w : expected) value = functions[w].a * value + functions[w].b;
                require(result.b == value);
            }
        }
    }
}

Tree from_prufer(int n, int code) {
    Tree tree(n);
    if (n == 1) return tree;
    std::vector<int> sequence(n - 2), degree(n, 1);
    for (int& v : sequence) { v = code % n; code /= n; ++degree[v]; }
    auto connect = [&](int a, int b) { tree[a].push_back(b); tree[b].push_back(a); };
    for (int v : sequence) {
        int leaf = 0;
        while (degree[leaf] != 1) ++leaf;
        connect(leaf, v); --degree[leaf]; --degree[v];
    }
    int a = 0, b = n - 1;
    while (degree[a] != 1) ++a;
    while (degree[b] != 1) --b;
    connect(a, b);
    return tree;
}

void hld_tests() {
    for (int n = 1; n <= 5; ++n) {
        int count = 1;
        for (int i = 0; i < n - 2; ++i) count *= n;
        for (int code = 0; code < count; ++code) {
            const Tree tree = from_prufer(n, code);
            for (int root = 0; root < n; ++root) check_tree(tree, root);
        }
    }
    for (int trial = 0; trial < 150; ++trial) {
        const int n = 1 + rng() % 40;
        Tree tree(n);
        for (int v = 1; v < n; ++v) {
            const int p = trial % 3 == 0 ? v - 1 : trial % 3 == 1 ? 0 : rng() % v;
            tree[p].push_back(v); tree[v].push_back(p);
        }
        for (auto& row : tree) std::shuffle(row.begin(), row.end(), rng);
        const auto original = tree;
        check_tree(tree, rng() % n);
        require(tree == original);
    }
    const int n = 200000;
    for (int shape = 0; shape < 3; ++shape) {
        Tree tree(n);
        for (int v = 1; v < n; ++v) {
            const int p = shape == 0 ? v - 1 : shape == 1 ? 0 : (v - 1) / 2;
            tree[p].push_back(v); tree[v].push_back(p);
        }
        const HLD hld(tree, shape == 0 ? n - 1 : 0);
        require(hld.subtree_size(shape == 0 ? n - 1 : 0) == n);
        for (int i = 0; i < 1000; ++i) {
            const int u = rng() % n, v = rng() % n;
            const auto parts = hld.path_segments(u, v);
            require(parts.size() <= 2U * static_cast<std::size_t>(std::bit_width(static_cast<unsigned>(n))));
            if (shape == 0) require(parts.size() == 1 && hld.lca(u, v) == std::max(u, v));
        }
    }
}

void print_value(LiChaoTree::Value value) {
    if (value == 0) { std::cout << '0'; return; }
    const bool negative = value < 0;
    unsigned __int128 magnitude = static_cast<unsigned __int128>(value);
    if (negative) magnitude = -magnitude;
    std::string digits;
    while (magnitude != 0) { digits.push_back('0' + magnitude % 10); magnitude /= 10; }
    if (negative) std::cout << '-';
    std::reverse(digits.begin(), digits.end());
    std::cout << digits;
}

void li_chao_oracle() {
    int n, q;
    std::cin >> n >> q;
    std::vector<long long> xs(n);
    for (auto& x : xs) std::cin >> x;
    LiChaoTree tree(xs);
    const auto original = xs;
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    require(tree.size() == static_cast<int>(xs.size()));
    while (q--) {
        char type;
        long long a, b;
        std::cin >> type >> a;
        if (type == 'a') { std::cin >> b; tree.add_line(a, b); }
        else {
            const auto answer = tree.query(a);
            if (answer) print_value(*answer);
            else std::cout << "empty";
            std::cout << '\n';
        }
    }
    require(original.size() == static_cast<std::size_t>(n));
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 1) { hld_tests(); return 0; }
    const std::string mode = argv[1];
    if (mode == "li") li_chao_oracle();
    else if (mode == "invalid-coordinate") { LiChaoTree tree({0, 2}); (void)tree.query(1); }
    else if (mode == "empty-tree") { (void)HLD(Tree{}); }
    else if (mode == "disconnected") { (void)HLD(Tree(2)); }
    else if (mode == "cycle") { (void)HLD(Tree{{1, 2}, {0, 2}, {0, 1}}); }
    else if (mode == "invalid-root") { (void)HLD(Tree(1), 1); }
}
