#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_E"
#include "../../library/data_structure/dual_segment_tree.hpp"
#include <iostream>
long long mapping(long long f,long long x){return f+x;}long long composition(long long f,long long g){return f+g;}long long id(){return 0;}
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;DualSegmentTree<long long,long long,mapping,composition,id>tree(std::vector<long long>(n,0));while(q--){int type;std::cin>>type;if(type==0){int l,r;long long x;std::cin>>l>>r>>x;tree.apply(l-1,r,x);}else{int p;std::cin>>p;std::cout<<tree.get(p-1)<<"\n";}}}
