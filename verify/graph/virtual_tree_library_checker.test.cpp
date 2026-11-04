#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "../../library/graph/virtual_tree.hpp"
#include <iostream>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<std::vector<int>>tree(n);for(int v=1;v<n;++v){int p;std::cin>>p;tree[p].push_back(v);tree[v].push_back(p);}VirtualTree vt(tree);while(q--){int u,v;std::cin>>u>>v;std::cout<<vt.build({u,v}).vertices.front()<<'\n';}}
