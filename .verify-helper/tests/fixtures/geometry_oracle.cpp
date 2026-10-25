#include <iostream>
#include <string>
#include <vector>
#include "library/geometry/convex_hull.hpp"
#include "library/geometry/polar_angle_sort.hpp"
#include "library/geometry/polygon.hpp"
#include "library/geometry/segment.hpp"

using namespace cpl::geometry;
using P = Point<long long>;

P read_point() {
    P p;
    std::cin >> p.x >> p.y;
    return p;
}

std::vector<P> read_points() {
    int n;
    std::cin >> n;
    std::vector<P> points(n);
    for (auto& p : points) p = read_point();
    return points;
}

void print_wide(Wide value) {
    if (value < 0) std::cout << '-';
    unsigned __int128 x = value < 0 ? 0 - static_cast<unsigned __int128>(value) : static_cast<unsigned __int128>(value);
    char digits[40];
    int count = 0;
    do { digits[count++] = static_cast<char>('0' + x % 10); x /= 10; } while (x);
    while (count) std::cout << digits[--count];
}

void print_points(const std::vector<P>& points) {
    std::cout << points.size();
    for (const auto& p : points) std::cout << ' ' << p.x << ' ' << p.y;
    std::cout << '\n';
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string command;
    while (std::cin >> command) {
        if (command == "point") {
            auto a = read_point(), b = read_point(), c = read_point();
            for (Wide value : {dot(a,b), cross(a,b), cross(a,b,c), norm2(a), dist2(a,b), Wide(ccw(a,b,c))}) {
                print_wide(value); std::cout << ' ';
            }
            std::cout << '\n';
        } else if (command == "segment") {
            auto a = read_point(), b = read_point(), c = read_point(), d = read_point();
            std::cout << on_segment(c, Segment<long long>{a,b}) << ' '
                      << segments_intersect(Segment<long long>{a,b}, Segment<long long>{c,d}) << '\n';
        } else if (command == "polar") {
            auto points = read_points(); polar_angle_sort(points); print_points(points);
        } else if (command == "hull") {
            bool keep;
            std::cin >> keep;
            print_points(convex_hull(read_points(), keep));
        } else if (command == "polygon") {
            auto points = read_points();
            print_wide(signed_area2(points)); std::cout << ' ' << polygon_orientation(points) << '\n';
        } else return 2;
    }
}
