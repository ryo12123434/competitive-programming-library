#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

/**
 * @brief 非負整数の0/1 Subset Sumを動的ビット列で計算する。各入力要素を高々1回使う。
 * @note 到達できる和 [0,limit] だけを保持。0重み・重複を許す。
 * @par Complexity N要素・上限Wなら O(N ceil((W+1)/64)) 時間・O(ceil((W+1)/64)) ワード空間。
 * @code
 * #include "library/algorithm/knapsack.hpp"
 * #include <cassert>
 * int main(){SubsetSum sums({3,3,7},10);assert(sums.reachable(6)&&sums.reachable(10)&&!sums.reachable(9));}
 * @endcode
 */
class SubsetSum {
public:
    /// @brief 非負 weights と limit>=0 から構築。
    SubsetSum(const std::vector<int> &weights, int limit) : limit_(limit) {
        assert(limit >= 0);
        bits_.assign(static_cast<std::size_t>(limit) / 64 + 1, 0);
        bits_[0] = 1;
        for (int weight : weights) {
            assert(weight >= 0);
            if (weight > limit)
                continue;
            std::size_t offset = weight / 64;
            unsigned shift = weight % 64;
            for (std::size_t i = bits_.size(); i-- > offset;) {
                auto value = bits_[i - offset] << shift;
                if (shift && i > offset)
                    value |= bits_[i - offset - 1] >> (64 - shift);
                bits_[i] |= value;
            }
        }
        unsigned used = static_cast<unsigned>(limit % 64) + 1;
        if (used < 64)
            bits_.back() &= (std::uint64_t(1) << used) - 1;
    }
    /// @brief 和sumを作れるか。負・上限超過ならfalse。O(1)。空集合の和0は常にtrue。
    bool reachable(int sum) const {
        return 0 <= sum && sum <= limit_ && ((bits_[sum / 64] >> (sum % 64)) & 1);
    }

private:
    int limit_;
    std::vector<std::uint64_t> bits_;
};

/// @brief 0/1 Knapsackの品物。各要素を高々1回使う。weightは非負。
struct KnapsackItem {
    long long weight, value;
};
/**
 * @brief 重さを状態にする0/1 Knapsack。容量以下での最大価値（空集合も可）を128bitで返す。
 * @pre capacity>=0、各weight>=0。価値は負も可、0重みの品物も1回だけ使う。
 * @par Complexity O(NW) 時間・O(W) 空間。W=capacity。全中間価値は128bitに収めること。
 * @code
 * #include "library/algorithm/knapsack.hpp"
 * #include <cassert>
 * int main(){assert(knapsack_by_weight({{0,4},{2,5},{3,8}},3)==12);}
 * @endcode
 */
inline __int128_t knapsack_by_weight(const std::vector<KnapsackItem> &items, int capacity) {
    assert(capacity >= 0);
    std::vector<std::optional<__int128_t>> dp(static_cast<std::size_t>(capacity) + 1);
    dp[0] = 0;
    for (auto item : items) {
        assert(item.weight >= 0);
        if (item.weight > capacity)
            continue;
        int w = static_cast<int>(item.weight);
        for (int c = capacity; c >= w; --c)
            if (dp[c - w]) {
                auto value = *dp[c - w] + item.value;
                if (!dp[c] || *dp[c] < value)
                    dp[c] = value;
            }
    }
    __int128_t best = 0;
    for (auto value : dp)
        if (value)
            best = std::max(best, *value);
    return best;
}
/**
 * @brief 価値を状態にする0/1 Knapsack。容量以下での最大価値を返す。
 * @pre capacity>=0、weight/value>=0、価値合計SはINT_MAX以下。重さの合計は128bitに収めること。
 * @par Complexity O(NS) 時間・O(S) 空間。重さの状態計算は128bitを利用する。
 * @code
 * #include "library/algorithm/knapsack.hpp"
 * #include <cassert>
 * int main(){assert(knapsack_by_value({{2,5},{3,8}},4)==8);}
 * @endcode
 */
inline long long knapsack_by_value(const std::vector<KnapsackItem> &items, long long capacity) {
    assert(capacity >= 0);
    __int128_t total = 0;
    for (auto item : items) {
        assert(item.weight >= 0 && item.value >= 0);
        total += item.value;
    }
    assert(total <= std::numeric_limits<int>::max());
    int sum = static_cast<int>(total);
    std::vector<std::optional<__int128_t>> dp(static_cast<std::size_t>(sum) + 1);
    dp[0] = 0;
    for (auto item : items) {
        int value = static_cast<int>(item.value);
        for (int v = sum; v >= value; --v)
            if (dp[v - value]) {
                auto weight = *dp[v - value] + item.weight;
                if (!dp[v] || weight < *dp[v])
                    dp[v] = weight;
            }
    }
    for (int v = sum; v >= 0; --v)
        if (dp[v] && *dp[v] <= capacity)
            return v;
    return 0;
}
/**
 * @brief Meet-in-the-Middleで部分集合和が[lower,upper)に入る部分集合数を返す。負数も可。
 * @pre 要素数N<=40、lower<=upper。和・境界の比較は符号付き128bitを使用する。
 * @note 同じ和を作る別の部分集合を別々に数える。空集合を含み、最大の個数2^40もuint64_tに収まる。
 * @par Complexity O(N 2^(N/2)) 時間、O(2^(N/2)) 空間。
 * @code
 * #include "library/algorithm/knapsack.hpp"
 * #include <cassert>
 * int main(){assert(subset_sum_mitm_count({-2,2},0,1)==2);}
 * @endcode
 */
inline std::uint64_t subset_sum_mitm_count(const std::vector<long long> &a, __int128_t lower,
                                           __int128_t upper) {
    assert(a.size() <= 40 && lower <= upper);
    auto generate = [&](std::size_t l, std::size_t r) {
        std::vector<__int128_t> s{0};
        s.reserve(std::size_t(1) << (r - l));
        for (std::size_t i = l; i < r; ++i) {
            auto size = s.size();
            for (std::size_t j = 0; j < size; ++j)
                s.push_back(s[j] + a[i]);
        }
        std::sort(s.begin(), s.end());
        return s;
    };
    auto left = generate(0, a.size() / 2), right = generate(a.size() / 2, a.size());
    auto count = [&](__int128_t bound) {
        std::uint64_t result = 0;
        std::size_t j = right.size();
        for (auto x : left) {
            while (j && x + right[j - 1] >= bound)
                --j;
            result += j;
        }
        return result;
    };
    return count(upper) - count(lower);
}
