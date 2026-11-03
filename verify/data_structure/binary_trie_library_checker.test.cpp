#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#include <iostream>
#include "../../library/data_structure/binary_trie.hpp"
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    BinaryTrie<std::uint32_t,30> trie;int q;std::cin>>q;
    while(q--) {
        int type;std::uint32_t x;std::cin>>type>>x;
        if(type==0){if(!trie.count(x))trie.insert(x);} // judge は集合、ライブラリは多重集合
        else if(type==1)trie.erase(x);
        else std::cout<<*trie.min_xor(x)<<'\n';
    }
}
