#define PROBLEM "https://judge.yosupo.jp/problem/runenumerate"
#include "../../library/string/runs.hpp"
#include <atcoder/string>
#include <iostream>
#include <tuple>
SuffixArrayQuery query(const std::string&s){auto a=SuffixArrayQuery::encode(s);auto sa=atcoder::suffix_array(a,255);return SuffixArrayQuery(s,sa,a.empty()?std::vector<int>{}:atcoder::lcp_array(a,sa));}
int main(){std::string s;std::cin>>s;auto forward=query(s),reversed=query(std::string(s.rbegin(),s.rend()));auto runs=enumerate_runs(forward,reversed);std::sort(runs.begin(),runs.end(),[](auto a,auto b){return std::tie(a.period,a.l,a.r)<std::tie(b.period,b.l,b.r);});std::cout<<runs.size()<<"\n";for(auto r:runs)std::cout<<r.period<<" "<<r.l<<" "<<r.r<<"\n";}
