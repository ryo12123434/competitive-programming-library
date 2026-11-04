#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @brief バイト列の部分文字列を管理する Suffix Automaton。末尾への逐次追加に対応。
 * @note 空文字列は常に存在し、出現数は N+1。重なる出現も数える。
 * 遷移は unsigned char をキーとする std::map。既存の列は保持しない。
 * @pre 長さ N<=(INT_MAX-1)/2。異なる非空部分文字列数は long long に収まる。
 * @par Complexity
 * 文字種類数 A<=256。構築・全 append は O(N log(A+1))、保持空間 O(N)。
 * N>=3 で状態数 <=2N-1、遷移数 <=3N-4（空入力は1状態）。
 * contains は O(M log(A+1))。出現数集計は追加後初回に O(N)、以後は検索のコストのみ。
 * @code
 * #include "library/string/suffix_automaton.hpp"
 * #include <cassert>
 * int main() {
 *     SuffixAutomaton sam("aba"); assert(sam.distinct_substrings()==5 && sam.occurrences("a")==2);
 *     sam.append('b'); assert(sam.contains("bab") && sam.occurrences("ab")==2);
 * }
 * @endcode
 */
class SuffixAutomaton {
public:
    /// @brief s を初期文字列にする。空で構築して append してもよい。
    explicit SuffixAutomaton(std::string_view s = {}) { states_.push_back({}); for (unsigned char c : s) append(c); }
    /// @brief 末尾に1バイトを追加。償却 O(log(A+1))。出現数のキャッシュを無効化する。
    void append(unsigned char c) {
        assert(length_ < (std::numeric_limits<int>::max() - 1) / 2);
        int current = static_cast<int>(states_.size()); states_.push_back({});
        states_[current].length = ++length_; states_[current].terminal = 1;
        int p = last_;
        while (p != -1 && !states_[p].next.contains(c)) { states_[p].next[c] = current; p = states_[p].link; }
        if (p == -1) states_[current].link = 0;
        else {
            int q = states_[p].next[c];
            if (states_[p].length + 1 == states_[q].length) states_[current].link = q;
            else {
                int clone = static_cast<int>(states_.size());
                State copy = states_[q]; copy.length = states_[p].length + 1; copy.terminal = 0;
                states_.push_back(std::move(copy));
                while (p != -1) {
                    auto it = states_[p].next.find(c);
                    if (it == states_[p].next.end() || it->second != q) break;
                    it->second = clone; p = states_[p].link;
                }
                states_[q].link = states_[current].link = clone;
            }
        }
        distinct_ += states_[current].length - states_[states_[current].link].length;
        last_ = current; dirty_ = true;
    }
    /// @brief 現在の文字列長。O(1)。
    int size() const { return length_; }
    /// @brief 状態数。O(1)。
    int state_count() const { return static_cast<int>(states_.size()); }
    /// @brief pattern が部分文字列として存在するか。O(M log(A+1))。
    bool contains(std::string_view pattern) const { return locate(pattern) != -1; }
    /// @brief 異なる非空部分文字列数。O(1)。
    long long distinct_substrings() const { return distinct_; }
    /// @brief pattern の重複を含む出現数。追加後初回は O(N+M log(A+1))、以後 O(M log(A+1))。
    long long occurrences(std::string_view pattern) {
        if (pattern.empty()) return static_cast<long long>(length_) + 1;
        if (dirty_) rebuild_counts();
        int v = locate(pattern); return v == -1 ? 0 : counts_[v];
    }
private:
    struct State { int length = 0, link = -1, terminal = 0; std::map<unsigned char, int> next; };
    std::vector<State> states_;
    std::vector<long long> counts_;
    int last_ = 0, length_ = 0;
    long long distinct_ = 0;
    bool dirty_ = true;
    int locate(std::string_view pattern) const {
        int v = 0;
        for (unsigned char c : pattern) {
            auto it = states_[v].next.find(c); if (it == states_[v].next.end()) return -1; v = it->second;
        }
        return v;
    }
    void rebuild_counts() {
        std::vector<int> bucket(static_cast<std::size_t>(length_) + 1), order(states_.size());
        counts_.resize(states_.size());
        for (std::size_t i = 0; i < states_.size(); ++i) { ++bucket[states_[i].length]; counts_[i] = states_[i].terminal; }
        for (int i = 1; i <= length_; ++i) bucket[i] += bucket[i - 1];
        for (std::size_t i = 0; i < states_.size(); ++i) order[--bucket[states_[i].length]] = static_cast<int>(i);
        for (auto it = order.rbegin(); it != order.rend(); ++it) if (*it) counts_[states_[*it].link] += counts_[*it];
        dirty_ = false;
    }
};
