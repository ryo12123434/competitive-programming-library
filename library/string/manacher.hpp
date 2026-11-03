#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief Manacher の結果。odd[i] は中心 i を含む半径、even[i] は i-1 と i の間を中心とする半径。
 * @note 奇数区間は [i-odd[i]+1,i+odd[i])、偶数区間は [i-even[i],i+even[i])。
 * 配列は参照用に公開する。変更すると取得 API の結果は保証しない。
 * @par Complexity
 * 保持空間 O(N)、is_palindrome O(1)、longest O(N)。生成例は manacher を参照。
 */
struct ManacherResult {
    std::vector<int> odd, even;

    /// @brief [l,r) が回文なら true。空区間も true。@pre 0 <= l <= r <= N。
    bool is_palindrome(int l, int r) const {
        assert(0 <= l && l <= r && r <= static_cast<int>(odd.size()));
        const int len = r - l;
        if (len == 0) return true;
        const int center = l + len / 2;
        return len % 2 ? odd[center] >= len / 2 + 1 : even[center] >= len / 2;
    }

    /// @brief 最長回文の半開区間。複数なら左端最小、空列なら (0,0)。O(N)。
    std::pair<int, int> longest() const {
        std::pair<int, int> best{0, 0};
        for (int i = 0; i < static_cast<int>(odd.size()); ++i) {
            for (auto interval : {std::pair{i - odd[i] + 1, i + odd[i]},
                                  std::pair{i - even[i], i + even[i]}}) {
                const int len = interval.second - interval.first;
                if (len > best.second - best.first ||
                    (len == best.second - best.first && interval.first < best.first)) best = interval;
            }
        }
        return best;
    }
};

/**
 * @brief 奇数・偶数回文半径をセンチネルなしで求める。
 * @tparam Sequence size()、0-indexed の []、要素の == を持つ列型（string、vector など）。
 * @param s 入力列。変更せず、結果は列を参照しない。
 * @pre N <= INT_MAX。要素比較は同値関係であること。
 * @par Complexity
 * 要素比較 O(1) の場合 O(N) 時間・空間。
 * @code
 * #include "library/string/manacher.hpp"
 * #include <string>
 * #include <cassert>
 * int main() {
 *     std::string s = "abba";
 *     auto result = manacher(s);
 *     assert(result.even[2] == 2 && result.is_palindrome(0, 4));
 *     auto [l, r] = result.longest();
 *     assert(s.substr(l, r-l) == "abba");
 * }
 * @endcode
 */
template<class Sequence>
ManacherResult manacher(const Sequence& s) {
    assert(s.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int n = static_cast<int>(s.size());
    ManacherResult result{std::vector<int>(n), std::vector<int>(n)};
    for (int i = 0, l = 0, r = -1; i < n; ++i) {
        int k = i > r ? 1 : std::min(result.odd[l + (r-i)], r-i+1);
        while (k <= i && k < n-i && s[i-k] == s[i+k]) ++k;
        result.odd[i] = k;
        if (i+k-1 > r) { l = i-k+1; r = i+k-1; }
    }
    for (int i = 0, l = 0, r = -1; i < n; ++i) {
        int k = i > r ? 0 : std::min(result.even[l + (r-i) + 1], r-i+1);
        while (k < i && k < n-i && s[i-k-1] == s[i+k]) ++k;
        result.even[i] = k;
        if (i+k-1 > r) { l = i-k; r = i+k-1; }
    }
    return result;
}
