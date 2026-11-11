#define PROBLEM "https://judge.yosupo.jp/problem/connected_components_of_complement_graph"
#include "../../library/graph/complement_graph.hpp"
#include "../../library/math/int128_io.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,m;std::cin>>n>>m;std::vector<std::pair<int,int>>edges(m);for(auto&[u,v]:edges)std::cin>>u>>v;auto groups=ComplementGraph(n,edges).components();std::cout<<groups.size()<<"\n";for(auto&c:groups){std::cout<<c.size();for(int v:c)std::cout<<" "<<v;std::cout<<"\n";}}
