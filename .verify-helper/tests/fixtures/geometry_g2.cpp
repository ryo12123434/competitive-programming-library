#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include "library/geometry/real_circle.hpp"
#include "library/geometry/polygon_query.hpp"
using namespace cpl::geometry;
RealPoint point() {RealPoint p;std::cin>>p.x>>p.y;return p;}
RealSegment segment() {auto a=point(),b=point();return {a,b};}
RealLine line() {auto s=segment();return RealLine::through(s.a,s.b);}
RealCircle circle() {auto c=point();long double r;std::cin>>r;return {c,r};}
void output(const Intersections& hit) {
    std::cout<<int(hit.kind)<<' '<<hit.points.size();
    for(auto p:hit.points)std::cout<<' '<<p.x<<' '<<p.y;
}
void output(const Tangents& hit) {
    std::cout<<int(hit.kind)<<' '<<hit.lines.size();
    for(auto l:hit.lines)std::cout<<' '<<l.p.x<<' '<<l.p.y<<' '<<l.d.x<<' '<<l.d.y;
}
void regression() {
    const auto eps=std::numeric_limits<long double>::epsilon();
    const RealLine a{{0,0},{1,1}},b{{0,1},{1,1+eps}};
    assert(intersections(a,b).kind==IntersectionKind::Uncertain);
    auto hit=intersections(a,b,RelativeTolerance{0});
    assert(hit.kind==IntersectionKind::Finite && hit.points.size()==1);
    const auto x=intersections(RealLine{{0,0},{1,1}},RealLine{{0,1},{1,1+1e-10L}});
    assert(x.kind==IntersectionKind::Finite && x.points.size()==1 && x.points[0].x<0);
    assert(intersections(RealCircle{{0,0},1},RealCircle{{2+eps*2,0},1}).kind==IntersectionKind::Uncertain);
    assert(intersections(RealCircle{{0,0},1},RealCircle{{2+1e-10L,0},1}).points.empty());
    assert(intersections(RealCircle{{0,0},1},RealCircle{{2-1e-10L,0},1}).points.size()==2);
    assert(intersections(RealCircle{{0,0},0},RealCircle{{0,0},0}).points.size()==1);
    assert(common_tangents(RealCircle{{0,0},0},RealCircle{{0,0},0}).kind==IntersectionKind::Infinite);
    assert(common_tangents(RealCircle{{0,0},0},RealCircle{{2,0},0}).lines.size()==1);
    assert(tangents_from(RealCircle{{0,0},0},{0,0}).kind==IntersectionKind::Infinite);
    assert(!projection(RealLine{},{}));
    assert(!reflection(RealLine{},{}));
    assert(!distance(RealPoint{},RealLine{}));
    assert(intersections(RealLine{},RealLine{{0,0},{1,0}}).kind==IntersectionKind::Invalid);
    assert(intersections(RealLine{},RealSegment{}).kind==IntersectionKind::Invalid);
    assert(intersections(RealLine{},RealCircle{{},1}).kind==IntersectionKind::Invalid);
    assert(intersections(RealCircle{{},-1},RealCircle{{},1}).kind==IntersectionKind::Invalid);
    assert(tangents_from(RealCircle{{},-1},{}).kind==IntersectionKind::Invalid);
    assert(common_tangents(RealCircle{{},-1},RealCircle{{},1}).kind==IntersectionKind::Invalid);
    for(long double scale:{1e-30L,1.0L,1e30L}) {
        auto h=intersections(RealCircle{{0,0},scale},RealCircle{{scale,0},scale});
        assert(h.kind==IntersectionKind::Finite && h.points.size()==2);
        for(auto p:h.points) assert(std::abs(length(p)/scale-1)<1e-15L);
    }
    assert(angle(RealPoint{1,0},RealPoint{0,1})>1.5L);
    assert((RealPoint{1,2}*2/2==RealPoint{1,2}));
    assert((RealPoint{1,2}<RealPoint{2,0}));
    assert((norm2(RealPoint{3,4})==25 && dist2(RealPoint{3,4},RealPoint{})==25));
}
int main() {
    regression();
    std::cout<<std::setprecision(36);
    std::string mode;
    while(std::cin>>mode) {
        if(mode=="ll") {auto a=line(),b=line();output(intersections(a,b));}
        else if(mode=="ls") {auto a=line();auto b=segment();output(intersections(a,b));}
        else if(mode=="ss") {auto a=segment(),b=segment();output(intersections(a,b));}
        else if(mode=="lc") {auto a=line();auto b=circle();output(intersections(a,b));}
        else if(mode=="cc") {auto a=circle(),b=circle();output(intersections(a,b));}
        else if(mode=="pt") {auto a=circle();auto p=point();output(tangents_from(a,p));}
        else if(mode=="ct") {auto a=circle(),b=circle();output(common_tangents(a,b));}
        else if(mode=="metrics") {
            auto p=point();auto s=segment();auto l=RealLine::through(s.a,s.b);
            const auto a=projection(l,p),b=reflection(l,p);const auto d=distance(p,l);
            if(a && b && d) std::cout<<a->x<<' '<<a->y<<' '<<b->x<<' '<<b->y<<' '<<*d;
            else std::cout<<"invalid";
            std::cout<<' '<<distance(p,s);
        } else if(mode=="distance") {
            auto a=segment(),b=segment();auto d=distance(a,b);
            if(d) std::cout<<*d;else std::cout<<"uncertain";
        }
        std::cout<<'\n';
    }
}
