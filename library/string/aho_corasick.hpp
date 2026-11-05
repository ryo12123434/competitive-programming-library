#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief 指定したバイト文字集合で複数パターンを同時検索する Aho-Corasick。
 * @note add_pattern 後に一度 build してから検索する。build 後のパターン追加は不可。
 * 重複パターンは別 ID。空パターンは文字間の全 N+1 境界に一致する。
 * テキスト中の集合外の文字は区切りとして扱う（空パターンはその境界にも一致）。
 * @pre パターンは指定文字集合内。文字集合に重複なし。各長さ・状態数・ID数は int に収まる。
 * @par Complexity
 * 総パターン長 L、状態数 S<=L+1、文字集合の大きさ A<=256、パターン数 P。
 * 全追加 O(L+S(A+1)+P)、build O(S(A+1))、保持空間 O(S(A+1)+P)。検索 O(N+出現数)。
 * count_occurrences は列挙せず O(N+S+パターン数) 時間・O(S+パターン数) 追加空間。
 * @code
 * #include "library/string/aho_corasick.hpp"
 * #include <cassert>
 * int main() {
 *     AhoCorasick ac("ab"); int id=ac.add_pattern("aba"); ac.add_pattern("ba"); ac.build();
 *     assert(ac.count_occurrences("ababa")[id]==2);
 *     ac.for_each_match("ababa", [](int pattern_id,int start){ (void)pattern_id; (void)start; });
 * }
 * @endcode
 */
class AhoCorasick {
public:
    /// @brief 一致の ID と開始位置。find_all() から取得する。
    struct Match { int pattern_id, position; };
    /// @brief alphabet の順で遷移表を構築。既定は英小文字。空集合も可。
    explicit AhoCorasick(std::string_view alphabet = "abcdefghijklmnopqrstuvwxyz") : alphabet_(alphabet) {
        index_.fill(-1);
        for (std::size_t i = 0; i < alphabet_.size(); ++i) {
            auto c = static_cast<unsigned char>(alphabet_[i]); assert(index_[c] == -1);
            index_[c] = static_cast<int>(i);
        }
        nodes_.emplace_back(alphabet_.size());
    }
    /// @brief パターンを登録し、登録順の ID を返す。@pre build 前、集合内の文字のみ。
    int add_pattern(std::string_view pattern) {
        assert(!built_ && pattern.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int v = 0;
        for (unsigned char c : pattern) {
            int k = index_[c]; assert(k != -1);
            if (nodes_[v].next[k] == -1) {
                assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
                int to = static_cast<int>(nodes_.size()); nodes_[v].next[k] = to;
                nodes_.emplace_back(alphabet_.size());
            }
            v = nodes_[v].next[k];
        }
        assert(terminal_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(terminal_.size());
        terminal_.push_back(v); lengths_.push_back(static_cast<int>(pattern.size())); nodes_[v].ids.push_back(id);
        return id;
    }
    /// @brief failure / 出力リンクと遷移を確定。2回目以降は何もしない。O(SA)。
    void build() {
        if (built_) return;
        std::queue<int> queue;
        for (int& to : nodes_[0].next) { if (to == -1) to = 0; else queue.push(to); }
        order_ = {0};
        while (!queue.empty()) {
            int v = queue.front(); queue.pop(); order_.push_back(v);
            int f = nodes_[v].fail;
            nodes_[v].output = !nodes_[f].ids.empty() ? f : nodes_[f].output;
            for (std::size_t k = 0; k < alphabet_.size(); ++k) {
                int& to = nodes_[v].next[k];
                if (to == -1) to = nodes_[f].next[k];
                else { nodes_[to].fail = nodes_[f].next[k]; queue.push(to); }
            }
        }
        built_ = true;
    }
    /**
     * @brief 一致ごとに callback(int pattern_id,int start) を呼ぶ。@pre build 済み、text.size()<=INT_MAX。
     * @note 終了位置の昇順。同じ終了位置の ID 順は保証しない。出力リンクで列挙し、ID全走査は行わない。
     * @par Complexity O(N+出現数)、callback のコストは別途。列挙の補助空間 O(1)。
     */
    template <class Callback>
    void for_each_match(std::string_view text, Callback callback) const {
        check_text(text); int v = 0;
        auto emit = [&](int state, int end) {
            for (int u = state; u != -1; u = nodes_[u].output)
                for (int id : nodes_[u].ids) callback(id, end - lengths_[id]);
        };
        emit(0, 0);
        for (std::size_t i = 0; i < text.size(); ++i) {
            int k = index_[static_cast<unsigned char>(text[i])]; v = k == -1 ? 0 : nodes_[v].next[k];
            emit(v, static_cast<int>(i + 1));
        }
    }
    /// @brief 一致を配列で取得。空間 O(出現数)。前提・順序は for_each_match と同じ。
    std::vector<Match> find_all(std::string_view text) const {
        std::vector<Match> result;
        for_each_match(text, [&](int id, int start) { result.push_back({id, start}); }); return result;
    }
    /// @brief ID ごとの出現数。failure 木で加算する。@pre build 済み、text.size()<=INT_MAX。
    std::vector<long long> count_occurrences(std::string_view text) const {
        check_text(text); std::vector<long long> count(nodes_.size()); count[0] = 1;
        int v = 0;
        for (unsigned char c : text) { int k = index_[c]; v = k == -1 ? 0 : nodes_[v].next[k]; ++count[v]; }
        for (auto it = order_.rbegin(); it != order_.rend(); ++it) if (*it) count[nodes_[*it].fail] += count[*it];
        std::vector<long long> result; result.reserve(terminal_.size());
        for (int u : terminal_) result.push_back(count[u]);
        return result;
    }
private:
    struct Node {
        std::vector<int> next, ids;
        int fail = 0, output = -1;
        explicit Node(std::size_t alphabet_size) : next(alphabet_size, -1) {}
    };
    std::string alphabet_;
    std::array<int, 256> index_;
    std::vector<Node> nodes_;
    std::vector<int> terminal_, lengths_, order_;
    bool built_ = false;
    void check_text([[maybe_unused]] std::string_view text) const {
        assert(built_ && text.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    }
};
