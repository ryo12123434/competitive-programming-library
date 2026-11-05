#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "../../library/string/suffix_automaton.hpp"
#include <iostream>
#include <string>
int main(){std::string s;std::cin>>s;std::cout<<SuffixAutomaton(s).distinct_substrings()<<'\n';}
