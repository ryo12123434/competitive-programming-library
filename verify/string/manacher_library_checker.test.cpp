#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_palindromes"
#include <iostream>
#include <string>
#include "../../library/string/manacher.hpp"
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    std::string s;std::cin>>s;auto pal=manacher(s);
    for(int i=0;i<static_cast<int>(s.size());++i) {
        if(i)std::cout<<' ';
        std::cout<<pal.odd[i]+pal.odd[i]-1;
        if(i+1<static_cast<int>(s.size()))std::cout<<' '<<2*pal.even[i+1];
    }
    std::cout<<'\n';
}
