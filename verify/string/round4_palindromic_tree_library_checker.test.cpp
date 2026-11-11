#define PROBLEM "https://judge.yosupo.jp/problem/eertree"
#include "../../library/string/palindromic_tree.hpp"
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string s;
    std::cin >> s;
    PalindromicTree tree;
    std::vector<int> prefix;
    for (char c : s)
        prefix.push_back(tree.add_char(c) - 1);
    std::cout << tree.distinct_count() << "\n";
    for (int id = 2; id < tree.node_count(); ++id) {
        auto n = tree.node_info(id);
        std::cout << n.inner - 1 << " " << n.link - 1 << "\n";
    }
    for (int v : prefix)
        std::cout << v << " ";
    std::cout << "\n";
}
