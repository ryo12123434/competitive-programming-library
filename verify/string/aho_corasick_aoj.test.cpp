#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_14_D"
#include "../../library/string/aho_corasick.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);std::string text;int q;std::cin>>text>>q;std::vector<std::string>patterns(q);std::array<bool,256>used{};for(auto&p:patterns){std::cin>>p;for(unsigned char c:p)used[c]=true;}std::string alphabet;for(int i=0;i<256;++i)if(used[i])alphabet+=static_cast<char>(i);AhoCorasick ac(alphabet);for(auto&p:patterns)ac.add_pattern(p);ac.build();for(auto c:ac.count_occurrences(text))std::cout<<(c!=0)<<'\n';}
