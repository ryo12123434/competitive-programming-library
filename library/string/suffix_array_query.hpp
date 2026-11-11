#pragma once
#include "../data_structure/sparse_table.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cpl::suffix_query_detail {
inline int minimum(int a, int b) { return std::min(a, b); }
} // namespace cpl::suffix_query_detail
/**
 * @brief ACL等で構築したSA/LCPにRMQを加え、接尾辞LCP・部分文字列比較・パターン検索を行う。
 * @param text 元文字列。0-indexed、バイトをunsignedとして比較する。
 * @param sa textの接尾辞配列。空接尾辞Nを含めず、サイズN。
 * @param lcp 隣接接尾辞のLCP配列。サイズmax(0,N-1)。ACL::lcp_arrayの返り値。
 * @pre 入力配列はtextの正しいSA/LCPで、長さはintに収まること。
 * @note ACL stringの非inline定義を各翻訳単位へ持ち込まないため、SA/LCP構築を利用側で明示する。
 * encodeは負のcharを0..255へ正規化し、ACLの整数列版を安全に使うための補助。
 * @par Complexity SA/LCP構築後にO(N log(N+1))時間・空間。lcp/compareはO(1)。
 * pattern_rangeは長さMのパターンにO((M+1)log(N+1))時間・O(1)追加空間。
 * @code
 * #include "library/string/suffix_array_query.hpp"
 * #include <atcoder/string>
 * #include <cassert>
 * int main(){std::string s="banana";auto bytes=SuffixArrayQuery::encode(s);auto sa=atcoder::suffix_array(bytes,255);SuffixArrayQuery q(s,sa,atcoder::lcp_array(bytes,sa));assert(q.lcp(1,3)==3&&q.compare(1,4,3,6)==0&&q.pattern_range("ana")==std::make_pair(1,3));}
 * @endcode
 */
class SuffixArrayQuery {
    std::string text_;
    std::vector<int> sa_, rank_;
    SparseTable<int, cpl::suffix_query_detail::minimum> rmq_;

public:
    SuffixArrayQuery(std::string text, const std::vector<int> &sa, const std::vector<int> &lcp)
        : text_(std::move(text)), sa_(sa), rank_(text_.size(), -1), rmq_(lcp) {
        assert(sa.size() == text_.size() && lcp.size() == (text_.empty() ? 0 : text_.size() - 1));
        for (int i = 0; i < size(); ++i) {
            assert(0 <= sa[i] && sa[i] < size() && rank_[sa[i]] < 0);
            rank_[sa[i]] = i;
        }
    }
    /// @brief ACLの整数列版へ渡すため、バイトを0..255のintに変換。O(N)。
    static std::vector<int> encode(std::string_view text) {
        std::vector<int> result;
        result.reserve(text.size());
        for (unsigned char c : text)
            result.push_back(c);
        return result;
    }
    int size() const { return static_cast<int>(text_.size()); }
    const std::string &text() const { return text_; }
    const std::vector<int> &suffix_array() const { return sa_; }
    /// @brief 接尾辞[i,N),[j,N)のLCP。空接尾辞Nも指定可。同位置はN-i。
    int lcp(int i, int j) const {
        assert(0 <= i && i <= size() && 0 <= j && j <= size());
        if (i == j)
            return size() - i;
        if (i == size() || j == size())
            return 0;
        int a = rank_[i], b = rank_[j];
        if (a > b)
            std::swap(a, b);
        return rmq_.prod(a, b);
    }
    /// @brief 2つの半開部分文字列を辞書順比較し-1/0/+1。空区間も可。
    int compare(int l1, int r1, int l2, int r2) const {
        assert(0 <= l1 && l1 <= r1 && r1 <= size() && 0 <= l2 && l2 <= r2 && r2 <= size());
        int a = r1 - l1, b = r2 - l2, k = std::min({lcp(l1, l2), a, b});
        if (k == std::min(a, b))
            return a < b ? -1 : a > b ? 1 : 0;
        auto x = static_cast<unsigned char>(text_[l1 + k]), y = static_cast<unsigned char>(text_[l2 + k]);
        return x < y ? -1 : 1;
    }
    /// @brief patternを接頭辞に持つ接尾辞のSA添字域[lo,hi)。空patternは[0,N)、空接尾辞Nは含まない。
    std::pair<int, int> pattern_range(std::string_view pattern) const {
        auto cmp = [&](int suffix) {
            std::size_t k = 0, available = text_.size() - suffix;
            while (k < pattern.size() && k < available && text_[suffix + k] == pattern[k])
                ++k;
            if (k == pattern.size())
                return 0;
            if (k == available)
                return -1;
            return static_cast<unsigned char>(text_[suffix + k]) < static_cast<unsigned char>(pattern[k]) ? -1
                                                                                                          : 1;
        };
        int l = 0, r = size();
        while (l < r) {
            int m = l + (r - l) / 2;
            if (cmp(sa_[m]) < 0)
                l = m + 1;
            else
                r = m;
        }
        int first = l;
        r = size();
        while (l < r) {
            int m = l + (r - l) / 2;
            if (cmp(sa_[m]) <= 0)
                l = m + 1;
            else
                r = m;
        }
        return {first, l};
    }
};
