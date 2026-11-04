#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_14_B"
#include "../../library/string/kmp.hpp"
#include <iostream>
#include <string>
int main(){std::string text,pattern;std::cin>>text>>pattern;for(int i:kmp_search(text,pattern))std::cout<<i<<'\n';}
