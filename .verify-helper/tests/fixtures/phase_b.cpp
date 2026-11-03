#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>
#include "library/string/manacher.hpp"
#include "library/math/xor_basis.hpp"
#include "library/data_structure/binary_trie.hpp"
#include "library/graph/rerooting.hpp"

namespace {
void check(bool condition) { if (!condition) std::abort(); }
std::mt19937_64 rng(20261026);
template<class Sequence> void check_palindromes(const Sequence& s) {
    auto result=manacher(s);
    const int n=static_cast<int>(s.size());
    std::pair<int,int> longest{0,0};
    for(int l=0;l<=n;++l) for(int r=l;r<=n;++r) {
        bool expected=true;
        for(int i=l,j=r-1;i<j;++i,--j) if(s[i]!=s[j]) expected=false;
        check(result.is_palindrome(l,r)==expected);
        if(expected && r-l>longest.second-longest.first) longest={l,r};
    }
    check(result.longest()==longest);
    for(int i=0;i<n;++i) {
        int odd=1,even=0;
        while(odd<=i && odd<n-i && s[i-odd]==s[i+odd]) ++odd;
        while(even<i && even<n-i && s[i-even-1]==s[i+even]) ++even;
        check(result.odd[i]==odd && result.even[i]==even);
    }
}
void manacher_tests() {
    for(int n=0;n<=10;++n) for(int mask=0;mask<(1<<n);++mask) {
        std::string s(n,'\0');for(int i=0;i<n;++i)s[i]=char((mask>>i&1)?255:0);
        check_palindromes(s);
    }
    for(int trial=0;trial<200;++trial) {
        std::vector<int> s(rng()%35);for(int& x:s)x=int(rng()%7)-3;
        check_palindromes(s);
    }
    auto result=manacher(std::string(1000000,'a'));
    check(result.longest()==std::pair{0,1000000} && result.is_palindrome(3,999997));
}
void basis_tests() {
    for(int trial=0;trial<500;++trial) {
        std::vector<std::uint64_t> values(rng()%11);
        for(auto& x:values)x=rng()%128;
        std::set<std::uint64_t> span{0};XorBasis<> basis;
        for(auto x:values) {
            const bool independent=!span.contains(x);
            check(basis.insert(x)==independent);
            auto copy=span;for(auto y:copy)span.insert(x^y);
            check(basis.distinct_count()==span.size());
        }
        for(std::uint64_t x=0;x<256;++x) {
            check(basis.can_represent(x)==span.contains(x));
            std::uint64_t lo=x,hi=x;
            for(auto y:span){lo=std::min(lo,x^y);hi=std::max(hi,x^y);}
            check(basis.min_xor(x)==lo && basis.max_xor(x)==hi);
        }
        std::shuffle(values.begin(),values.end(),rng);XorBasis<> other;
        for(auto x:values)other.insert(x);
        check(other.rank()==basis.rank() && other.max_xor()==basis.max_xor());
        XorBasis<> merged;merged.merge(other);merged.merge(merged);
        check(merged.rank()==basis.rank());merged.clear();check(merged.rank()==0 && merged.max_xor(9)==9);
    }
    XorBasis<> full;for(int i=0;i<64;++i)check(full.insert(std::uint64_t{1}<<i));
    check(full.rank()==64 && full.distinct_count()==(static_cast<unsigned __int128>(1)<<64));
    check(full.can_represent(UINT64_MAX) && full.max_xor()==UINT64_MAX && full.min_xor(UINT64_MAX)==0);
    XorBasis<std::uint32_t> small;small.insert(UINT32_MAX);check(small.max_xor()==UINT32_MAX);
}
template<class U,int Bits> void trie_test() {
    BinaryTrie<U,Bits> trie;std::multiset<U> expected;
    const U mask=[] {if constexpr(Bits==std::numeric_limits<U>::digits)return std::numeric_limits<U>::max();else return U((U{1}<<Bits)-1);}();
    for(int step=0;step<6000;++step) {
        U x=U(rng())&mask;
        if(step%13==0)x=0;
        if(step%17==0)x=mask;
        const int type=int(rng()%6);
        if(type<2){trie.insert(x);expected.insert(x);}
        else if(type==2){auto it=expected.find(x);check(trie.erase(x)==(it!=expected.end()));if(it!=expected.end())expected.erase(it);}
        else if(type==3){trie.xor_all(x);std::multiset<U> changed;for(auto y:expected)changed.insert(y^x);expected=std::move(changed);}
        check(trie.size()==expected.size() && trie.empty()==expected.empty() && trie.count(x)==expected.count(x));
        check(trie.count_less(x)==static_cast<std::size_t>(std::distance(expected.begin(),expected.lower_bound(x))));
        check(!trie.kth(expected.size()));
        if(expected.empty()){check(!trie.min_xor(x) && !trie.max_xor(x));continue;}
        U lo=mask,hi=0;for(auto y:expected){lo=std::min(lo,U(x^y));hi=std::max(hi,U(x^y));}
        check(trie.min_xor(x)==lo && trie.max_xor(x)==hi);
        check(expected.contains(*trie.argmin_xor(x)) && (*trie.argmin_xor(x)^x)==lo);
        check(expected.contains(*trie.argmax_xor(x)) && (*trie.argmax_xor(x)^x)==hi);
        const auto k=rng()%expected.size();check(trie.kth(k)==*std::next(expected.begin(),k));
    }
    for(auto x:expected)check(trie.erase(x));
    check(trie.empty());
    if constexpr(Bits<std::numeric_limits<U>::digits) {
        check(trie.count(U{1}<<Bits)==0 && !trie.erase(U{1}<<Bits));
    }
}
void trie_tests() {
    trie_test<std::uint64_t,64>();trie_test<std::uint32_t,32>();trie_test<std::uint32_t,5>();trie_test<std::uint32_t,1>();
    BinaryTrie<> trie;
    for(int i=0;i<100000;++i){auto x=rng();trie.insert(x);check(trie.erase(x));}
}
struct S {long long sum,count;};
void rerooting_tests() {
    check(rerooting<int>({},[](int a,int b){return a+b;},0,[](int a,int){return a+1;},[](int a,int,int,int){return a;}).empty());
    for(int trial=0;trial<300;++trial) {
        const int n=1+rng()%30;std::vector<std::vector<RerootingEdge>> tree(n);
        std::vector<long long> weights,vertices(n);
        for(auto& x:vertices)x=rng()%6;
        for(int v=1;v<n;++v) {
            int p=trial%3==0?v-1:trial%3==1?0:rng()%v;
            int id=static_cast<int>(weights.size());weights.push_back(rng()%10);weights.push_back(rng()%10);
            tree[p].push_back({v,id});tree[v].push_back({p,id+1});
        }
        for(auto& row:tree)std::shuffle(row.begin(),row.end(),rng);
        auto merge=[](S a,S b){return S{a.sum+b.sum,a.count+b.count};};
        auto finish=[&](S a,int v){return S{a.sum,a.count+vertices[v]};};
        auto transfer=[&](S a,int,int,int id){return S{a.sum+a.count*weights[id],a.count};};
        auto result=rerooting<S>(tree,merge,S{0,0},finish,transfer);
        std::function<S(int,int)> brute=[&](int v,int parent){S s{0,0};for(auto e:tree[v])if(e.to!=parent){
            auto child=brute(e.to,v);
            int reverse=-1;for(auto r:tree[e.to])if(r.to==v)reverse=r.id;
            s=merge(s,transfer(child,e.to,v,reverse));}return finish(s,v);};
        auto text_finish=[](std::string a,int v){return "("+std::to_string(v)+a+")";};
        auto text_transfer=[](std::string a,int from,int to,int id){return "["+std::to_string(from)+":"+std::to_string(to)+":"+std::to_string(id)+a+"]";};
        auto text=rerooting<std::string>(tree,[](auto a,auto b){return a+b;},"",text_finish,text_transfer);
        std::function<std::string(int,int)> text_brute=[&](int v,int parent){std::string s;for(auto e:tree[v])if(e.to!=parent){
            int reverse=-1;for(auto r:tree[e.to])if(r.to==v)reverse=r.id;
            s+=text_transfer(text_brute(e.to,v),e.to,v,reverse);}return text_finish(s,v);};
        for(int v=0;v<n;++v){auto b=brute(v,-1);check(result[v].sum==b.sum && result[v].count==b.count);check(text[v]==text_brute(v,-1));}
    }
    const int n=200000;
    for(bool star:{false,true}) {
        std::vector<std::vector<RerootingEdge>> tree(n);
        for(int v=1;v<n;++v){int p=star?0:v-1;tree[p].push_back({v,v});tree[v].push_back({p,v});}
        auto result=rerooting<S>(tree,[](S a,S b){return S{a.sum+b.sum,a.count+b.count};},S{0,0},
            [](S a,int){return S{a.sum,a.count+1};},[](S a,int,int,int){return S{a.sum+a.count,a.count};});
        check(result[0].sum==(star?n-1:1LL*n*(n-1)/2));
        check(result[n-1].sum==(star?2LL*n-3:1LL*n*(n-1)/2));
    }
}
}
int main(int argc,char** argv) {
    const std::string mode=argc>1?argv[1]:"all";
    if(mode=="all"||mode=="manacher")manacher_tests();
    if(mode=="all"||mode=="basis")basis_tests();
    if(mode=="all"||mode=="trie")trie_tests();
    if(mode=="all"||mode=="rerooting")rerooting_tests();
}
