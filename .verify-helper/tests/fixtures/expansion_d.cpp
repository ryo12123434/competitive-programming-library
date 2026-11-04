#include "library/data_structure/swag.hpp"
#include "library/data_structure/disjoint_sparse_table.hpp"
#include <cassert>
#include <deque>
#include <numeric>
#include <random>
#include <string>
long long sum(long long a,long long b){return a+b;}long long zero(){return 0;}
std::string concat(std::string a,std::string b){return a+b;}std::string empty_word(){return {};}
struct NoDefault{long long x;NoDefault()=delete;explicit NoDefault(long long v):x(v){}};
NoDefault combine(NoDefault a,NoDefault b){return NoDefault(a.x+b.x);}
NoDefault identity(){return NoDefault(0);}
void swag(){std::mt19937 rng(6451);SWAG<std::string,concat,empty_word>q;std::deque<std::string>naive;assert(q.empty()&&q.fold()==""&&!q.pop());for(int i=0;i<30000;++i){if(rng()%3){std::string s(1,'a'+rng()%26);q.push(s);naive.push_back(s);}else{assert(q.pop()==!naive.empty());if(!naive.empty())naive.pop_front();}std::string expected;for(const auto&s:naive)expected+=s;assert(q.fold()==expected&&q.size()==naive.size()&&q.empty()==naive.empty());}while(q.pop()){}assert(q.fold()=="");SWAG<NoDefault,combine,identity> other;other.push(NoDefault(4));assert(other.fold().x==4);}
void disjoint(){std::mt19937 rng(124);for(int n=0;n<100;++n){std::vector<long long>a(n);std::vector<std::string>words(n);for(int i=0;i<n;++i){a[i]=static_cast<int>(rng()%101)-50;words[i]=std::string(1,'a'+rng()%26);}DisjointSparseTable<long long,sum>table(a);DisjointSparseTable<std::string,concat>text(words);assert(table.size()==n);for(int l=0;l<n;++l){long long expected=0;std::string word;for(int r=l+1;r<=n;++r){expected+=a[r-1];word+=words[r-1];assert(table.prod(l,r)==expected&&text.prod(l,r)==word);}}}DisjointSparseTable<NoDefault,combine>none(std::vector<NoDefault>{NoDefault(4),NoDefault(5)});assert(none.prod(0,2).x==9);}
void large(){const int n=200000;SWAG<long long,sum,zero>q;for(int i=0;i<n;++i)q.push(i);assert(q.fold()==static_cast<long long>(n)*(n-1)/2);for(int i=0;i<n;++i)assert(q.pop());assert(q.empty());std::vector<long long>a(n,1);DisjointSparseTable<long long,sum>table(a);for(int i=0;i<n;++i)assert(table.prod(i,n)==n-i);}
int main(int argc,char**argv){std::string mode=argc>1?argv[1]:"all";if(mode=="all"||mode=="swag")swag();if(mode=="all"||mode=="disjoint")disjoint();if(mode=="large")large();}
