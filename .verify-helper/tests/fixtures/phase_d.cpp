#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include "library/data_structure/rollback_union_find.hpp"
#include "library/data_structure/wavelet_matrix.hpp"

namespace {
void check(bool value) {if(!value)std::abort();}
std::mt19937_64 rng(20261026);
template<class T> void check_matrix(const std::vector<T>& values) {
    const int n=static_cast<int>(values.size());const WaveletMatrix<T> matrix(values);
    check(matrix.size()==n);
    for(int i=0;i<n;++i)check(matrix.access(i)==values[i]);
    std::vector<T> queries=values;
    for(T x:{std::numeric_limits<T>::min(),std::numeric_limits<T>::max(),T(0),T(1),T(-1)})queries.push_back(x);
    std::sort(queries.begin(),queries.end());queries.erase(std::unique(queries.begin(),queries.end()),queries.end());
    for(int l=0;l<=n;++l)for(int r=l;r<=n;++r) {
        std::vector<T> sorted(values.begin()+l,values.begin()+r);std::sort(sorted.begin(),sorted.end());
        for(int k=-1;k<=r-l;++k) {
            if(k<0 || k>=r-l){check(!matrix.kth_smallest(l,r,k) && !matrix.kth_largest(l,r,k));}
            else check(matrix.kth_smallest(l,r,k)==sorted[k] && matrix.kth_largest(l,r,k)==sorted[r-l-1-k]);
        }
        for(std::size_t i=0;i<queries.size();++i) {
            const T x=queries[i];const auto lo=std::lower_bound(sorted.begin(),sorted.end(),x),hi=std::upper_bound(sorted.begin(),sorted.end(),x);
            check(matrix.count(l,r,x)==hi-lo && matrix.count_less(l,r,x)==lo-sorted.begin());
            if(l==0)check(matrix.rank(x,r)==hi-lo);
            if(lo==sorted.begin())check(!matrix.prev_value(l,r,x));else check(matrix.prev_value(l,r,x)==*(lo-1));
            if(lo==sorted.end())check(!matrix.next_value(l,r,x));else check(matrix.next_value(l,r,x)==*lo);
            for(std::size_t j=i;j<queries.size();j+=1+queries.size()/5) {
                int expected=0;for(T v:sorted)expected+=x<=v && v<queries[j];
                check(matrix.range_freq(l,r,x,queries[j])==expected);
            }
        }
    }
}
void wavelet_tests() {
    for(int n=0;n<=6;++n) {
        int total=1;for(int i=0;i<n;++i)total*=3;
        for(int code=0;code<total;++code){int rest=code;std::vector<long long> a(n);for(auto& x:a){x=rest%3-1;rest/=3;}check_matrix(a);}
    }
    for(int trial=0;trial<80;++trial) {
        std::vector<long long> a(rng()%18);
        for(auto& x:a)x=trial%2?static_cast<long long>(rng()):static_cast<int>(rng()%21)-10;
        if(!a.empty()){a.front()=LLONG_MIN;a.back()=LLONG_MAX;}
        check_matrix(a);std::sort(a.begin(),a.end());check_matrix(a);std::reverse(a.begin(),a.end());check_matrix(a);
    }
    check_matrix<std::uint64_t>({0,UINT64_MAX,1,std::uint64_t{1}<<63,UINT64_MAX,0});
    check_matrix<std::int8_t>({INT8_MIN,0,INT8_MAX,-1,1});check_matrix<std::uint8_t>({0,255,127,128,255});
    check_matrix<std::int32_t>({INT32_MIN,0,INT32_MAX});check_matrix<std::uint32_t>({0,UINT32_MAX});
    for(int n:{63,64,65,127,128,129}) {
        std::vector<long long> a(n);for(auto& x:a)x=rng()%2;
        WaveletMatrix<> w(a);for(int r=0;r<=n;++r)for(int x=0;x<3;++x)
            check(w.rank(x,r)==std::count(a.begin(),a.begin()+r,x));
    }
    const int n=500000;std::vector<long long> values(n);
    for(int i=0;i<n;++i)values[i]=i-n/2;
    const WaveletMatrix<> dense(values);
    check(dense.bit_width()==19 && dense.kth_smallest(1,n,0)==1-n/2);
    check(dense.count_less(0,n,0)==n/2 && dense.access(n-1)==n-1-n/2);
    for(auto& x:values)x=std::bit_cast<long long>(rng());
    values[0]=LLONG_MIN;values[1]=LLONG_MAX;
    const WaveletMatrix<> wide(values);
    check(wide.bit_width()==64 && wide.kth_smallest(0,n,0)==LLONG_MIN && wide.kth_largest(0,n,0)==LLONG_MAX);
    for(int i=0;i<n;i+=997)check(wide.access(i)==values[i]);
    std::fill(values.begin(),values.end(),LLONG_MAX);const WaveletMatrix<> same(values);
    check(same.bit_width()==0 && same.rank(LLONG_MAX,n)==n && same.access(n-1)==LLONG_MAX);
}

std::vector<int> labels(int n,const std::vector<std::pair<int,int>>& edges) {
    std::vector<std::vector<int>> graph(n);for(auto [u,v]:edges){graph[u].push_back(v);graph[v].push_back(u);}
    std::vector<int> result(n,-1);
    for(int v=0;v<n;++v)if(result[v]==-1){std::vector<int> queue{v};result[v]=v;
        for(std::size_t i=0;i<queue.size();++i)for(int to:graph[queue[i]])if(result[to]==-1){result[to]=v;queue.push_back(to);}}
    return result;
}
void rollback_tests() {
    RollbackUnionFind empty(0);check(empty.components()==0 && !empty.undo());check(empty.rollback(empty.snapshot()));
    for(int trial=0;trial<80;++trial) {
        const int n=1+rng()%20;RollbackUnionFind uf(n),foreign(n);
        std::vector<std::pair<int,int>> edges;std::vector<std::uint64_t> ids;std::uint64_t generation=0;
        struct Saved {RollbackUnionFind::Snapshot token;std::vector<std::uint64_t> path;};
        std::vector<Saved> saved{{uf.snapshot(),{}}};
        for(int step=0;step<600;++step) {
            const int type=rng()%5;
            if(type<2){int u=rng()%n,v=rng()%n;auto before=labels(n,edges);check(uf.merge(u,v)==(before[u]!=before[v]));edges.emplace_back(u,v);ids.push_back(++generation);}
            else if(type==2){check(uf.undo()==!edges.empty());if(!edges.empty()){edges.pop_back();ids.pop_back();}}
            else if(type==3)saved.push_back({uf.snapshot(),ids});
            else {
                const auto& s=saved[rng()%saved.size()];const bool valid=s.path.size()<=ids.size() && std::equal(s.path.begin(),s.path.end(),ids.begin());
                check(uf.is_valid(s.token)==valid && uf.rollback(s.token)==valid);
                if(valid){edges.resize(s.path.size());ids.resize(s.path.size());}
            }
            check(!uf.rollback(foreign.snapshot()) && !uf.rollback(RollbackUnionFind::Snapshot{}));
            const auto component=labels(n,edges);std::set<int> unique(component.begin(),component.end());
            check(uf.components()==static_cast<int>(unique.size()));
            for(int u=0;u<n;++u){check(uf.size(u)==std::count(component.begin(),component.end(),component[u]));
                for(int v=0;v<n;++v)check(uf.same(u,v)==(component[u]==component[v]));}
        }
    }
    RollbackUnionFind branching(3);auto root=branching.snapshot();branching.merge(0,1);auto discarded=branching.snapshot();
    check(branching.rollback(root));branching.merge(1,2);check(!branching.is_valid(discarded) && !branching.rollback(discarded));
    const int n=200000;RollbackUnionFind large(n);auto original=large.snapshot();
    for(int v=1;v<n;++v)check(large.merge(v-1,v));
    check(large.components()==1 && large.size(0)==n);check(large.rollback(original) && large.components()==n);
}

void dynamic_connectivity() {
    // 区間木に辺の生存区間を載せる、オフライン動的連結性の統合テスト。
    const int n=15,q=100;std::vector<std::pair<int,int>> edges;
    std::vector<std::vector<std::pair<int,int>>> seg(4*q),at(q);
    std::function<void(int,int,int,int,int,std::pair<int,int>)> add=[&](int node,int l,int r,int a,int b,auto edge) {
        if(b<=l || r<=a)return;
        if(a<=l && r<=b){seg[node].push_back(edge);return;}
        int m=l+(r-l)/2;add(node*2,l,m,a,b,edge);add(node*2+1,m,r,a,b,edge);
    };
    for(int i=0;i<150;++i){int l=rng()%q,r=rng()%q;if(l>r)std::swap(l,r);auto edge=std::pair{int(rng()%n),int(rng()%n)};
        add(1,0,q,l,r,edge);for(int t=l;t<r;++t)at[t].push_back(edge);}
    RollbackUnionFind uf(n);
    std::function<void(int,int,int)> solve=[&](int node,int l,int r){auto token=uf.snapshot();
        for(auto [u,v]:seg[node])uf.merge(u,v);
        if(r-l==1){auto c=labels(n,at[l]);for(int u=0;u<n;++u)for(int v=0;v<n;++v)check(uf.same(u,v)==(c[u]==c[v]));}
        else{int m=l+(r-l)/2;solve(node*2,l,m);solve(node*2+1,m,r);}
        check(uf.rollback(token));};
    solve(1,0,q);check(uf.components()==n);
}
}
int main(int argc,char** argv) {
    const std::string mode=argc>1?argv[1]:"all";
    if(mode=="all"||mode=="wavelet")wavelet_tests();
    if(mode=="all"||mode=="rollback")rollback_tests();
    if(mode=="all"||mode=="dynamic")dynamic_connectivity();
}
