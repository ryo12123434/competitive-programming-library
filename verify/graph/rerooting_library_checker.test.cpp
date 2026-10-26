#define PROBLEM "https://judge.yosupo.jp/problem/tree_path_composite_sum"
#include <atcoder/modint>
#include <iostream>
#include <vector>
#include "../../library/graph/rerooting.hpp"
using Mint=atcoder::modint998244353;
struct State { Mint sum,count; };
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n;std::cin>>n;std::vector<Mint> values(n),b(n-1),c(n-1);
    for(auto& x:values){int a;std::cin>>a;x=a;}
    std::vector<std::vector<RerootingEdge>> tree(n);
    for(int i=0;i<n-1;++i){int u,v,x,y;std::cin>>u>>v>>x>>y;b[i]=x;c[i]=y;tree[u].push_back({v,i});tree[v].push_back({u,i});}
    auto result=rerooting<State>(tree,[](State a,State b){return State{a.sum+b.sum,a.count+b.count};},State{0,0},
        [&](State a,int v){return State{a.sum+values[v],a.count+1};},
        [&](State a,int,int,int id){return State{a.sum*b[id]+a.count*c[id],a.count};});
    for(int v=0;v<n;++v)std::cout<<result[v].sum.val()<<(v==n-1?'\n':' ');
}
