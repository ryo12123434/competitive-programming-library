#pragma once

#include <cassert>
#include <string>
#include <vector>

/**
 * @brief base から始まる連続した char_size 種類の文字を管理する Trie 木。
 * @pre 各文字のバイト値は [base, base + char_size)。ノード数と挿入数は int に収まること。
 * @note 重複する単語と空文字列も個別に数える。空間計算量は O(char_size * ノード数 + 挿入数)。
 *
 * @tparam char_size アルファベットの文字数。正で、base + char_size <= 256。
 * @tparam base アルファベットの先頭の非負バイト値（例: 小文字なら 'a'）。
 * @note search_LCP は挿入回数を基準とする。重複した単語も別の挿入として数える。
 *
 * @code
 * #include "library/string/trie.hpp"
 * #include <cassert>
 * int main() {
 *     Trie<26, 'a'> trie;
 *     trie.insert("abc");
 *     trie.insert("abd");
 *     assert(trie.search_word("abc"));
 *     assert(trie.search_prefix("ab"));
 *     assert(trie.search_LCP("abc") == 2);
 * }
 * @endcode
 */
template <int char_size, int base>
struct Trie {
    static_assert(char_size > 0 && base >= 0 && base + char_size <= 256);
    /**
     * @brief 文字遷移と終端の単語 ID、通過する挿入回数を保持するノード。
     * @note next はアルファベット内の添字から遷移先（-1 は未登録）、accept は終端 ID、common は通過回数。通常は直接変更しない。
     * @code
     * #include "library/string/trie.hpp"
     * #include <cassert>
     * int main() {
     *     Trie<26, 'a'>::Node root(-1);
     *     assert(root.common == 0);
     * }
     * @endcode
     */
    struct Node {
        std::vector<int> next, accept;
        int c, common;
        /**
         * @brief 文字 c に対応する未接続のノードを生成する。
         * @param c アルファベット内の 0-indexed の添字。根には -1 を使う。
         * @par Complexity
         * O(char_size) 時間・空間。
         */
        explicit Node(int c) : next(char_size, -1), c(c), common(0) {}
    };
    std::vector<Node> nodes;
    int root;

    /// @brief 空の Trie 木を O(char_size) で構築する。
    Trie() : root(0) { nodes.emplace_back(-1); }

    /**
     * @brief 指定した ID で word を挿入する。償却 O(char_size * (|word| + 1))。
     * @param word 挿入する単語。空文字列も可。
     * @param word_id 終端に保存する任意の ID。重複 ID も許す。
     * @par Complexity
     * 償却 O(char_size * (|word| + 1)) 時間。
     */
    void insert(const std::string& word, int word_id) {
        int v = root;
        for (unsigned char ch : word) {
            int c = static_cast<int>(ch) - base;
            assert(0 <= c && c < char_size);
            if (nodes[v].next[c] == -1) {
                nodes[v].next[c] = static_cast<int>(nodes.size());
                nodes.emplace_back(c);
            }
            ++nodes[v].common;
            v = nodes[v].next[c];
        }
        ++nodes[v].common;
        nodes[v].accept.push_back(word_id);
    }

    /**
     * @brief 0-indexed の挿入順を ID として word を挿入する。
     * @param word 挿入する単語。空文字列も可。
     * @note 明示 ID の挿入も数えて自動 ID を付ける。ID の一意性は呼び出し側で管理する。
     * @par Complexity
     * 償却 O(char_size * (|word| + 1)) 時間。
     */
    void insert(const std::string& word) { insert(word, count()); }

    /**
     * @brief word の存在を O(|word|) で判定する。prefix が true なら接頭辞として判定する。
     * @param word 調べる文字列。
     * @param prefix true なら接頭辞、false なら単語の終端として検索する。
     * @return 登録があれば true。prefix が false のとき、空単語は insert("") した場合に限り存在する。
     */
    bool search_word(const std::string& word, bool prefix = false) const {
        int v = root;
        for (unsigned char ch : word) {
            int c = static_cast<int>(ch) - base;
            assert(0 <= c && c < char_size);
            v = nodes[v].next[c];
            if (v == -1) return false;
        }
        return prefix || !nodes[v].accept.empty();
    }

    /**
     * @brief 接頭辞の存在を O(|prefix|) で判定する。空の接頭辞は常に存在する。
     * @param prefix 調べる接頭辞。
     * @return 接頭辞の経路があれば true。空の接頭辞は常に true。
     */
    bool search_prefix(const std::string& prefix) const { return search_word(prefix, true); }

    /**
     * @brief 2 回以上の挿入で共有される word の最長接頭辞の長さを O(|word|) で返す。
     * @note word を先に挿入すると、他の挿入済み単語との最大 LCP を得られる。
     *
     * @param word 調べる文字列。他の単語との最大 LCP を求める場合は、この単語も先に挿入する。
     * @return 2 回以上の挿入に共有される最長接頭辞の文字数。該当なしは 0。
     */
    int search_LCP(const std::string& word) const {
        int v = root, length = 0;
        for (unsigned char ch : word) {
            int c = static_cast<int>(ch) - base;
            assert(0 <= c && c < char_size);
            int next = nodes[v].next[c];
            if (next == -1 || nodes[next].common <= 1) break;
            v = next;
            ++length;
        }
        return length;
    }

    /**
     * @brief 重複を含む挿入済み単語数を O(1) で返す。
     * @return 重複・空単語を含めた挿入回数。
     */
    int count() const { return nodes[root].common; }
    /**
     * @brief 根を含むノード数を O(1) で返す。
     * @return 根を含むノード数（空の Trie では 1）。
     */
    int size() const { return static_cast<int>(nodes.size()); }
};
