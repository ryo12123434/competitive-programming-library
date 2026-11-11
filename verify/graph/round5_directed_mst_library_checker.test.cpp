#define PROBLEM "https://judge.yosupo.jp/problem/directedmst"
#include "../../library/graph/directed_mst.hpp"
#include "../../library/math/int128_io.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,m,s;std::cin>>n>>m>>s;std::vector<DirectedMSTEdge>edges(m);for(auto&e:edges)std::cin>>e.from>>e.to>>e.cost;auto result=directed_mst(n,s,edges);if(!result)return 1;print_int128(std::cout,result->cost)<<"\n";for(int i=0;i<n;++i)std::cout<<(i==s?s:result->parent[i])<<" ";std::cout<<"\n";}
