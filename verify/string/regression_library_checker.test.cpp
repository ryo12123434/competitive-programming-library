#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// A + B を入出力用に使い、Trie の重複・空文字列とハッシュの部分列・連結を検証する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include "../../library/string/trie.hpp"
#include "../../library/string/rolling_hash.hpp"

int main() {
    Trie<26, 'a'> trie;
    assert(trie.count() == 0 && trie.size() == 1);
    assert(trie.search_prefix("") && !trie.search_word(""));
    trie.insert(""); trie.insert("abc", 42); trie.insert("abc"); trie.insert("abd");
    assert(trie.count() == 4 && trie.size() == 5);
    assert(trie.search_word("") && trie.search_word("abc") && !trie.search_word("ab"));
    assert(trie.search_prefix("ab") && !trie.search_prefix("ac"));
    assert(trie.search_LCP("abc") == 3 && trie.search_LCP("abd") == 2);
    assert((trie.nodes[3].accept == std::vector<int>{42, 2}));

    std::mt19937 rng(1703);
    Trie<3, 'a'> random_trie;
    std::vector<std::string> words;
    for (int i = 0; i < 300; ++i) {
        std::string word(rng() % 8, 'a');
        for (char& c : word) c += rng() % 3;
        random_trie.insert(word);
        words.push_back(word);
    }
    for (int i = 0; i < static_cast<int>(words.size()); ++i) {
        assert(random_trie.search_word(words[i]));
        int best = 0;
        for (int j = 0; j < static_cast<int>(words.size()); ++j) if (i != j) {
            int k = 0;
            while (k < static_cast<int>(std::min(words[i].size(), words[j].size())) && words[i][k] == words[j][k]) ++k;
            best = std::max(best, k);
        }
        assert(random_trie.search_LCP(words[i]) == best);
    }

    RollingHash empty("");
    assert(empty.get(0, 0) == 0);
    for (int trial = 0; trial < 100; ++trial) {
        std::string s(rng() % 20, '\0');
        for (char& c : s) c = static_cast<char>(rng() % 256);
        RollingHash h(s);
        int n = static_cast<int>(s.size());
        for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
            RollingHash sub(s.substr(l, r - l));
            assert(h.get(l, r) == sub.get(0, r - l));
            int m = (l + r) / 2;
            assert(RollingHash::connect(h.get(l, m), h.get(m, r), r - m, h.power) == h.get(l, r));
        }
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
