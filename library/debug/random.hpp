#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cpl::random {
/**
 * @brief 0-indexed の端点と整数の重みを持つ辺。
 * @code
 * #include "library/debug/random.hpp"
 * #include <cassert>
 * int main() {
 *     cpl::random::WeightedEdge edge{0, 1, -2};
 *     assert(edge.weight == -2);
 * }
 * @endcode
 */
struct WeightedEdge { int from, to; long long weight; };

/**
 * @brief 固定 seed で再現できる、テスト入力用の乱数生成器。
 * @note 同じ seed・同じ順の API 呼び出しで再現できる。整数の分布・シャッフルは標準ライブラリの実装差に依存しない。
 * @note グラフは単純無向グラフ。連結グラフや木の分布は全候補上の一様分布ではない。
 * @note LOCAL の有無に関係なく利用可能。
 * @code
 * #include "library/debug/random.hpp"
 * #include <cassert>
 * int main() {
 *     cpl::random::Random random(42);
 *     auto a = random.array(5, -10, 10);
 *     auto tree = random.tree(5);
 *     auto queries = random.intervals(5, 3, true);
 *     assert(a.size() == 5 && tree.size() == 4 && queries.size() == 3);
 * }
 * @endcode
 */
class Random {
    std::uint64_t seed_;
    std::mt19937_64 engine_;

    // bound == 0 は uint64_t の全範囲。剰余の偏りを棄却法で除く。
    std::uint64_t below(std::uint64_t bound) {
        if (bound == 0) return engine_();
        const std::uint64_t threshold = -bound % bound;
        for (;;) {
            const auto value = engine_();
            if (value >= threshold) return value % bound;
        }
    }

    std::vector<std::uint64_t> sample_ids(std::uint64_t total, std::size_t count) {
        assert(count <= total);
        std::unordered_set<std::uint64_t> used;
        used.reserve(count);
        std::vector<std::uint64_t> result;
        result.reserve(count);
        for (std::uint64_t i = total - count; i < total; ++i) {
            auto value = below(i + 1);
            if (!used.insert(value).second) { value = i; used.insert(i); }
            result.push_back(value);
        }
        return result;
    }

    static std::uint64_t edge_id(int u, int v) {
        if (u > v) std::swap(u, v);
        return static_cast<std::uint64_t>(v) * (v - 1) / 2 + u;
    }

public:
    /// @brief seed を指定して構築する。既定値は 0。計算量・保持空間は O(1)。
    explicit Random(std::uint64_t seed = 0) : seed_(seed), engine_(seed) {}
    /// @brief seed で状態を初期化し直す。O(1)。
    void seed(std::uint64_t value) { seed_ = value; engine_.seed(value); }
    /// @brief 最後に設定した seed を返す。現在のエンジン内部状態ではない。O(1)。
    std::uint64_t seed_value() const { return seed_; }

    /**
     * @brief 閉区間 [lo, hi] の整数を一様に生成する。
     * @param lo 下限（含む）。
     * @param hi 上限（含む）。
     * @return long long の乱数。型の最小値・最大値も指定可能。
     * @pre lo <= hi。long long は 64 bit（AtCoder / MinGW-w64）。
     * @par Complexity
     * 期待 O(1) 時間・O(1) 空間。
     */
    long long integer(long long lo, long long hi) {
        static_assert(sizeof(long long) == sizeof(std::uint64_t));
        assert(lo <= hi);
        const auto range = static_cast<std::uint64_t>(hi) - static_cast<std::uint64_t>(lo) + 1;
        const auto bits = static_cast<std::uint64_t>(lo) + below(range);
        return std::bit_cast<long long>(bits);
    }

    /**
     * @brief [lo, hi) の実数を生成する。lo == hi ならその値を返す。
     * @param lo 下限（含む）。
     * @param hi 上限（含まない）。
     * @pre lo <= hi、両端は有限の double。
     * @note 53 bit の乱数を使う。浮動小数点の丸めは環境に依存する。
     * @par Complexity
     * O(1) 時間・空間。
     */
    double real(double lo = 0, double hi = 1) {
        assert(std::isfinite(lo) && std::isfinite(hi) && lo <= hi);
        if (lo == hi) return lo;
        const double unit = static_cast<double>(engine_() >> 11) * 0x1.0p-53;
        return std::min(std::lerp(lo, hi, unit), std::nextafter(hi, lo));
    }

    /// @brief [lo, hi] 内の {lo, hi, -1, 0, 1} を重複なしの昇順で返す。@pre lo <= hi。O(1)。
    static std::vector<long long> boundaries(long long lo, long long hi) {
        assert(lo <= hi);
        std::vector<long long> result{lo, hi};
        for (long long value : {-1LL, 0LL, 1LL}) if (lo <= value && value <= hi) result.push_back(value);
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    /// @brief 半分の確率で boundaries の値、残りは一様整数を返す。@pre lo <= hi。期待 O(1)。
    long long edge_integer(long long lo, long long hi) {
        if (below(2) == 0) {
            const auto values = boundaries(lo, hi);
            return values[below(values.size())];
        }
        return integer(lo, hi);
    }

    /// @brief n 個の一様整数 [lo, hi] を返す。空配列も可能。期待 O(n) 時間・O(n) 出力空間。
    std::vector<long long> array(std::size_t n, long long lo, long long hi) {
        assert(lo <= hi);
        std::vector<long long> result(n);
        for (auto& value : result) value = integer(lo, hi);
        return result;
    }

    /// @brief vector を Fisher-Yates 法で一様に並べ替える。O(n) 時間・追加空間 O(1)。
    template <class T>
    void shuffle(std::vector<T>& values) {
        for (std::size_t i = values.size(); i > 1; --i) {
            const auto j = below(i);
            using std::swap;
            swap(values[i - 1], values[j]);
        }
    }

    /// @brief [0, n) の一様ランダムな順列を返す。@pre n >= 0。O(n) 時間・空間。
    std::vector<int> permutation(int n) {
        assert(n >= 0);
        std::vector<int> result(n);
        for (int i = 0; i < n; ++i) result[i] = i;
        shuffle(result);
        return result;
    }

    /// @brief alphabet の各バイトから一様に選んだ長さ n の文字列。@pre alphabet は非空。O(n) 時間・空間。
    std::string string(std::size_t n, std::string_view alphabet = "abc") {
        assert(!alphabet.empty());
        std::string result(n, '\0');
        for (char& ch : result) ch = alphabet[below(alphabet.size())];
        return result;
    }

    /**
     * @brief 添字区間 [l, r) を生成する。0 <= l <= r <= n。
     * @param n 配列の要素数（非負）。
     * @param nonempty true なら l < r とする。この場合 n > 0 が必要。
     * @return 0-indexed の端点。false では空区間・[0,n) も生成し得る。
     * @note true では全非空区間に一様。false では各区間に一様ではない。
     * @par Complexity
     * 期待 O(1) 時間・空間。
     */
    std::pair<int, int> interval(int n, bool nonempty = false) {
        assert(n >= 0 && (!nonempty || n > 0));
        int l = static_cast<int>(integer(0, n));
        int r = static_cast<int>(integer(0, n - static_cast<int>(nonempty)));
        if (nonempty && r >= l) ++r;
        if (l > r) std::swap(l, r);
        return {l, r};
    }

    /// @brief interval(n, nonempty) を q 個生成する。同じ前提条件。期待 O(q) 時間・空間。
    std::vector<std::pair<int, int>> intervals(int n, std::size_t q, bool nonempty = false) {
        assert(n >= 0 && (!nonempty || n > 0));
        std::vector<std::pair<int, int>> result(q);
        for (auto& value : result) value = interval(n, nonempty);
        return result;
    }

    /// @brief 0-indexed の n 頂点の木を辺列で返す。n == 0 は空列。@pre n >= 0。期待 O(n) 時間・空間。
    std::vector<std::pair<int, int>> tree(int n) {
        assert(n >= 0);
        auto order = permutation(n);
        std::vector<std::pair<int, int>> edges;
        for (int v = 1; v < n; ++v) edges.emplace_back(order[integer(0, v - 1)], order[v]);
        shuffle(edges);
        return edges;
    }

    /**
     * @brief n 頂点 m 辺の単純無向グラフを辺列で生成する。密なグラフも全辺の列挙を必要としない。
     * @param n 頂点数。頂点は [0, n)。
     * @param m 辺数。0 <= m <= n*(n-1)/2。
     * @param connected true なら先に全域木を作る。n > 0 なら m >= n-1 が必要。
     * @return 自己ループ・多重辺のない辺列。n == 0, 1 は m == 0 に限る。
     * @pre n, m >= 0。要求する出力をメモリに保持できること。
     * @note false は連結性を保証しない。必ず非連結にするなら disconnected_graph を使う。
     * @par Complexity
     * 期待 O((m+1) log(n+m+2)) 時間、O(m+1) 空間。連結指定時は n <= m+1。
     */
    std::vector<std::pair<int, int>> graph(int n, int m, bool connected = false) {
        assert(n >= 0 && m >= 0);
        const std::uint64_t total = n < 2 ? 0 : static_cast<std::uint64_t>(n) * (n - 1) / 2;
        assert(static_cast<std::uint64_t>(m) <= total && (!connected || n == 0 || m >= n - 1));
        auto edges = connected ? tree(n) : std::vector<std::pair<int, int>>{};
        std::vector<std::uint64_t> forbidden;
        for (auto [u, v] : edges) forbidden.push_back(edge_id(u, v));
        std::sort(forbidden.begin(), forbidden.end());
        auto ranks = sample_ids(total - forbidden.size(), static_cast<std::size_t>(m) - edges.size());
        std::sort(ranks.begin(), ranks.end());
        std::size_t skipped = 0;
        for (auto rank : ranks) {
            auto id = rank + skipped;
            while (skipped < forbidden.size() && forbidden[skipped] <= id) { ++skipped; ++id; }
            int low = 1, high = n;
            while (high - low > 1) {
                const int mid = low + (high - low) / 2;
                if (static_cast<std::uint64_t>(mid) * (mid - 1) / 2 <= id) low = mid;
                else high = mid;
            }
            edges.emplace_back(static_cast<int>(id - static_cast<std::uint64_t>(low) * (low - 1) / 2), low);
        }
        shuffle(edges);
        return edges;
    }

    /**
     * @brief 必ず孤立点を持つ単純無向グラフ。孤立点の番号もランダムに選ぶ。
     * @param n 頂点数（2 以上）。
     * @param m 辺数。0 <= m <= (n-1)*(n-2)/2。
     * @return 非連結な辺列。残りの n-1 頂点の連結性は保証しない。
     * @par Complexity
     * graph と同じ時間・空間。
     */
    std::vector<std::pair<int, int>> disconnected_graph(int n, int m) {
        assert(n >= 2);
        auto edges = graph(n - 1, m);
        const int isolated = static_cast<int>(integer(0, n - 1));
        for (auto& [u, v] : edges) { u += u >= isolated; v += v >= isolated; }
        return edges;
    }

    /// @brief 辺列に [lo, hi] の一様整数の重みを付ける。@pre lo <= hi。期待 O(m) 時間・空間。
    std::vector<WeightedEdge> with_weights(const std::vector<std::pair<int, int>>& edges, long long lo, long long hi) {
        assert(lo <= hi);
        std::vector<WeightedEdge> result;
        result.reserve(edges.size());
        for (auto [u, v] : edges) result.push_back({u, v, integer(lo, hi)});
        return result;
    }

    /// @brief graph(n,m,connected) に整数の重み [lo,hi] を付ける。前提条件・計算量は graph と同じ。
    std::vector<WeightedEdge> weighted_graph(int n, int m, long long lo, long long hi, bool connected = false) {
        return with_weights(graph(n, m, connected), lo, hi);
    }
};
} // namespace cpl::random
