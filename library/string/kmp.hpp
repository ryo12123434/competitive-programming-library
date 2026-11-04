#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

/**
 * @brief 各接頭辞の「自身を除く最長の接頭辞かつ接尾辞」の長さを求める。
 * @tparam Sequence size(), operator[] と要素の == を持つ列。string / vector など。
 * @param s 入力列。空なら空配列を返す。@pre s.size()<=INT_MAX。
 * @return pi[i] は s[0,i+1) の最長 border の長さ。pi[0]=0。
 * @par Complexity
 * 要素の比較が O(1) なら O(N) 時間・空間。
 * @code
 * #include "library/string/kmp.hpp"
 * #include <string>
 * #include <cassert>
 * int main() { assert((prefix_function(std::string("ababa")) == std::vector<int>{0,0,1,2,3})); }
 * @endcode
 */
template <class Sequence>
std::vector<int> prefix_function(const Sequence& s) {
    assert(s.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int n = static_cast<int>(s.size());
    std::vector<int> pi(n);
    for (int i = 1; i < n; ++i) {
        int j = pi[i - 1];
        while (j && !(s[i] == s[j])) j = pi[j - 1];
        if (s[i] == s[j]) ++j;
        pi[i] = j;
    }
    return pi;
}

/**
 * @brief KMP で pattern の出現開始位置を昇順に列挙する。重なった出現も含む。
 * @param text 検索対象。@param pattern パターン。空なら 0..text.size() の全境界に一致。
 * @pre 両列の長さは INT_MAX 以下。要素同士を == で比較可能。
 * @return 0-indexed の開始位置列。
 * @par Complexity
 * O(N+M) 時間、補助空間 O(M)、返り値の空間 O(出現数)。
 * @code
 * #include "library/string/kmp.hpp"
 * #include <string>
 * #include <cassert>
 * int main() { assert((kmp_search(std::string("aaaa"),std::string("aa")) == std::vector<int>{0,1,2})); }
 * @endcode
 */
template <class Text, class Pattern>
std::vector<int> kmp_search(const Text& text, const Pattern& pattern) {
    assert(text.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    auto pi = prefix_function(pattern);
    int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
    std::vector<int> result;
    if (!m) {
        for (int i = 0;; ++i) { result.push_back(i); if (i == n) break; }
        return result;
    }
    int j = 0;
    for (int i = 0; i < n; ++i) {
        while (j && !(text[i] == pattern[j])) j = pi[j - 1];
        if (text[i] == pattern[j]) ++j;
        if (j == m) { result.push_back(i - m + 1); j = pi[j - 1]; }
    }
    return result;
}
