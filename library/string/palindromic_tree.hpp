#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief 回文木ノードの情報。lengthは長さ、linkは最長真回文接尾辞、innerは両端を除いた回文ID。
 * @note first_endは最初の出現の半開終端。特殊根0(長さ-1)・1(長さ0)ではinner=-1、first_end=0。
 * @code
 * #include "library/string/palindromic_tree.hpp"
 * #include <cassert>
 * int main(){PalindromicTree t("a");auto node=t.node_info(2);assert(node.length==1&&node.inner==0&&node.link==1);}
 * @endcode
 */
struct PalindromeNode {
    int length, link, inner, first_end;
};
/**
 * @brief 末尾追加可能なPalindromic Tree（Eertree）。全256バイトを扱い、センチネル文字は使わない。
 * @note Manacherは各中心の半径、本クラスは異なる回文とその接尾辞関係・出現回数を管理する。
 * ID0は奇根、1は偶根、2以降は初出順の非空回文。add_charは追加後の最長回文接尾辞IDを返す。
 * @pre 文字数+2と全IDがintに収まること。文字はUTF-8等でもバイト単位で扱う。
 * @par Complexity N文字・アルファベット数sigma<=256に対し全追加O(N log(sigma+1))、空間O(N)。
 * 1回の追加の最悪はO(N log(sigma+1))、occurrencesはO(N)時間・出力空間。
 * @code
 * #include "library/string/palindromic_tree.hpp"
 * #include <cassert>
 * int main(){PalindromicTree t("ababa");assert(t.distinct_count()==5&&t.node_info(t.longest_suffix()).length==5);auto count=t.occurrences();assert(count[2]==3);}
 * @endcode
 */
class PalindromicTree {
    std::string text_;
    std::vector<PalindromeNode> nodes_{{-1, 0, -1, 0}, {0, 0, -1, 0}};
    std::vector<std::map<unsigned char, int>> next_{2};
    std::vector<std::uint64_t> terminal_{0, 0};
    int last_ = 1;
    int extendable(int v, int position, unsigned char c) const {
        while (position - 1 - nodes_[v].length < 0 ||
               static_cast<unsigned char>(text_[position - 1 - nodes_[v].length]) != c)
            v = nodes_[v].link;
        return v;
    }

public:
    explicit PalindromicTree(std::string_view text = {}) {
        for (char c : text)
            add_char(c);
    }
    /// @brief 1バイト追加し、最長回文接尾辞のIDを返す。既存IDは不変。
    int add_char(char character) {
        assert(text_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max() - 2));
        unsigned char c = static_cast<unsigned char>(character);
        text_.push_back(character);
        int position = static_cast<int>(text_.size()) - 1;
        int inner = extendable(last_, position, c);
        auto it = next_[inner].find(c);
        if (it != next_[inner].end())
            last_ = it->second;
        else {
            int length = nodes_[inner].length + 2, link = 1;
            if (length > 1) {
                int v = extendable(nodes_[inner].link, position, c);
                link = next_[v].at(c);
            }
            last_ = static_cast<int>(nodes_.size());
            nodes_.push_back({length, link, inner, position + 1});
            next_.emplace_back();
            terminal_.push_back(0);
            next_[inner].emplace(c, last_);
        }
        ++terminal_[last_];
        return last_;
    }
    int longest_suffix() const { return last_; }
    int node_count() const { return static_cast<int>(nodes_.size()); }
    int distinct_count() const { return node_count() - 2; }
    /// @brief ノード情報をO(1)でコピーして返す。@pre 有効なノードID。
    PalindromeNode node_info(int id) const {
        assert(0 <= id && id < node_count());
        return nodes_[id];
    }
    /// @brief 各ノードの全出現回数を返す。特殊根2個は0。再呼出し・後からの追加も可能。
    std::vector<std::uint64_t> occurrences() const {
        auto result = terminal_;
        for (int id = node_count() - 1; id >= 2; --id)
            result[nodes_[id].link] += result[id];
        result[0] = result[1] = 0;
        return result;
    }
};
