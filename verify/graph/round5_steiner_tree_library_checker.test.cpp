#define PROBLEM "https://judge.yosupo.jp/problem/minimum_steiner_tree"
#include "../../library/graph/steiner_tree.hpp"
#include "../../library/math/int128_io.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,m;std::cin>>n>>m;std::vector<SteinerTreeEdge>edges(m);for(auto&e:edges)std::cin>>e.u>>e.v>>e.cost;int k;std::cin>>k;std::vector<int>term(k);for(auto&v:term)std::cin>>v;auto result=steiner_tree_with_edges(n,edges,term);if(!result)return 1;print_int128(std::cout,result->cost)<<" "<<result->edge_ids.size()<<"\n";for(int id:result->edge_ids)std::cout<<id<<" ";std::cout<<"\n";}
