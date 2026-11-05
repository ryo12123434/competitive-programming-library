#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include "../../library/data_structure/persistent_segment_tree.hpp"
#include <atcoder/modint>
#include <iostream>
using Mint=atcoder::modint998244353;
struct Affine{Mint a,b;};
Affine op(Affine f,Affine g){return {g.a*f.a,g.a*f.b+g.b};}
Affine e(){return {1,0};}
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<Affine>a(n);for(auto&f:a){int c,d;std::cin>>c>>d;f={c,d};}PersistentSegmentTree<Affine,op,e>seg(a);int version=0;while(q--){int t;std::cin>>t;if(t==0){int p,c,d;std::cin>>p>>c>>d;version=seg.set(version,p,{c,d});}else{int l,r,x;std::cin>>l>>r>>x;auto f=seg.prod(version,l,r);std::cout<<(f.a*x+f.b).val()<<'\n';}}}
