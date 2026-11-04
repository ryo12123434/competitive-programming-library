#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include "../../library/graph/offline_dynamic_connectivity.hpp"
#include <iostream>
struct Query{int type,u,v;};
int main(){
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,q;std::cin>>n>>q;std::vector<Query>queries(q+1);std::vector<std::vector<int>>children(q+1);
    for(int i=1;i<=q;++i){int k;auto&x=queries[i];std::cin>>x.type>>k>>x.u>>x.v;children[k+1].push_back(i);}
    OfflineDynamicConnectivity graph(n);std::vector<int>answer_id(q+1,-1);std::vector<std::pair<int,bool>>stack{{0,false}};
    while(!stack.empty()){
        auto[v,exit]=stack.back();stack.pop_back();auto x=queries[v];
        if(exit){if(v&&x.type==0)graph.remove_edge(x.u,x.v);continue;}
        if(v){if(x.type==0)graph.add_edge(x.u,x.v);else answer_id[v]=graph.query_same(x.u,x.v);}
        stack.push_back({v,true});for(int to:children[v])stack.push_back({to,false});
    }
    auto answers=graph.solve();for(int i=1;i<=q;++i)if(answer_id[i]!=-1)std::cout<<answers[answer_id[i]]<<'\n';
}
