#include "library/debug/debug.hpp"
#include "library/debug/timer.hpp"
#include "library/debug/random.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <map>
#include <numeric>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using cpl::debug::repr;

void check_display() {
    assert(repr(std::vector<std::vector<std::vector<int>>>{{{1, 2}}, {}}) == "[[[1, 2]], []]");
    assert(repr(std::pair<int, std::vector<int>>{1, {2, 3}}) == "(1, [2, 3])");
    assert(repr(std::tuple<int, bool, std::string>{7, true, "abc"}) == "(7, true, \"abc\")");
    assert(repr(std::tuple<>{}) == "()");
    assert(repr(std::array<int, 3>{1, 2, 3}) == "[1, 2, 3]");
    assert(repr(std::set<int>{3, 1}) == "{1, 3}");
    assert(repr(std::map<int, std::vector<bool>>{{1, {true, false}}}) == "{(1, [true, false])}");
    assert(repr(std::unordered_map<int, int>{{1, 2}}) == "{(1, 2)}");
    assert(repr(std::unordered_set<int>{1}) == "{1}");
    assert(repr("a\nb\t") == "\"a\\nb\\t\"");
    assert(repr('\n') == "'\\n'");
    assert(repr(std::string("\x01\x7f", 2)) == "\"\\x01\\x7f\"");
    assert(repr(static_cast<unsigned char>(255)) == "255");
    char raw[2]{'a', 'b'};
    assert(repr(raw) == "\"ab\"");
    const char* null = nullptr;
    assert(repr(null) == "nullptr");
    struct Unprintable {};
    assert(repr(Unprintable{}) == "<unprintable>");
    const std::vector<int> a{1, 2, 3};
    assert(repr(a, {2, 8}) == "[1, 2, ...]");
    assert(repr(a, {0, 8}) == "[...]");
    assert(repr(std::vector<std::vector<int>>{{1}, {2}}, {20, 1}) == "[..., ...]");
    assert(repr(std::tuple{1, 2}, {1, 8}) == "(1, ...)");
    std::ostringstream captured;
    auto* previous = std::cerr.rdbuf(captured.rdbuf());
    int side = 0;
    debug(a, ++side, std::pair<int, int>{4, 5});
    debug();
    if (true) debug(a); else ++side;
    std::vector<std::vector<int>> dp{{1, 999}, {3}, {4, 5}};
    debug_table(dp);
    { CPL_SCOPED_TIMER("scope"); CPL_SCOPED_TIMER("second"); }
    std::cerr.rdbuf(previous);
#ifdef LOCAL
    assert(side == 1);
    const auto output = captured.str();
    assert(output.find(std::string(__FILE__) + ":") != std::string::npos);
    assert(output.find("a, ++side, std::pair<int, int>{4, 5}") != std::string::npos);
    assert(output.find("([1, 2, 3], 1, (4, 5))") != std::string::npos);
    assert(output.find("r/c") != std::string::npos);
    assert(output.find("scope:") != std::string::npos && output.find(" ms") != std::string::npos);
#else
    assert(side == 0 && captured.str().empty());
    debug(undefined_symbol());
    debug_table(undefined_symbol(), undefined_options());
    CPL_SCOPED_TIMER(undefined_label());
    static_assert(std::is_empty_v<cpl::debug::Timer>);
#endif
    cpl::debug::Timer timer;
#ifdef LOCAL
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    assert(timer.elapsed_ms() >= 1);
    timer.reset();
    assert(timer.elapsed_ms() >= 0);
#else
    assert(timer.elapsed_ms() == 0);
#endif
    cpl::debug::TableOptions view;
    view.row_begin = 1; view.row_end = 2; view.col_end = 1;
    const auto displayed = cpl::debug::render_table(dp, view);
    assert(displayed.find("999") == std::string::npos);
    assert(displayed.find("limited view") != std::string::npos);
    view.row_begin = 10;
    assert(cpl::debug::render_table(dp, view) == "(empty view)\n");
    assert(cpl::debug::render_table(std::vector<std::vector<int>>{}) == "(empty view)\n");
    view = {}; view.max_cols = 1;
    assert(cpl::debug::render_table(dp, view).find("limited view") != std::string::npos);
    view = {}; view.max_cell_width = 1;
    assert(cpl::debug::render_table(dp, view).find("9...") != std::string::npos);
}

int components(int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<int> parent(n);
    std::iota(parent.begin(), parent.end(), 0);
    auto root = [&](int u) { while (u != parent[u]) u = parent[u]; return u; };
    std::set<std::pair<int, int>> seen;
    int count = n;
    for (auto [u, v] : edges) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(seen.insert(std::minmax(u, v)).second);
        u = root(u); v = root(v);
        if (u != v) { parent[u] = v; --count; }
    }
    return count;
}

void check_random() {
    using cpl::random::Random;
    Random a(42), b(42);
    assert(a.seed_value() == 42);
    for (int i = 0; i < 1000; ++i) {
        assert(a.integer(LLONG_MIN, LLONG_MAX) == b.integer(LLONG_MIN, LLONG_MAX));
        const auto x = a.integer(-5, 7);
        assert(-5 <= x && x <= 7);
        assert(x == b.integer(-5, 7));
        const auto real = a.real(-1e308, 1e308);
        assert(std::isfinite(real) && -1e308 <= real && real < 1e308);
        assert(real == b.real(-1e308, 1e308));
        assert(a.array(10, -10, 10) == b.array(10, -10, 10));
        assert(a.permutation(10) == b.permutation(10));
        assert(a.string(10, "ab") == b.string(10, "ab"));
        assert(a.intervals(10, 10) == b.intervals(10, 10));
        assert(a.graph(8, 15, true) == b.graph(8, 15, true));
        assert(a.disconnected_graph(8, 10) == b.disconnected_graph(8, 10));
    }
    a.seed(42); b.seed(42);
    assert(a.integer(0, LLONG_MAX) == b.integer(0, LLONG_MAX));
    // mt19937_64 と独自の区間変換の固定出力も確認する。
    a.seed(0);
    assert(a.integer(LLONG_MIN, LLONG_MAX) == std::bit_cast<long long>(std::mt19937_64(0)() + (1ULL << 63)));
    assert(a.integer(LLONG_MIN, LLONG_MIN) == LLONG_MIN);
    assert(a.integer(LLONG_MAX, LLONG_MAX) == LLONG_MAX);
    assert(a.real(3, 3) == 3);
    const double adjacent = std::nextafter(1.0, 2.0);
    for (int i = 0; i < 100; ++i) assert(a.real(1, adjacent) == 1);
    assert(a.array(0, 0, 0).empty());
    assert(a.permutation(0).empty());
    assert(a.string(0).empty() && a.string(5, "x") == "xxxxx");
    assert((Random::boundaries(-5, 9) == std::vector<long long>{-5, -1, 0, 1, 9}));
    assert((Random::boundaries(3, 3) == std::vector<long long>{3}));
    std::set<long long> boundary_hits;
    for (int i = 0; i < 1000; ++i) boundary_hits.insert(a.edge_integer(-5, 9));
    for (auto x : Random::boundaries(-5, 9)) assert(boundary_hits.contains(x));
    assert((a.interval(0) == std::pair{0, 0}));
    for (int n = 1; n < 30; ++n) {
        for (auto [l, r] : a.intervals(n, 100, true)) assert(0 <= l && l < r && r <= n);
        auto p = a.permutation(n);
        std::sort(p.begin(), p.end());
        for (int i = 0; i < n; ++i) assert(p[i] == i);
        auto tree = a.tree(n);
        assert(tree.size() == static_cast<std::size_t>(n - 1) && components(n, tree) == 1);
        for (int m : {0, n * (n - 1) / 4, n * (n - 1) / 2}) {
            auto edges = a.graph(n, m);
            assert(edges.size() == static_cast<std::size_t>(m));
            components(n, edges);
        }
        for (int m : {n - 1, n * (n - 1) / 2}) {
            auto edges = a.graph(n, m, true);
            assert(edges.size() == static_cast<std::size_t>(m) && components(n, edges) == 1);
        }
        if (n > 1) {
            auto edges = a.disconnected_graph(n, (n - 1) * (n - 2) / 2);
            assert(components(n, edges) > 1);
        }
    }
    assert(a.tree(0).empty() && a.graph(0, 0, true).empty());
    auto sparse = a.graph(1000000000, 3);
    assert(sparse.size() == 3); // 全候補辺や全頂点の配列は確保しない。
    assert(a.disconnected_graph(1000000000, 3).size() == 3);
    auto weighted = a.weighted_graph(6, 5, -3, 5, true);
    std::vector<std::pair<int, int>> unweighted;
    for (auto [u, v, w] : weighted) { assert(-3 <= w && w <= 5); unweighted.emplace_back(u, v); }
    assert(components(6, unweighted) == 1);
    auto weights = a.with_weights(a.tree(5), LLONG_MIN, LLONG_MAX);
    assert(weights.size() == 4);
}

int main() { check_display(); check_random(); }
