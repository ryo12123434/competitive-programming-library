#pragma once
#include "suffix_array_query.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

/**
 * @brief 半開の極大周期区間[l,r)と最小周期period。長さは2*period以上。
 * @note 直前/直後へ同じ周期では延長できない。enumerate_runsの返り値は(l,r)順、重複なし。
 * @code
 * #include "library/string/runs.hpp"
 * #include <atcoder/string>
 * #include <cassert>
 * SuffixArrayQuery query(const std::string&s){auto a=SuffixArrayQuery::encode(s);auto sa=atcoder::suffix_array(a,255);return SuffixArrayQuery(s,sa,a.empty()?std::vector<int>{}:atcoder::lcp_array(a,sa));}
 * int main(){std::string s="ababab";auto a=query(s),b=query(std::string(s.rbegin(),s.rend()));auto r=enumerate_runs(a,b);assert(r.size()==1&&r[0].l==0&&r[0].r==6&&r[0].period==2);}
 * @endcode
 */
struct StringRun {
    int l, r, period;
    bool operator==(const StringRun &) const = default;
};
/**
 * @brief 元列と逆列のSuffixArrayQueryを利用し、Runsを列挙する。浮動小数点・ハッシュは使わない。
 * @param forward 元の文字列のSA/LCP/RMQ。
 * @param reversed 逆文字列のSA/LCP/RMQ。違う文字列ならinvalid_argument。
 * @note 各周期pでp間隔の格子上の2点をLCP/LCSで伸長する。長さ>=2pの区間は必ずこの2点を含む。
 * 非最小周期pの候補はFine–Wilfの定理より最小周期と同じ両端を持つ。端点ごとの最小pだけ残す。
 * 端点2回の安定計数ソートで、候補数CにO(N+C)の重複除去を行う。
 * @par Complexity sum_p O(N/p)=O(N log(N+1))時間・最悪空間（RMQ構築も同じ）。空列は空。
 */
inline std::vector<StringRun> enumerate_runs(const SuffixArrayQuery &forward,
                                             const SuffixArrayQuery &reversed) {
    int n = forward.size();
    if (reversed.size() != n ||
        !std::equal(forward.text().begin(), forward.text().end(), reversed.text().rbegin()))
        throw std::invalid_argument("Runs: reverse text mismatch");
    std::vector<StringRun> candidates;
    for (int p = 1; p <= n / 2; ++p) {
        int last_l = -1, last_r = -1;
        for (int i = 0; i < n - p; i += p) {
            int j = i + p, left = i - reversed.lcp(n - i, n - j), right = j + forward.lcp(i, j);
            if (right - left >= 2 * p && (left != last_l || right != last_r)) {
                candidates.push_back({left, right, p});
                last_l = left;
                last_r = right;
            }
        }
    }
    if (candidates.empty())
        return {};
    std::vector<StringRun> temporary(candidates.size());
    std::vector<std::size_t> offsets(static_cast<std::size_t>(n) + 2);
    auto counting = [&](bool by_left) {
        std::fill(offsets.begin(), offsets.end(), 0);
        for (auto r : candidates)
            ++offsets[static_cast<std::size_t>(by_left ? r.l : r.r) + 1];
        for (std::size_t i = 1; i < offsets.size(); ++i)
            offsets[i] += offsets[i - 1];
        for (auto r : candidates)
            temporary[offsets[by_left ? r.l : r.r]++] = r;
        candidates.swap(temporary);
    };
    counting(false);
    counting(true);
    std::vector<StringRun> result;
    for (auto r : candidates) {
        if (result.empty() || result.back().l != r.l || result.back().r != r.r)
            result.push_back(r);
        else
            result.back().period = std::min(result.back().period, r.period);
    }
    return result;
}
