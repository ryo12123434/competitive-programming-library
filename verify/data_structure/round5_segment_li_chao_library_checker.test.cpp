#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"
#include "../../library/data_structure/segment_li_chao_tree.hpp"
#include "../../library/math/int128_io.hpp"
#include <iostream>
#include <array>
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);int n,q;std::cin>>n>>q;std::vector<std::array<long long,4>>initial(n);for(auto&a:initial)for(auto&x:a)std::cin>>x;struct Query{int type;std::array<long long,4>args;};std::vector<Query>queries;std::vector<long long>xs;while(q--){int type;std::cin>>type;Query a{type,{}};if(type==0)for(auto&x:a.args)std::cin>>x;else{std::cin>>a.args[0];xs.push_back(a.args[0]);}queries.push_back(a);}SegmentLiChaoTree tree(xs);for(auto a:initial)tree.add_segment(a[0],a[1],a[2],a[3]);for(auto a:queries)if(a.type==0)tree.add_segment(a.args[0],a.args[1],a.args[2],a.args[3]);else{auto v=tree.query(a.args[0]);if(v)print_int128(std::cout,*v)<<"\n";else std::cout<<"INFINITY\n";}}
