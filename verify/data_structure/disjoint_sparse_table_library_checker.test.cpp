#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include "../../library/data_structure/disjoint_sparse_table.hpp"
#include <iostream>
long long op(long long a,long long b){return a+b;}
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;DisjointSparseTable<long long,op>table(a);while(q--){int l,r;std::cin>>l>>r;std::cout<<table.prod(l,r)<<'\n';}}
