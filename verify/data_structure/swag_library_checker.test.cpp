#define PROBLEM "https://judge.yosupo.jp/problem/queue_operate_all_composite"
#include "../../library/data_structure/swag.hpp"
#include <atcoder/modint>
#include <iostream>
using Mint=atcoder::modint998244353;struct Affine{Mint a,b;};
Affine op(Affine f,Affine g){return {g.a*f.a,g.a*f.b+g.b};}Affine e(){return {1,0};}
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int q;std::cin>>q;SWAG<Affine,op,e>queue;while(q--){int t;std::cin>>t;if(t==0){int a,b;std::cin>>a>>b;queue.push({a,b});}else if(t==1)queue.pop();else{int x;std::cin>>x;auto f=queue.fold();std::cout<<(f.a*x+f.b).val()<<'\n';}}}
