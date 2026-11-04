#include "library/string/kmp.hpp"
#include "library/string/booth.hpp"
#include "library/string/aho_corasick.hpp"
#include "library/string/suffix_automaton.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <string>

std::vector<int> find_naive(const std::string&t,const std::string&p){std::vector<int>a;for(std::size_t i=0;i+p.size()<=t.size();++i)if(t.compare(i,p.size(),p)==0)a.push_back(i);return a;}
std::vector<std::string> strings(){std::vector<std::string>a{""};for(int n=1;n<=8;++n)for(int m=0;m<(1<<n);++m){std::string s(n,'a');for(int i=0;i<n;++i)s[i]+=m>>i&1;a.push_back(s);}return a;}
void kmp_booth(){
    auto words=strings();
    for(const auto&s:words){
        auto pi=prefix_function(s);for(int i=0;i<static_cast<int>(s.size());++i){int best=0;for(int k=1;k<=i;++k)if(s.substr(0,k)==s.substr(i-k+1,k))best=k;assert(pi[i]==best);}
        int best=0;std::string rotated=s;
        for(int i=0;i<static_cast<int>(s.size());++i){auto t=s.substr(i)+s.substr(0,i);if(t<rotated){rotated=t;best=i;}}
        assert(booth(s)==best&&minimal_rotation(s)==rotated);
        for(const auto&p:words)if(p.size()<=5)assert(kmp_search(s,p)==find_naive(s,p));
    }
    for(int n=1;n<=6;++n){
        int total=1;for(int i=0;i<n;++i)total*=3;
        for(int mask=0;mask<total;++mask){
            std::string s(n,' ');int value=mask;for(char&c:s){c=static_cast<char>((value%3==0)?0:(value%3==1)?128:255);value/=3;}
            int best=0;std::string expected=s;for(int i=1;i<n;++i){auto t=s.substr(i)+s.substr(0,i);if(t<expected){expected=t;best=i;}}
            assert(booth(s)==best&&minimal_rotation(s)==expected);
        }
    }
    std::vector<int> t{-3,1,-3,1,-3},p{-3,1};assert((kmp_search(t,p)==std::vector<int>{0,2}));assert(booth(t)==4);
    std::string bytes("\0\xff\0",3);assert((kmp_search(bytes,std::string("\0",1))==std::vector<int>{0,2}));
}
void aho(){
    std::mt19937 rng(1245);
    for(int trial=0;trial<1000;++trial){
        AhoCorasick ac("ab");std::vector<std::string>ps;
        for(int i=0;i<trial%14;++i){std::string p;int n=rng()%8;for(int j=0;j<n;++j)p+='a'+rng()%2;assert(ac.add_pattern(p)==i);ps.push_back(p);}
        ac.build();ac.build();
        std::string text;for(int i=0;i<50;++i)text+='a'+rng()%3;
        auto counts=ac.count_occurrences(text);std::vector<std::pair<int,int>>expected,actual,streamed;
        for(int id=0;id<static_cast<int>(ps.size());++id){auto positions=find_naive(text,ps[id]);assert(counts[id]==static_cast<long long>(positions.size()));for(int p:positions)expected.emplace_back(id,p);}
        for(auto m:ac.find_all(text))actual.emplace_back(m.pattern_id,m.position);
        ac.for_each_match(text,[&](int id,int pos){streamed.emplace_back(id,pos);});assert(actual==streamed);
        std::sort(expected.begin(),expected.end());std::sort(actual.begin(),actual.end());assert(actual==expected);
    }
    AhoCorasick empty("");empty.add_pattern("");empty.build();assert(empty.count_occurrences("abc")[0]==4);
    std::string alphabet;for(int c=0;c<256;++c)alphabet+=static_cast<char>(c);
    AhoCorasick bytes(alphabet);bytes.add_pattern(std::string("\xff\0",2));bytes.build();assert(bytes.count_occurrences(std::string("\xff\0\xff\0",4))[0]==2);
}
void sam(){
    for(const auto&s:strings()){
        SuffixAutomaton sam;
        std::string prefix;
        for(std::size_t step=0;;++step){
            std::set<std::string>sub;
            for(std::size_t i=0;i<prefix.size();++i)for(std::size_t j=i+1;j<=prefix.size();++j)sub.insert(prefix.substr(i,j-i));
            assert(sam.distinct_substrings()==static_cast<long long>(sub.size()));assert(sam.contains("")&&sam.occurrences("")==static_cast<long long>(prefix.size()+1));
            for(const auto&p:sub)assert(sam.contains(p)&&sam.occurrences(p)==static_cast<long long>(find_naive(prefix,p).size()));
            assert(!sam.contains("c")&&sam.occurrences("c")==0);
            if(step==s.size())break;
            sam.append(s[step]);prefix+=s[step];
        }
        SuffixAutomaton built(s);assert(built.distinct_substrings()==sam.distinct_substrings());
    }
    std::mt19937 rng(956);
    for(int trial=0;trial<250;++trial){std::string s;for(int i=0;i<40;++i)s+=static_cast<char>(rng()%256);SuffixAutomaton automaton(s);std::set<std::string>sub;
        for(int i=0;i<40;++i)for(int j=i+1;j<=40;++j){auto p=s.substr(i,j-i);sub.insert(p);assert(automaton.occurrences(p)==static_cast<long long>(find_naive(s,p).size()));}
        assert(automaton.distinct_substrings()==static_cast<long long>(sub.size()));}
}
void large(){std::string s(200000,'a');assert(kmp_search(s,std::string(100000,'a')).size()==100001);assert(booth(s)==0);SuffixAutomaton sam(s);assert(sam.distinct_substrings()==200000&&sam.occurrences("aa")==199999);AhoCorasick ac;for(int len=1;len<=1000;++len)ac.add_pattern(std::string(len,'a'));ac.build();auto counts=ac.count_occurrences(s);assert(counts.back()==199001);}
int main(int argc,char**argv){std::string mode=argc>1?argv[1]:"all";if(mode=="all"||mode=="kmp_booth")kmp_booth();if(mode=="all"||mode=="aho")aho();if(mode=="all"||mode=="sam")sam();if(mode=="large")large();}
