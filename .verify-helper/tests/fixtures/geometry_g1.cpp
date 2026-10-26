#include <iostream>
#include <string>
#include <algorithm>
#include "library/geometry/polygon_query.hpp"
#include "library/geometry/point_pair.hpp"
#include "library/geometry/lattice_polygon.hpp"
#include "library/geometry/polar_angle_sort.hpp"

using namespace cpl::geometry;
void print(Wide value) {
    if(value==0) {std::cout<<0;return;}
    unsigned __int128 x=static_cast<unsigned __int128>(value);
    if(value<0) {std::cout<<'-';x=-x;}
    std::string s;
    while(x) {s.push_back('0'+x%10);x/=10;}
    std::reverse(s.begin(),s.end());std::cout<<s;
}
int main() {
    std::string mode;
    while(std::cin>>mode) {
        int n; std::cin>>n;
        std::vector<Point<>> p(n);
        for(auto& v:p) std::cin>>v.x>>v.y;
        if(mode=="pair") {
            for(auto pair:{closest_pair(p),farthest_pair(p)}) {
                if(!pair) std::cout<<"none ";
                else {std::cout<<pair->first<<' '<<pair->second<<' ';print(pair->distance2);std::cout<<' ';}
            }
        } else if(mode=="query") {
            Point<> q;std::cin>>q.x>>q.y;
            ConvexPolygon<> convex(p);
            std::cout<<int(contains_polygon(p,q))<<' '<<int(convex.contains(q))<<' '
                     <<is_convex(p)<<' '<<is_convex(p,true);
            if(p.size()==3) std::cout<<' '<<int(contains_triangle(p[0],p[1],p[2],q));
        } else {
            print(boundary_lattice_points(p));std::cout<<' ';
            const auto interior=interior_lattice_points(p);
            if(interior) print(*interior);else std::cout<<"none";
        }
        std::cout<<'\n';
    }
}
