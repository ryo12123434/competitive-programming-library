#include <algorithm>
#include <climits>
#include <cstdlib>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>
#include <atcoder/modint>

#include "library/graph/functional_graph.hpp"
#include "library/graph/euler_tour.hpp"
#include "library/graph/tree_diameter.hpp"
#include "library/data_structure/prefix_sum_2d.hpp"
#include "library/math/matrix.hpp"

namespace {
void require(bool value) { if (!value) std::abort(); }
std::mt19937_64 rng(20261025);
using Mint = atcoder::modint998244353;

void check_functional(const std::vector<int>& next) {
    const int n = static_cast<int>(next.size());
    const FunctionalGraph graph(next);
    require(graph.size() == n);
    std::set<std::vector<int>> cycles;
    for (int start = 0; start < n; ++start) {
        std::vector<int> position(n, -1), path;
        int v = start;
        while (position[v] == -1) {
            position[v] = static_cast<int>(path.size());
            path.push_back(v);
            v = next[v];
        }
        std::vector<int> cycle(path.begin() + position[v], path.end());
        std::rotate(cycle.begin(), std::min_element(cycle.begin(), cycle.end()), cycle.end());
        cycles.insert(cycle);
        require(graph.on_cycle(start) == (position[v] == 0));
        require(graph.distance_to_cycle(start) == position[v]);
        require(graph.cycle_entry(start) == v);
        require(graph.reachable_count(start) == static_cast<int>(path.size()));
        const int id = graph.cycle_id(start);
        require(0 <= id && id < static_cast<int>(graph.cycles().size()));
        require(graph.cycles()[id] == cycle);
    }
    require(graph.cycles() == std::vector<std::vector<int>>(cycles.begin(), cycles.end()));
}

void functional_tests() {
    check_functional({});
    // N <= 6 の全 N^N 個の遷移を列挙。各始点からの直接シミュレーションと比較する。
    for (int n = 1; n <= 6; ++n) {
        int count = 1;
        for (int i = 0; i < n; ++i) count *= n;
        for (int code = 0; code < count; ++code) {
            int rest = code;
            std::vector<int> next(n);
            for (int& to : next) { to = rest % n; rest /= n; }
            check_functional(next);
        }
    }
    for (int trial = 0; trial < 300; ++trial) {
        const int n = 1 + rng() % 100;
        std::vector<int> next(n);
        for (int& to : next) to = rng() % n;
        check_functional(next);
    }
    const int n = 200000;
    std::vector<int> next(n);
    std::iota(next.begin(), next.end(), 1);
    next.back() = n - 1;
    const FunctionalGraph chain(next);
    require(chain.distance_to_cycle(0) == n - 1 && chain.reachable_count(0) == n);
    require(chain.cycle_entry(0) == n - 1);
    next.back() = 0;
    const FunctionalGraph ring(next);
    require(ring.cycles().size() == 1 && ring.cycles()[0].size() == next.size());
    std::iota(next.begin(), next.end(), 0);
    const FunctionalGraph loops(next);
    require(loops.cycles().size() == next.size() && loops.reachable_count(0) == 1);
}

using Tree = std::vector<std::vector<int>>;

Tree from_prufer(int n, int code) {
    Tree tree(n);
    if (n == 1) return tree;
    std::vector<int> sequence(n - 2), degree(n, 1);
    for (int& v : sequence) { v = code % n; code /= n; ++degree[v]; }
    auto connect = [&](int a, int b) { tree[a].push_back(b); tree[b].push_back(a); };
    for (int v : sequence) {
        int leaf = 0;
        while (degree[leaf] != 1) ++leaf;
        connect(leaf, v);
        --degree[leaf]; --degree[v];
    }
    int a = 0, b = n - 1;
    while (degree[a] != 1) ++a;
    while (degree[b] != 1) --b;
    connect(a, b);
    return tree;
}

void check_euler(const Tree& tree, int root) {
    const int n = static_cast<int>(tree.size());
    const EulerTour tour(tree, root);
    std::vector<int> parent(n, -1), depth(n), size(n, 1), order;
    // 小さい木だけ再帰で独立に求める。深い木は別途非再帰の性質テストを行う。
    std::function<void(int, int)> visit = [&](int v, int p) {
        order.push_back(v);
        parent[v] = p;
        for (int to : tree[v]) if (to != p) {
            depth[to] = depth[v] + 1;
            visit(to, v);
            size[v] += size[to];
        }
    };
    visit(root, -1);
    require(tour.size() == n && tour.order() == order);
    for (int v = 0; v < n; ++v) {
        require(tour.parent(v) == parent[v] && tour.depth(v) == depth[v]);
        require(tour.subtree_size(v) == size[v] && tour.vertex(tour.tin(v)) == v);
        const auto [l, r] = tour.subtree_range(v);
        require(l == tour.tin(v) && r == tour.tout(v) && r - l == size[v]);
        for (int u = 0; u < n; ++u) {
            int ancestor = u;
            while (ancestor != -1 && ancestor != v) ancestor = parent[ancestor];
            require((ancestor == v) == (l <= tour.tin(u) && tour.tin(u) < r));
        }
    }
}

void check_diameter(const Tree& tree, int mode) {
    using Diameter = TreeDiameter<long long>;
    const int n = static_cast<int>(tree.size());
    std::vector<std::vector<Diameter::Edge>> weighted(n);
    constexpr long long inf = 1LL << 60;
    std::vector<std::vector<long long>> distance(n, std::vector<long long>(n, inf));
    for (int v = 0; v < n; ++v) {
        distance[v][v] = 0;
        for (int to : tree[v]) if (v < to) {
            const long long cost = mode == 0 ? 0 : mode == 1 ? 1 : rng() % 100;
            weighted[v].push_back({to, cost});
            weighted[to].push_back({v, cost});
            distance[v][to] = distance[to][v] = cost;
        }
    }
    const auto edge_cost = distance;
    // 二回探索とは独立した Floyd-Warshall による全点対比較。
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                distance[i][j] = std::min(distance[i][j], distance[i][k] + distance[k][j]);
    long long expected = 0;
    for (const auto& row : distance) for (long long d : row) expected = std::max(expected, d);
    const Diameter diameter(weighted);
    const auto& path = diameter.get_path();
    require(diameter.length() == expected && !path.empty());
    require(diameter.endpoints() == std::pair(path.front(), path.back()));
    std::set<int> seen;
    long long length = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        require(0 <= path[i] && path[i] < n && seen.insert(path[i]).second);
        if (i != 0) {
            require(edge_cost[path[i - 1]][path[i]] != inf);
            length += edge_cost[path[i - 1]][path[i]];
        }
    }
    require(length == expected);
    if (mode == 1) {
        const TreeDiameter unweighted(tree);
        require(unweighted.length() == expected);
        require(unweighted.get_path().size() == static_cast<std::size_t>(expected + 1));
    }
}

void tree_tests() {
    for (int n = 1; n <= 5; ++n) {
        int count = 1;
        for (int i = 0; i < n - 2; ++i) count *= n;
        for (int code = 0; code < count; ++code) {
            Tree tree = from_prufer(n, code);
            for (int root = 0; root < n; ++root) check_euler(tree, root);
            for (int mode = 0; mode < 3; ++mode) check_diameter(tree, mode);
        }
    }
    for (int trial = 0; trial < 400; ++trial) {
        const int n = 1 + rng() % 35;
        Tree tree(n);
        for (int v = 1; v < n; ++v) {
            const int p = rng() % v;
            tree[p].push_back(v); tree[v].push_back(p);
        }
        for (auto& row : tree) std::shuffle(row.begin(), row.end(), rng);
        check_euler(tree, rng() % n);
        check_diameter(tree, trial % 3);
    }
    const int n = 200000;
    Tree chain(n), star(n);
    for (int v = 1; v < n; ++v) {
        chain[v - 1].push_back(v); chain[v].push_back(v - 1);
        star[0].push_back(v); star[v].push_back(0);
    }
    const EulerTour tour(chain, n - 1), star_tour(star);
    require(tour.depth(0) == n - 1 && tour.parent(0) == 1 && tour.tout(0) == n);
    require(tour.subtree_size(n - 1) == n && star_tour.subtree_size(0) == n);
    const TreeDiameter diameter(chain), star_diameter(star);
    require(diameter.length() == n - 1 && diameter.get_path().size() == chain.size());
    require(star_diameter.length() == 2);
    using Diameter = TreeDiameter<long long>;
    const Diameter large(std::vector<std::vector<Diameter::Edge>>{{{1, LLONG_MAX}}, {{0, LLONG_MAX}}});
    require(large.length() == LLONG_MAX);
}

struct AdditiveOnly {
    long long value{};
    AdditiveOnly& operator+=(const AdditiveOnly& rhs) { value += rhs.value; return *this; }
    AdditiveOnly& operator-=(const AdditiveOnly& rhs) { value -= rhs.value; return *this; }
};

void check_prefix(const std::vector<std::vector<long long>>& values) {
    const int h = static_cast<int>(values.size()), w = h == 0 ? 0 : static_cast<int>(values[0].size());
    std::vector<std::vector<Mint>> modular(h, std::vector<Mint>(w));
    std::vector<std::vector<AdditiveOnly>> additive(h, std::vector<AdditiveOnly>(w));
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        modular[y][x] = values[y][x]; additive[y][x].value = values[y][x];
    }
    const PrefixSum2D<long long> sum(values);
    const PrefixSum2D<Mint> mod_sum(modular);
    const PrefixSum2D<AdditiveOnly> add_sum(additive);
    require(sum.height() == h && sum.width() == w);
    for (int u = 0; u <= h; ++u) for (int d = u; d <= h; ++d)
        for (int l = 0; l <= w; ++l) for (int r = l; r <= w; ++r) {
            long long expected = 0;
            for (int y = u; y < d; ++y) for (int x = l; x < r; ++x) expected += values[y][x];
            require(sum.query(u, d, l, r) == expected);
            require(mod_sum.query(u, d, l, r) == Mint(expected));
            require(add_sum.query(u, d, l, r).value == expected);
        }
}

void prefix_tests() {
    check_prefix({});
    check_prefix({{LLONG_MAX}});
    for (int code = 0; code < 81; ++code) {
        auto values = std::vector(2, std::vector<long long>(2));
        int rest = code;
        for (auto& row : values) for (auto& x : row) { x = rest % 3 - 1; rest /= 3; }
        check_prefix(values);
    }
    for (int trial = 0; trial < 300; ++trial) {
        const int h = rng() % 7, w = rng() % 7;
        auto values = std::vector(h, std::vector<long long>(w));
        for (auto& row : values) for (auto& x : row) x = static_cast<int>(rng() % 41) - 20;
        check_prefix(values);
        const PrefixSum2D<long long> sum(values);
        if (h != 0 && w != 0) {
            const long long before = sum.query(0, h, 0, w);
            ++values[0][0];
            require(sum.query(0, h, 0, w) == before);
        }
    }
}

void matrix_tests() {
    const Matrix<long long> empty;
    require(empty.height() == 0 && empty.width() == 0 && empty.pow(0).height() == 0);
    const Matrix<int> empty_wide(0, INT_MAX);
    require(empty_wide.height() == 0 && empty_wide.width() == INT_MAX);
    for (int h = 0; h <= 4; ++h) for (int w = 0; w <= 4; ++w) for (int k = 0; k <= 4; ++k)
        for (int trial = 0; trial < 10; ++trial) {
            Matrix<long long> a(h, w), b(w, k);
            for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) a(y, x) = static_cast<int>(rng() % 11) - 5;
            for (int y = 0; y < w; ++y) for (int x = 0; x < k; ++x) b(y, x) = static_cast<int>(rng() % 11) - 5;
            const auto product = a * b;
            require(product.height() == h && product.width() == k);
            for (int y = 0; y < h; ++y) for (int x = 0; x < k; ++x) {
                long long expected = 0;
                for (int i = 0; i < w; ++i) expected += a(y, i) * b(i, x);
                require(product(y, x) == expected);
            }
        }
    // 全 2x2 三値行列について、歩数を 1 ずつ進めるベクトル DP と二分累乗を比較。
    for (int code = 0; code < 81; ++code) {
        Matrix<long long> a(2, 2);
        int rest = code;
        for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) { a(y, x) = rest % 3 - 1; rest /= 3; }
        for (int steps = 0; steps <= 8; ++steps) {
            const auto power = a.pow(steps);
            for (int start = 0; start < 2; ++start) {
                std::vector<long long> state(2);
                state[start] = 1;
                for (int t = 0; t < steps; ++t) {
                    std::vector<long long> next(2);
                    for (int to = 0; to < 2; ++to)
                        for (int from = 0; from < 2; ++from) next[to] += state[from] * a(from, to);
                    state = next;
                }
                for (int end = 0; end < 2; ++end) require(power(start, end) == state[end]);
            }
        }
    }
    for (int trial = 0; trial < 200; ++trial) {
        const int n = 1 + rng() % 5, steps = rng() % 15;
        Matrix<Mint> a(n, n);
        for (int y = 0; y < n; ++y) for (int x = 0; x < n; ++x) a(y, x) = rng();
        const auto power = a.pow(steps);
        for (int start = 0; start < n; ++start) {
            std::vector<Mint> state(n);
            state[start] = 1;
            for (int t = 0; t < steps; ++t) {
                std::vector<Mint> next(n);
                for (int to = 0; to < n; ++to)
                    for (int from = 0; from < n; ++from) next[to] += state[from] * a(from, to);
                state = next;
            }
            for (int end = 0; end < n; ++end) require(power(start, end) == state[end]);
        }
    }
    const Matrix<long long> single(std::vector<std::vector<long long>>{{LLONG_MAX}});
    require(single.pow(1)(0, 0) == LLONG_MAX && single.pow(0)(0, 0) == 1);
    Matrix<Mint> scalar(std::vector<std::vector<Mint>>{{2}});
    require(scalar.pow(ULLONG_MAX)(0, 0) == Mint(2).pow(LLONG_MAX) * Mint(2).pow(LLONG_MAX) * 2);
    auto source = std::vector<std::vector<long long>>{{1, 2}, {3, 4}};
    const Matrix<long long> copied(source);
    source[0][0] = 99;
    require(copied(0, 0) == 1 && copied.height() == 2 && copied.width() == 2);
}
} // namespace

void run_task_a_regressions() {
    functional_tests();
    tree_tests();
    prefix_tests();
    matrix_tests();
}

#ifndef CPL_TASK_A_NO_MAIN
int main(int argc, char** argv) {
    if (argc == 1) { run_task_a_regressions(); return 0; }
    const std::string mode = argv[1];
    if (mode == "functional") { functional_tests(); return 0; }
    if (mode == "tree") { tree_tests(); return 0; }
    if (mode == "prefix") { prefix_tests(); return 0; }
    if (mode == "matrix") { matrix_tests(); return 0; }
    if (mode == "matrix-product") { (void)(Matrix<int>(2, 3) * Matrix<int>(2, 4)); }
    if (mode == "matrix-power") { (void)Matrix<int>(2, 3).pow(0); }
    if (mode == "matrix-ragged") { (void)Matrix<int>(std::vector<std::vector<int>>{{1}, {2, 3}}); }
    if (mode == "matrix-negative") { (void)Matrix<int>(-1, 2); }
    if (mode == "prefix-ragged") { (void)PrefixSum2D<int>(std::vector<std::vector<int>>{{1}, {2, 3}}); }
    if (mode == "prefix-range") { (void)PrefixSum2D<int>({{1}}).query(0, 2, 0, 1); }
    if (mode == "functional-range") { (void)FunctionalGraph({1}); }
    if (mode == "diameter-negative") {
        (void)TreeDiameter<>(std::vector<std::vector<TreeDiameter<>::Edge>>{{{1, -1}}, {{0, -1}}});
    }
    if (mode == "euler-empty") { (void)EulerTour(Tree{}); }
    if (mode == "euler-disconnected") { (void)EulerTour(Tree(2)); }
    return 0;
}
#endif
