#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_4_C"

#include <iostream>
#include "../../library/string/trie.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    Trie<26, 'A'> trie;
    while (n--) {
        std::string op, word;
        std::cin >> op >> word;
        if (op == "insert") trie.insert(word);
        else std::cout << (trie.search_word(word) ? "yes" : "no") << '\n';
    }
}
