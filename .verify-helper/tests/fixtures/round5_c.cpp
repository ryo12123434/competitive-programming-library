#include "library/string/suffix_array_query.hpp"
#include "library/string/dynamic_rolling_hash.hpp"
#include "library/string/runs.hpp"

#include <atcoder/string>
#include <random>
#include <set>
#include <tuple>
SuffixArrayQuery make_query(const std::string &s) {
    auto bytes = SuffixArrayQuery::encode(s);
    auto sa = atcoder::suffix_array(bytes, 255);
    return SuffixArrayQuery(s, sa, bytes.empty() ? std::vector<int>{} : atcoder::lcp_array(bytes, sa));
}
void suffix() {
    std::mt19937 rng(726);
    for (int rep = 0; rep < 600; ++rep) {
        std::string s;
        for (int i = rng() % 60; i--;)
            s += char(rng() % 256);
        auto q = make_query(s);
        int n = s.size();
        auto unsigned_less = [](const std::string &a, const std::string &b) {
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                                [](unsigned char x, unsigned char y) { return x < y; });
        };
        for (int i = 0; i <= n; ++i)
            for (int j = 0; j <= n; ++j) {
                int expected = 0;
                while (i + expected < n && j + expected < n && s[i + expected] == s[j + expected])
                    ++expected;
                assert(q.lcp(i, j) == expected);
            }
        for (int t = 0; t < 200; ++t) {
            int l = rng() % (n + 1), r = rng() % (n + 1), x = rng() % (n + 1), y = rng() % (n + 1);
            if (l > r)
                std::swap(l, r);
            if (x > y)
                std::swap(x, y);
            auto a = s.substr(l, r - l), b = s.substr(x, y - x);
            int expected = a == b ? 0 : unsigned_less(a, b) ? -1 : 1;
            assert(q.compare(l, r, x, y) == expected);
            std::string pattern;
            if (rng() % 2)
                pattern = a;
            else
                for (int k = rng() % 10; k--;)
                    pattern += char(rng() % 256);
            auto [lo, hi] = q.pattern_range(pattern);
            std::vector<int> matches;
            for (int i = 0; i < n; ++i)
                if (s.compare(i, pattern.size(), pattern) == 0)
                    matches.push_back(i);
            std::vector<int> actual(q.suffix_array().begin() + lo, q.suffix_array().begin() + hi);
            std::sort(actual.begin(), actual.end());
            assert(actual == matches);
        }
    }
    auto q = make_query(std::string(200000, 'a'));
    assert(q.lcp(0, 1) == 199999 &&
           q.pattern_range(std::string(10000, 'a')).second - q.pattern_range(std::string(10000, 'a')).first ==
               190001);
}
std::uint64_t direct_hash(const std::string &s, int l, int r, std::uint64_t base) {
    std::uint64_t value = 0;
    for (int i = l; i < r; ++i)
        value = (__uint128_t(value) * base + static_cast<unsigned char>(s[i])) % RollingHash::mod;
    return value;
}
void dynamic_hash() {
    std::mt19937_64 rng(394);
    for (auto base : std::vector<std::uint64_t>{256, 911382323, RollingHash::mod - 2})
        for (int n = 0; n < 70; ++n) {
            std::string s(n, 'a');
            for (auto &c : s)
                c = char(rng() % 256);
            DynamicRollingHash h(s, base);
            for (int t = 0; t < 500; ++t) {
                if (n && rng() % 2) {
                    int i = rng() % n;
                    char c = char(rng() % 256);
                    s[i] = c;
                    h.set(i, c);
                }
                int l = rng() % (n + 1), r = rng() % (n + 1);
                if (l > r)
                    std::swap(l, r);
                int m = l + (r - l ? int(rng() % (r - l + 1)) : 0);
                auto hash = h.get(l, r);
                assert(hash.hash == direct_hash(s, l, r, base) && hash.length == r - l && hash.base == base);
                assert(h.connect(h.get(l, m), h.get(m, r)) == hash);
            }
        }
    auto old = RollingHash::base;
    RollingHash::base = 911382323;
    RollingHash a("abc");
    DynamicRollingHash b("abc");
    assert(a.get(0, 3) == b.get(0, 3).hash);
    RollingHash::base = 999983;
    assert(b.get(0, 3).hash == direct_hash("abc", 0, 3, 911382323));
    RollingHash::base = old;
    bool caught = false;
    try {
        b.connect(b.get(0, 1), DynamicRollingHash("a", 256).get(0, 1));
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
    std::string s(200000, 'a');
    DynamicRollingHash large(s, 911382323);
    for (int i = 0; i < 200000; i += 17) {
        large.set(i, 'b');
        s[i] = 'b';
    }
    assert(large.get(0, 200000).hash == direct_hash(s, 0, 200000, 911382323));
}
std::vector<StringRun> brute(const std::string &s) {
    int n = s.size();
    std::vector<StringRun> answer;
    for (int l = 0; l < n; ++l)
        for (int r = l + 2; r <= n; ++r) {
            int period = 0;
            for (int p = 1; p <= r - l; ++p) {
                bool ok = true;
                for (int i = l + p; i < r; ++i)
                    if (s[i] != s[i - p]) {
                        ok = false;
                        break;
                    }
                if (ok) {
                    period = p;
                    break;
                }
            }
            if (2 * period > r - l)
                continue;
            if (l > 0 && s[l - 1] == s[l - 1 + period])
                continue;
            if (r < n && s[r] == s[r - period])
                continue;
            answer.push_back({l, r, period});
        }
    return answer;
}
void check_runs(const std::string &s) {
    auto a = make_query(s), b = make_query(std::string(s.rbegin(), s.rend()));
    assert(enumerate_runs(a, b) == brute(s));
}
void runs() {
    for (int n = 0; n <= 12; ++n)
        for (int mask = 0; mask < (1 << n); ++mask) {
            std::string s(n, 'a');
            for (int i = 0; i < n; ++i)
                if (mask >> i & 1)
                    s[i] = 'b';
            check_runs(s);
        }
    std::mt19937 rng(416);
    for (int t = 0; t < 1000; ++t) {
        std::string s;
        for (int i = rng() % 50; i--;)
            s += char('a' + rng() % 4);
        check_runs(s);
    }
    std::string s(200000, 'a');
    auto a = make_query(s);
    auto one = enumerate_runs(a, a);
    assert(one.size() == 1 && one[0] == StringRun({0, 200000, 1}));
    for (int i = 0; i < 200000; ++i)
        s[i] = char('a' + (__builtin_popcount(unsigned(i)) & 1));
    auto f = make_query(s), r = make_query(std::string(s.rbegin(), s.rend()));
    auto many = enumerate_runs(f, r);
    for (auto run : many) {
        assert(run.r - run.l >= 2 * run.period &&
               f.lcp(run.l, run.l + run.period) >= run.r - run.l - run.period);
        if (run.l)
            assert(s[run.l - 1] != s[run.l - 1 + run.period]);
        if (run.r < int(s.size()))
            assert(s[run.r] != s[run.r - run.period]);
    }
    bool caught = false;
    try {
        enumerate_runs(make_query("ab"), make_query("ab"));
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    assert(caught);
}
int main() {
    suffix();
    dynamic_hash();
    runs();
}
