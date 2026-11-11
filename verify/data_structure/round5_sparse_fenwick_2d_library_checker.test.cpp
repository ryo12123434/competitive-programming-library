#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#include "../../library/data_structure/sparse_fenwick_tree_2d.hpp"
#include <iostream>
#include <tuple>
struct Query{int type;long long a,b,c,d;};
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<std::tuple<long long,long long,long long>>initial;std::vector<std::pair<long long,long long>>points;for(int i=0;i<n;++i){long long x,y,w;std::cin>>x>>y>>w;initial.emplace_back(y,x,w);points.emplace_back(y,x);}std::vector<Query>queries;while(q--){int t;std::cin>>t;Query a{t,0,0,0,0};if(t==0){std::cin>>a.a>>a.b>>a.c;points.emplace_back(a.b,a.a);}else std::cin>>a.a>>a.b>>a.c>>a.d;queries.push_back(a);}SparseFenwickTree2D<long long>bit(points);for(auto[y,x,w]:initial)bit.add(y,x,w);for(auto a:queries)if(a.type==0)bit.add(a.b,a.a,a.c);else std::cout<<bit.sum(a.b,a.d,a.a,a.c)<<"\n";}
