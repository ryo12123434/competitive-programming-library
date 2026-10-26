#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#include "../../library/data_structure/rollback_union_find.hpp"
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,q;std::cin>>n>>q;
    struct Query{int type=2,u=0,v=0;};
    std::vector<Query> queries(q+1);std::vector<std::vector<int>> children(q+1);
    for(int i=1;i<=q;++i){int k;auto& x=queries[i];std::cin>>x.type>>k>>x.u>>x.v;children[k+1].push_back(i);}
    RollbackUnionFind uf(n);std::vector<RollbackUnionFind::Snapshot> saved(q+1);std::vector<int> answers(q+1);
    std::vector<std::pair<int,bool>> stack{{0,false}};
    while(!stack.empty()) {
        const auto [node,exit]=stack.back();stack.pop_back();
        if(exit){[[maybe_unused]] const bool ok=uf.rollback(saved[node]);assert(ok);continue;}
        saved[node]=uf.snapshot();const auto query=queries[node];
        if(query.type==0)uf.merge(query.u,query.v);
        if(query.type==1)answers[node]=uf.same(query.u,query.v);
        stack.emplace_back(node,true);
        for(int child:children[node])stack.emplace_back(child,false);
    }
    for(int i=1;i<=q;++i)if(queries[i].type==1)std::cout<<answers[i]<<'\n';
}
