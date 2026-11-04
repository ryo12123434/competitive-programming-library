#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#include "../../library/data_structure/segment_tree_beats.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;SegmentTreeBeats seg(a);while(q--){int t,l,r;std::cin>>t>>l>>r;if(t==3)std::cout<<static_cast<long long>(seg.range_sum(l,r))<<'\n';else{long long x;std::cin>>x;if(t==0)seg.range_chmin(l,r,x);if(t==1)seg.range_chmax(l,r,x);if(t==2)seg.range_add(l,r,x);}}}
