#include "library/data_structure/fast_set.hpp"
#include "library/data_structure/dynamic_segment_tree.hpp"
#include "library/data_structure/union_find_monoid.hpp"
#include "library/string/palindromic_tree.hpp"
#include "library/geometry/minimum_enclosing_circle.hpp"
#include "library/geometry/minkowski_sum.hpp"

#include "library/geometry/convex_hull.hpp"
#include <climits>
#include <map>
#include <numeric>
#include <set>
#include <string>
using namespace cpl::geometry;
using P = Point<long long>;
long long sum(long long a, long long b) { return a + b; }
long long zero() { return 0; }
std::string join(std::string a, std::string b) { return a + b; }
std::string empty_string() { return ""; }
void sets() {
    std::mt19937 rng(83);
    for (int n : {0, 1, 63, 64, 65, 127, 128, 129, 4095, 4096, 4097, 65537}) {
        FastSet s(n);
        std::set<int> ref;
        for (int k = 0; k < 5000; ++k) {
            int x = n ? int(rng() % n) : 0;
            if (n) {
                if (rng() % 2)
                    assert(s.insert(x) == ref.insert(x).second);
                else
                    assert(s.erase(x) == bool(ref.erase(x)));
            }
            int query = int(rng() % 100000) - 10000;
            auto next = ref.lower_bound(query), prev = ref.upper_bound(query);
            std::optional<int> a, b;
            if (next != ref.end())
                a = *next;
            if (prev != ref.begin())
                b = *--prev;
            assert(s.next(query) == a && s.prev(query) == b);
            assert(s.size() == int(ref.size()) && s.empty() == ref.empty());
            if (n) {
                int l = rng() % (n + 1), r = rng() % (n + 1);
                if (l > r)
                    std::swap(l, r);
                auto it = ref.lower_bound(l);
                assert(s.any(l, r) == (it != ref.end() && *it < r));
            }
            if (!ref.empty())
                assert(s.min() == *ref.begin() && s.max() == *ref.rbegin());
        }
        if (n < 5000) {
            FastSet full(n, true);
            assert(full.size() == n);
            for (int i = 0; i < n; ++i)
                assert(full.contains(i));
            for (int i = 0; i < n; ++i)
                full.erase(i);
            assert(full.empty() && !full.next(0));
        }
    }
    FastSet text(std::string_view("010010"));
    assert(text.size() == 2 && text.min() == 1 && text.max() == 4);
}
void dynamic_tree() {
    std::mt19937_64 rng(554);
    DynamicSegmentTree<long long, sum, zero> s(-100, 101);
    std::map<long long, long long> ref;
    for (int t = 0; t < 10000; ++t) {
        long long p = int(rng() % 201) - 100, value = int(rng() % 100) - 50;
        s.set(p, value);
        ref[p] = value;
        long long l = int(rng() % 202) - 100, r = int(rng() % 202) - 100;
        if (l > r)
            std::swap(l, r);
        long long expected = 0;
        for (auto [x, v] : ref)
            if (l <= x && x < r)
                expected += v;
        auto count = s.node_count();
        assert(s.prod(l, r) == expected && s.node_count() == count && s.get(p) == value);
    }
    DynamicSegmentTree<long long, sum, zero> empty(0, 0);
    assert(empty.all_prod() == 0 && empty.prod(0, 0) == 0);
    DynamicSegmentTree<long long, sum, zero> wide(LLONG_MIN, LLONG_MAX);
    for (long long p : {LLONG_MIN, LLONG_MIN + 1, -1LL, 0LL, LLONG_MAX - 1})
        wide.set(p, 1);
    assert(wide.all_prod() == 5 && wide.prod(LLONG_MIN, 0) == 3 && wide.prod(0, LLONG_MAX) == 2);
    DynamicSegmentTree<std::string, join, empty_string> ordered(-10, 11);
    std::map<int, std::string> letters;
    for (int t = 0; t < 500; ++t) {
        int p = int(rng() % 21) - 10;
        std::string value(1, char('a' + rng() % 26));
        ordered.set(p, value);
        letters[p] = value;
        std::string expected;
        for (auto &[x, c] : letters)
            expected += c;
        assert(ordered.all_prod() == expected);
    }
    DynamicSegmentTree<long long, sum, zero> large(LLONG_MIN, LLONG_MAX);
    large.reserve_nodes(1);
    for (int i = 0; i < 20000; ++i)
        large.set(std::int64_t(rng() >> 1), 1);
    assert(large.all_prod() == 20000 && large.node_count() < 20000 * 65ULL);
}
void monoid() {
    std::mt19937 rng(454);
    int n = 50;
    std::vector<int> label(n);
    std::iota(label.begin(), label.end(), 0);
    std::vector<long long> value(n);
    for (auto &x : value)
        x = int(rng() % 30) - 15;
    UnionFindMonoid<long long, sum, zero> u(value);
    for (int t = 0; t < 5000; ++t) {
        int a = rng() % n, b = rng() % n;
        if (rng() % 3 == 0) {
            long long x = int(rng() % 100) - 50;
            u.set(a, x);
            value[label[a]] = x;
        } else {
            int x = label[a], y = label[b];
            u.merge(a, b);
            if (x != y) {
                value[x] += value[y];
                for (auto &z : label)
                    if (z == y)
                        z = x;
            }
        }
        for (int i = 0; i < n; ++i) {
            assert(u.get(i) == value[label[i]]);
            assert(u.same(a, i) == (label[a] == label[i]));
            assert(u.size(i) == std::count(label.begin(), label.end(), label[i]));
        }
    }
    UnionFindMonoid<std::string, join, empty_string> strings(std::vector<std::string>{"a", "b", "c", "d"});
    strings.merge(1, 2);
    strings.merge(0, 1);
    strings.merge(3, 0);
    assert(strings.get(2) == "dabc");
    strings.set(1, "x");
    strings.merge(0, 3);
    assert(strings.get(2) == "x");
    UnionFindMonoid<long long, sum, zero> empty(0);
}
bool palindrome(const std::string &s) { return std::equal(s.begin(), s.end(), s.rbegin()); }
void check_palindromes(const std::string &s) {
    PalindromicTree tree;
    std::string prefix;
    assert(tree.longest_suffix() == 1);
    for (char c : s) {
        prefix += c;
        int id = tree.add_char(c);
        std::map<std::string, std::uint64_t> expected;
        int longest = 0;
        for (int l = 0; l < int(prefix.size()); ++l)
            for (int r = l + 1; r <= int(prefix.size()); ++r) {
                auto part = prefix.substr(l, r - l);
                if (palindrome(part)) {
                    ++expected[part];
                    if (r == int(prefix.size()))
                        longest = std::max(longest, r - l);
                }
            }
        assert(tree.node_info(id).length == longest && tree.distinct_count() == int(expected.size()));
        auto count = tree.occurrences();
        assert(count[0] == 0 && count[1] == 0);
        std::set<std::string> unique;
        for (int v = 2; v < tree.node_count(); ++v) {
            auto node = tree.node_info(v);
            auto part = prefix.substr(node.first_end - node.length, node.length);
            assert(unique.insert(part).second && palindrome(part) && count[v] == expected[part]);
            assert(tree.node_info(node.inner).length == node.length - 2);
            int suffix = 0;
            for (int k = 1; k < node.length; ++k)
                if (palindrome(part.substr(node.length - k)))
                    suffix = k;
            assert(tree.node_info(node.link).length == suffix);
        }
    }
}
void eertree() {
    for (int len = 0; len <= 7; ++len) {
        int total = 1;
        for (int i = 0; i < len; ++i)
            total *= 3;
        for (int mask = 0; mask < total; ++mask) {
            int x = mask;
            std::string s;
            for (int i = 0; i < len; ++i) {
                s += char('a' + x % 3);
                x /= 3;
            }
            check_palindromes(s);
        }
    }
    std::mt19937 rng(52);
    for (int t = 0; t < 200; ++t) {
        std::string s;
        for (int k = rng() % 25; k--;)
            s += char(rng() % 256);
        check_palindromes(s);
    }
    std::string s(200000, 'a');
    PalindromicTree tree(s);
    auto count = tree.occurrences();
    assert(tree.distinct_count() == 200000 && count[2] == 200000 && count.back() == 1);
    tree.add_char('b');
    assert(tree.node_info(tree.longest_suffix()).length == 1);
}
RealCircle brute_circle(const std::vector<RealPoint> &p) {
    RealCircle best{{0, 0}, 1e100L};
    auto check = [&](RealCircle c) {
        for (auto x : p)
            if (distance(x, c.center) > c.radius + 1e-12L * std::max(1.0L, c.radius))
                return;
        if (c.radius < best.radius)
            best = c;
    };
    for (auto a : p)
        check({a, 0});
    for (std::size_t i = 0; i < p.size(); ++i)
        for (std::size_t j = 0; j < i; ++j) {
            auto m = p[i] + (p[j] - p[i]) / 2;
            check({m, distance(m, p[i])});
            for (std::size_t k = 0; k < j; ++k) {
                auto u = p[j] - p[i], v = p[k] - p[i];
                auto d = 2 * (u.x * v.y - u.y * v.x);
                if (d == 0)
                    continue;
                auto a = u.x * u.x + u.y * u.y, b = v.x * v.x + v.y * v.y;
                RealPoint c = p[i] + RealPoint{(a * v.y - b * u.y) / d, (u.x * b - v.x * a) / d};
                check({c, distance(c, p[i])});
            }
        }
    return best;
}
void circles() {
    assert(!minimum_enclosing_circle({}));
    std::mt19937_64 rng(52);
    for (int t = 0; t < 2500; ++t) {
        int n = 1 + rng() % 10;
        std::vector<RealPoint> p;
        for (int i = 0; i < n; ++i)
            p.push_back({static_cast<long double>(int(rng() % 41) - 20),
                         static_cast<long double>(int(rng() % 41) - 20)});
        auto ref = brute_circle(p);
        for (std::uint64_t seed : {0, 1, 2}) {
            auto got = minimum_enclosing_circle(p, seed);
            assert(got && std::abs(got->circle.radius - ref.radius) <= 1e-10L * std::max(1.0L, ref.radius));
            for (auto x : p)
                assert(distance(got->circle.center, x) <= got->circle.radius);
            for (int i : got->support)
                assert(i >= 0 && i < n);
        }
    }
    for (auto p :
         std::vector<std::vector<RealPoint>>{{{0, 0}},
                                             {{1, 1}, {1, 1}},
                                             {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
                                             {{0, 0}, {1, 1e-25L}, {2, 0}},
                                             {{1e12L, 1e12L}, {1e12L + 2, 1e12L}, {1e12L + 1, 1e12L + 1}}}) {
        auto r = minimum_enclosing_circle(p);
        assert(r);
        for (auto x : p)
            assert(distance(r->circle.center, x) <= r->circle.radius);
    }
    std::vector<RealPoint> p;
    for (int i = 0; i < 100000; ++i)
        p.push_back({static_cast<long double>(int(rng() % 20001) - 10000),
                     static_cast<long double>(int(rng() % 20001) - 10000)});
    auto r = minimum_enclosing_circle(p);
    for (auto x : p)
        assert(distance(r->circle.center, x) <= r->circle.radius);
}
void minkowski() {
    std::mt19937_64 rng(62);
    auto check = [](std::vector<P> a, std::vector<P> b) {
        std::vector<P> all;
        for (auto x : a)
            for (auto y : b)
                all.push_back(x + y);
        auto expected = convex_hull(all);
        auto got = minkowski_sum(a, b);
        assert(got == expected);
    };
    for (int t = 0; t < 3000; ++t) {
        std::vector<P> a, b;
        for (int k = rng() % 20; k--;)
            a.push_back({int(rng() % 21) - 10, int(rng() % 21) - 10});
        for (int k = rng() % 20; k--;)
            b.push_back({int(rng() % 21) - 10, int(rng() % 21) - 10});
        a = convex_hull(a);
        b = convex_hull(b);
        if (!a.empty())
            std::rotate(a.begin(), a.begin() + rng() % a.size(), a.end());
        if (!b.empty())
            std::rotate(b.begin(), b.begin() + rng() % b.size(), b.end());
        if (rng() % 2)
            std::reverse(a.begin(), a.end());
        if (rng() % 2)
            std::reverse(b.begin(), b.end());
        check(a, b);
    }
    check({{1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 0}, {1, 0}}, {{0, 0}, {0, 1}});
    check({{0, 4}, {0, -3}, {0, 1}}, {{3, 0}, {-2, 0}});
    const long long bound = (1LL << 60) - 1;
    check({{-bound, -bound}, {bound, -bound}, {bound, bound}, {-bound, bound}},
          {{bound, 0}, {0, bound}, {-bound, 0}, {0, -bound}});
    bool caught = false;
    try {
        minkowski_sum(std::vector<Point<int>>{{INT_MAX, 0}}, std::vector<Point<int>>{{1, 0}});
    } catch (const std::overflow_error &) {
        caught = true;
    }
    assert(caught);
}
int main() {
    sets();
    dynamic_tree();
    monoid();
    eertree();
    circles();
    minkowski();
}
