#include "library/graph/offline_dynamic_connectivity.hpp"
#include "library/graph/virtual_tree.hpp"
#include "library/graph/dsu_on_tree.hpp"
#include "library/graph/two_edge_connected_components.hpp"
#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <string>

std::vector<int> labels(int n,const std::vector<std::pair<int,int>>&edges,int skip=-1){
    std::vector<std::vector<int>>g(n);for(int i=0;i<static_cast<int>(edges.size());++i)if(i!=skip){auto[u,v]=edges[i];g[u].push_back(v);g[v].push_back(u);}
    std::vector<int>c(n,-1);for(int v=0;v<n;++v)if(c[v]==-1){std::queue<int>q;q.push(v);c[v]=v;while(!q.empty()){int u=q.front();q.pop();for(int to:g[u])if(c[to]==-1){c[to]=v;q.push(to);}}}return c;
}
void dynamic(){
    std::mt19937 rng(76853);
    OfflineDynamicConnectivity zero(0);zero.query_components();assert(zero.solve()==std::vector<int>{0});
    for(int trial=0;trial<140;++trial){int n=1+trial%15;OfflineDynamicConnectivity graph(n);std::vector<int>expected;std::map<std::pair<int,int>,int>active;
        for(int step=0;step<300;++step){int u=rng()%n,v=rng()%n;if(u>v)std::swap(u,v);auto edge=std::pair(u,v);int kind=rng()%4;
            if(kind==0){graph.add_edge(u,v);++active[edge];}
            if(kind==1){bool ok=active.contains(edge);assert(graph.remove_edge(v,u)==ok);if(ok&&--active[edge]==0)active.erase(edge);}
            if(kind>=2){std::vector<std::pair<int,int>>edges;for(auto[e,c]:active){(void)c;edges.push_back(e);}auto c=labels(n,edges);int id;
                if(kind==2){id=graph.query_same(u,v);expected.push_back(c[u]==c[v]);}
                else{id=graph.query_components();expected.push_back(std::set<int>(c.begin(),c.end()).size());}
                assert(id==static_cast<int>(expected.size())-1);}
            if(step%80==0)assert(graph.solve()==expected);
        }
        assert(graph.solve()==expected&&graph.solve()==expected);
    }
    OfflineDynamicConnectivity multi(2);multi.add_edge(0,1);multi.add_edge(1,0);multi.remove_edge(0,1);multi.query_same(0,1);multi.remove_edge(1,0);multi.query_same(0,1);assert((multi.solve()==std::vector<int>{1,0}));
}
void virtual_dsu(){
    std::mt19937 rng(5132);
    for(int trial=0;trial<180;++trial){int n=1+trial%34,root=rng()%n;std::vector<std::vector<int>>g(n);
        for(int v=1;v<n;++v){int p=trial%3==0?v-1:trial%3==1?0:rng()%v;g[v].push_back(p);g[p].push_back(v);}
        std::vector<int>parent(n,-1),depth(n),order{root};
        for(std::size_t i=0;i<order.size();++i)for(int to:g[order[i]])if(to!=parent[order[i]]){parent[to]=order[i];depth[to]=depth[order[i]]+1;order.push_back(to);}
        auto ancestor=[&](int a,int v){while(v!=-1){if(v==a)return true;v=parent[v];}return false;};
        auto lca=[&](int a,int b){while(depth[a]>depth[b])a=parent[a];while(depth[b]>depth[a])b=parent[b];while(a!=b){a=parent[a];b=parent[b];}return a;};
        VirtualTree tree(g,root);
        for(int q=0;q<60;++q){std::vector<int>selected;int k=rng()%(n+2);for(int i=0;i<k;++i)selected.push_back(rng()%n);auto result=tree.build(selected);std::set<int>expected(selected.begin(),selected.end());
            for(int a:selected)for(int b:selected)expected.insert(lca(a,b));
            assert(std::set<int>(result.vertices.begin(),result.vertices.end())==expected);
            for(int i=0;i<static_cast<int>(result.vertices.size());++i){int v=result.vertices[i],p=parent[v];while(p!=-1&&!expected.contains(p))p=parent[p];
                if(p==-1)assert(i==0&&result.parent[i]==-1&&result.distance[i]==0);
                else{assert(0<=result.parent[i]&&result.parent[i]<i);assert(result.vertices[result.parent[i]]==p&&result.distance[i]==depth[v]-depth[p]);}}
        }
        std::vector<int>color(n),counts(n),active(n),result(n,-1);int distinct=0,answers=0;
        for(int&c:color)c=rng()%n;
        dsu_on_tree(g,[&](int v){assert(!active[v]);active[v]=1;if(counts[color[v]]++==0)++distinct;},
            [&](int v){assert(active[v]);active[v]=0;if(--counts[color[v]]==0)--distinct;},
            [&](int v){++answers;std::set<int>cs;for(int u=0;u<n;++u){assert(active[u]==ancestor(v,u));if(active[u])cs.insert(color[u]);}assert(static_cast<int>(cs.size())==distinct);result[v]=distinct;},root);
        assert(answers==n&&distinct==0&&std::accumulate(active.begin(),active.end(),0)==0);
    }
    int calls=0;dsu_on_tree(std::vector<std::vector<int>>{},[&](int){++calls;},[&](int){++calls;},[&](int){++calls;});assert(calls==0);
}
void components(){
    std::mt19937 rng(9033);
    for(int trial=0;trial<700;++trial){int n=trial%18,m=n?rng()%45:0;Lowlink graph(n);std::vector<std::pair<int,int>>edges;
        for(int i=0;i<m;++i){int u=rng()%n,v=rng()%n;graph.add_edge(u,v);edges.emplace_back(u,v);}graph.build();TwoEdgeConnectedComponents cc(graph);
        auto base=labels(n,edges);int base_count=std::set<int>(base.begin(),base.end()).size();std::vector<bool>bridge(m);std::vector<std::pair<int,int>>remaining;
        for(int i=0;i<m;++i){auto c=labels(n,edges,i);bridge[i]=static_cast<int>(std::set<int>(c.begin(),c.end()).size())>base_count;if(!bridge[i])remaining.push_back(edges[i]);}
        auto expected=labels(n,remaining);for(int u=0;u<n;++u)for(int v=0;v<n;++v)assert((cc.component[u]==cc.component[v])==(expected[u]==expected[v]));
        std::vector<int>seen(n),edge_count(m);
        for(int c=0;c<cc.component_count();++c){for(int v:cc.groups[c]){assert(cc.component[v]==c);++seen[v];}for(auto e:cc.forest[c]){assert(bridge[e.edge_id]&&e.to!=c);++edge_count[e.edge_id];auto[u,v]=edges[e.edge_id];assert((cc.component[u]==c&&cc.component[v]==e.to)||(cc.component[v]==c&&cc.component[u]==e.to));}}
        for(int x:seen)assert(x==1);
        for(int i=0;i<m;++i)assert(edge_count[i]==(bridge[i]?2:0));
    }
}
void large(){const int n=200000;std::vector<std::vector<int>>g(n);for(int v=1;v<n;++v){g[v-1].push_back(v);g[v].push_back(v-1);}VirtualTree tree(g);auto t=tree.build({n-1,n/2,0});assert(t.vertices.size()==3&&t.distance.back()==n-1-n/2);int size=0;dsu_on_tree(g,[&](int){++size;},[&](int){--size;},[&](int v){assert(size==n-v);});assert(size==0);Lowlink graph(n);for(int v=1;v<n;++v)graph.add_edge(v-1,v);graph.build();TwoEdgeConnectedComponents cc(graph);assert(cc.component_count()==n);OfflineDynamicConnectivity dc(n);for(int v=1;v<n;++v)dc.add_edge(v-1,v);dc.query_same(0,n-1);dc.query_components();assert((dc.solve()==std::vector<int>{1,1}));}
int main(int argc,char**argv){std::string mode=argc>1?argv[1]:"all";if(mode=="all"||mode=="dynamic")dynamic();if(mode=="all"||mode=="virtual_dsu")virtual_dsu();if(mode=="all"||mode=="components")components();if(mode=="large")large();}
