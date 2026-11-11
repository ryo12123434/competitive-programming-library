#define PROBLEM "https://atcoder.jp/contests/abc331/tasks/abc331_f"
#include "../../library/string/dynamic_rolling_hash.hpp"
#include <iostream>
#include <string>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::string s;std::cin>>n>>q>>s;DynamicRollingHash forward(s);DynamicRollingHash reversed(std::string(s.rbegin(),s.rend()),forward.base());while(q--){int type;std::cin>>type;if(type==1){int x;char c;std::cin>>x>>c;forward.set(x-1,c);reversed.set(n-x,c);}else{int l,r;std::cin>>l>>r;std::cout<<(forward.get(l-1,r)==reversed.get(n-r,n-l+1)?"Yes":"No")<<"\n";}}}
