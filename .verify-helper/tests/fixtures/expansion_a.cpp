#include "library/data_structure/segment_tree_beats.hpp"
#include "library/data_structure/persistent_segment_tree.hpp"
#include <cassert>
#include <climits>
#include <numeric>
#include <random>
#include <string>

long long plus_value(long long a,long long b){return a+b;}
long long zero_value(){return 0;}
std::string concat(std::string a,std::string b){return a+b;}
std::string empty_string(){return {};}
using Wide=SegmentTreeBeats::Sum;
void beats(){
    std::mt19937_64 rng(74539);
    for(int trial=0;trial<140;++trial){
        int n=trial%41;
        std::vector<long long>a(n);
        for(auto&x:a)x=static_cast<int>(rng()%81)-40;
        SegmentTreeBeats seg(a);
        for(int step=0;step<1200;++step){
            int l=rng()%(n+1),r=rng()%(n+1);if(l>r)std::swap(l,r);
            long long x=static_cast<int>(rng()%121)-60;
            switch(rng()%4){
            case 0:seg.range_chmin(l,r,x);for(int i=l;i<r;++i)a[i]=std::min(a[i],x);break;
            case 1:seg.range_chmax(l,r,x);for(int i=l;i<r;++i)a[i]=std::max(a[i],x);break;
            case 2:seg.range_add(l,r,x);for(int i=l;i<r;++i)a[i]+=x;break;
            default:break;
            }
            Wide sum=0;std::optional<long long>lo,hi;
            for(int i=l;i<r;++i){sum+=a[i];lo=lo?std::min(*lo,a[i]):a[i];hi=hi?std::max(*hi,a[i]):a[i];}
            assert(seg.range_sum(l,r)==sum&&seg.range_min(l,r)==lo&&seg.range_max(l,r)==hi);
            if(n&&step%23==0)for(int i=0;i<n;++i)assert(seg.point_get(i)==a[i]);
        }
    }
    const std::vector<long long> limits{LLONG_MIN,LLONG_MIN+1,-100,-1,0,1,100,LLONG_MAX-1,LLONG_MAX};
    for(int trial=0;trial<100;++trial){
        int n=1+rng()%12;std::vector<long long>a(n);for(auto&x:a)x=limits[rng()%limits.size()];
        SegmentTreeBeats seg(a);
        for(int step=0;step<500;++step){
            int l=rng()%n,r=1+rng()%n;if(l>r)std::swap(l,r);
            long long x=limits[rng()%limits.size()];int kind=rng()%3;
            if(kind==0){seg.range_chmin(l,r,x);for(int i=l;i<r;++i)a[i]=std::min(a[i],x);}
            if(kind==1){seg.range_chmax(l,r,x);for(int i=l;i<r;++i)a[i]=std::max(a[i],x);}
            if(kind==2){bool safe=true;for(int i=l;i<r;++i)if(Wide(a[i])+x<LLONG_MIN||Wide(a[i])+x>LLONG_MAX)safe=false;
                if(safe){seg.range_add(l,r,x);for(int i=l;i<r;++i)a[i]=static_cast<long long>(Wide(a[i])+x);}}
            assert(seg.range_sum(0,n)==std::accumulate(a.begin(),a.end(),Wide(0)));
            int at=rng()%n;assert(seg.point_get(at)==a[at]);
            if(step%31==0)for(int i=0;i<n;++i)assert(seg.point_get(i)==a[i]);
        }
    }
    const std::vector<std::vector<long long>> extreme{{LLONG_MIN,LLONG_MAX},{LLONG_MIN,LLONG_MIN},{LLONG_MAX,LLONG_MAX},{LLONG_MIN,0,LLONG_MAX,1}};
    for(auto a:extreme){
        SegmentTreeBeats seg(a);int n=a.size();
        assert(seg.range_sum(0,n)==std::accumulate(a.begin(),a.end(),Wide(0)));
        seg.range_chmin(0,n,LLONG_MIN);assert(seg.range_sum(0,n)==Wide(LLONG_MIN)*n);
        seg.range_add(0,n,LLONG_MAX);assert(seg.range_min(0,n)==-1);
        seg.range_chmax(0,n,LLONG_MAX);seg.range_add(0,n,LLONG_MIN);
        for(int i=0;i<n;++i)assert(seg.point_get(i)==-1);
    }
    // 遅延の加算累積が64bitを越える場合と、子の一時値だけが範囲外になる場合。
    SegmentTreeBeats tag(std::vector<long long>{LLONG_MIN,LLONG_MIN+1,LLONG_MIN});
    tag.range_add(0,3,LLONG_MAX);tag.range_add(0,3,LLONG_MAX);
    tag.range_chmin(0,3,LLONG_MAX-1);
    assert(tag.point_get(0)==LLONG_MAX-1&&tag.point_get(2)==LLONG_MAX-1);
    SegmentTreeBeats temporary(std::vector<long long>{LLONG_MAX,0,1});
    temporary.range_chmin(0,3,5);temporary.range_add(0,3,LLONG_MAX-5);
    assert(temporary.point_get(0)==LLONG_MAX&&temporary.point_get(1)==LLONG_MAX-5);
    for(int n=1;n<=7;++n){
        for(int mask=0;mask<(1<<n);++mask){
            std::vector<long long>a(n);for(int i=0;i<n;++i)a[i]=(mask>>i&1)?LLONG_MAX:LLONG_MIN;
            SegmentTreeBeats seg(a);seg.range_chmin(0,n,0);seg.range_add(0,n,LLONG_MAX);seg.range_chmax(0,n,1);
            for(int i=0;i<n;++i)assert(seg.point_get(i)==((mask>>i&1)?LLONG_MAX:1));
        }
    }
}
void persistent(){
    std::mt19937 rng(452);
    for(int n=0;n<45;++n){
        std::vector<long long>a(n);std::iota(a.begin(),a.end(),-20);
        PersistentSegmentTree<long long,plus_value,zero_value>seg(a);
        std::vector<std::vector<long long>>versions{a};
        for(int step=0;step<700;++step){
            int v=rng()%versions.size();
            if(n){int i=rng()%n;long long x=static_cast<int>(rng()%91)-45;auto b=versions[v];b[i]=x;
                assert(seg.set(v,i,x)==static_cast<int>(versions.size()));versions.push_back(b);}
            for(int q=0;q<4;++q){v=rng()%versions.size();int l=rng()%(n+1),r=rng()%(n+1);if(l>r)std::swap(l,r);
                assert(seg.prod(v,l,r)==std::accumulate(versions[v].begin()+l,versions[v].begin()+r,0LL));
                assert(seg.all_prod(v)==std::accumulate(versions[v].begin(),versions[v].end(),0LL));
                if(n){int i=rng()%n;assert(seg.get(v,i)==versions[v][i]);}}
        }
    }
    PersistentSegmentTree<std::string,concat,empty_string>words(std::vector<std::string>{"a","b","c"});
    int v=words.set(0,1,"XYZ");assert(words.prod(v,0,3)=="aXYZc"&&words.all_prod(0)=="abc");
    std::vector<std::vector<std::string>> texts{{"a","b","c"}};
    for(int i=0;i<300;++i){int base=rng()%texts.size(),index=rng()%3;auto next=texts[base];next[index]=std::string(1,'a'+rng()%26);
        int id=words.set(base==0?0:base+1,index,next[index]);texts.push_back(next);
        assert(id==static_cast<int>(texts.size()));
        int check=rng()%texts.size();auto expected=texts[check];int ver=check==0?0:check+1;
        assert(words.all_prod(ver)==expected[0]+expected[1]+expected[2]);}
    PersistentSegmentTree<long long,plus_value,zero_value>zeros(8);assert(zeros.all_prod(0)==0);
}
void large(){
    const int n=200000;
    std::vector<long long>a(n);std::iota(a.begin(),a.end(),0);
    SegmentTreeBeats seg(a);
    for(int t=0;t<20000;++t){int l=t%n;seg.range_add(l,n,1);seg.range_chmin(0,n,n);seg.range_chmax(0,n,0);}
    assert(seg.range_min(0,n)==1&&seg.range_max(0,n)==n);
    PersistentSegmentTree<long long,plus_value,zero_value>p(a);int v=0;
    for(int i=0;i<50000;++i)v=p.set(v,i,-i);
    assert(p.all_prod(0)==static_cast<long long>(n)*(n-1)/2);
}
int main(int argc,char**argv){std::string mode=argc>1?argv[1]:"all";if(mode=="all"||mode=="beats")beats();if(mode=="all"||mode=="persistent")persistent();if(mode=="large")large();}
