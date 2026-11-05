#define PROBLEM "https://judge.yosupo.jp/problem/two_edge_connected_components"
#include "../../library/graph/two_edge_connected_components.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,m;std::cin>>n>>m;Lowlink graph(n);while(m--){int u,v;std::cin>>u>>v;graph.add_edge(u,v);}graph.build();TwoEdgeConnectedComponents cc(graph);std::cout<<cc.component_count()<<'\n';for(auto&group:cc.groups){std::cout<<group.size();for(int v:group)std::cout<<' '<<v;std::cout<<'\n';}}
