#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include "../../library/string/suffix_array_query.hpp"
#include <atcoder/string>
#include <iostream>
int main(){std::string s;std::cin>>s;auto a=SuffixArrayQuery::encode(s);auto sa=atcoder::suffix_array(a,255);SuffixArrayQuery q(s,sa,a.empty()?std::vector<int>{}:atcoder::lcp_array(a,sa));long long answer=1LL*s.size()*(s.size()+1)/2;for(int i=1;i<q.size();++i)answer-=q.lcp(sa[i-1],sa[i]);std::cout<<answer<<"\n";}
