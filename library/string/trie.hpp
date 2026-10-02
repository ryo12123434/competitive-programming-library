#pragma once

#include <cassert>
#include <string>
#include <vector>

/**
 * @brief base から始まる連続した char_size 種類の文字を管理する Trie 木。
 * @pre 各文字のバイト値は [base, base + char_size)。ノード数と挿入数は int に収まること。
 * @note 重複する単語と空文字列も個別に数える。空間計算量は O(char_size * ノード数 + 挿入数)。
 */
template <int char_size, int base>
struct Trie {
    static_assert(char_size > 0 && base >= 0 && base + char_size <= 256);
    struct Node {
        std::vector<int> next, accept;
        int c, common;
        explicit Node(int c) : next(char_size, -1), c(c), common(0) {}
    };
    std::vector<Node> nodes;
    int root;

    /// @brief 空の Trie 木を O(char_size) で構築する。
    Trie() : root(0) { nodes.emplace_back(-1); }

    /// @brief 指定した ID で word を挿入する。償却 O(char_size * |word|)。
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

    /// @brief 0-indexed の挿入順を ID として word を挿入する。
    void insert(const std::string& word) { insert(word, count()); }

    /// @brief word の存在を O(|word|) で判定する。prefix が true なら接頭辞として判定する。
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

    /// @brief 接頭辞の存在を O(|prefix|) で判定する。空の接頭辞は常に存在する。
    bool search_prefix(const std::string& prefix) const { return search_word(prefix, true); }

    /**
     * @brief 2 回以上の挿入で共有される word の最長接頭辞の長さを O(|word|) で返す。
     * @note word を先に挿入すると、他の挿入済み単語との最大 LCP を得られる。
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

    /// @brief 重複を含む挿入済み単語数を O(1) で返す。
    int count() const { return nodes[root].common; }
    /// @brief 根を含むノード数を O(1) で返す。
    int size() const { return static_cast<int>(nodes.size()); }
};
