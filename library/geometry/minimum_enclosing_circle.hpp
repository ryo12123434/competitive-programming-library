#pragma once
#include "real_circle.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cpl::geometry {
/**
 * @brief 最小包含円の数値近似と、円を決めた元配列の点ID（1～3個）。
 * @note 最終半径は全点の計算距離まで外向きに補正する。supportは補正前の境界を決めた点。
 * @code
 * #include "library/geometry/minimum_enclosing_circle.hpp"
 * #include <cassert>
 * int main(){auto r=cpl::geometry::minimum_enclosing_circle({{0,0},{2,0}});assert(r&&r->circle.radius>=1&&r->support.size()==2);}
 * @endcode
 */
struct MinimumEnclosingCircleResult {
    RealCircle circle;
    std::vector<int> support;
};
/**
 * @brief 乱択増分法で最小包含円を求める。空入力はnullopt。同一点・共線も許す。
 * @param points RealPointの列。元の順序は変えない。
 * @param seed シャッフルのseed。同一環境では再現可能。期待計算量は一様な順序のモデルによる。
 * @param tolerance 半径・距離の尺度に対する相対許容幅。絶対EPSではない。
 * @pre 入力と全中間計算が有限で表現可能。点数はintに収まること。
 * @throws std::runtime_error 境界制約が数値的に矛盾する場合。正確性を保証できない円を黙って返さない。
 * @throws std::overflow_error 円を有限値として表現できない場合。
 * @note 実数演算の絶対精度保証はない。入力丸めや条件数が大きい問題は厳密演算を要する。
 * 2境界点に対する中心の可能範囲を1次元の上下限制約として解く。円周の厳密判定にはsupportを利用できる。
 * @par Complexity 一様乱択モデルで期待O(N)、最悪O(N^3)時間。入力コピーO(N)空間。
 */
inline std::optional<MinimumEnclosingCircleResult>
minimum_enclosing_circle(const std::vector<RealPoint> &points, std::uint64_t seed = 0x2718281828459045ULL,
                         RelativeTolerance tolerance = {}) {
    if (points.empty())
        return std::nullopt;
    assert(std::isfinite(tolerance.relative) && tolerance.relative >= 0);
    std::vector<std::pair<RealPoint, int>> order;
    order.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
        assert(std::isfinite(points[i].x) && std::isfinite(points[i].y));
        order.emplace_back(points[i], static_cast<int>(i));
    }
    std::mt19937_64 rng(seed);
    std::shuffle(order.begin(), order.end(), rng);
    auto contains = [&](RealCircle c, RealPoint p) {
        auto d = distance(c.center, p);
        return d - c.radius <= tolerance.relative * std::max(d, c.radius);
    };
    auto pair_circle = [&](std::size_t prefix, std::pair<RealPoint, int> a, std::pair<RealPoint, int> b) {
        const auto delta = b.first - a.first;
        const auto length_ab = length(delta);
        assert(length_ab > 0);
        const auto direction = delta / length_ab, normal = perpendicular(direction),
                   middle = a.first + delta / 2;
        const auto base = length_ab / 2;
        long double lower = -std::numeric_limits<long double>::infinity(), upper = -lower;
        int lower_point = -1, upper_point = -1;
        for (std::size_t k = 0; k < prefix; ++k) {
            auto w = order[k].first - middle;
            auto d = length(w), scale = std::max(d, base);
            auto unit = w / scale;
            auto side = cross(direction, unit);
            auto rhs = (d / scale - base / scale) * (d / scale + base / scale);
            if (side == 0) {
                if (rhs > tolerance.relative * 2)
                    throw std::runtime_error("MEC: infeasible collinear boundary");
                continue;
            }
            auto bound = (rhs / (2 * side)) * scale;
            if (side > 0 && bound > lower) {
                lower = bound;
                lower_point = order[k].second;
            }
            if (side < 0 && bound < upper) {
                upper = bound;
                upper_point = order[k].second;
            }
        }
        if (lower > upper)
            throw std::runtime_error("MEC: inconsistent boundary constraints");
        auto shift = std::clamp(0.0L, lower, upper);
        RealCircle circle{middle + normal * shift, std::hypot(base, shift)};
        if (!std::isfinite(circle.center.x) || !std::isfinite(circle.center.y) ||
            !std::isfinite(circle.radius))
            throw std::overflow_error("MEC: nonfinite circle");
        std::vector<int> support{a.second, b.second};
        if (shift != 0)
            support.push_back(shift > 0 ? lower_point : upper_point);
        return MinimumEnclosingCircleResult{circle, std::move(support)};
    };
    MinimumEnclosingCircleResult result{{order[0].first, 0}, {order[0].second}};
    for (std::size_t i = 1; i < order.size(); ++i)
        if (!contains(result.circle, order[i].first)) {
            result = {{order[i].first, 0}, {order[i].second}};
            for (std::size_t j = 0; j < i; ++j)
                if (!contains(result.circle, order[j].first))
                    result = pair_circle(j, order[i], order[j]);
        }
    for (auto p : points)
        result.circle.radius = std::max(result.circle.radius, distance(p, result.circle.center));
    if (result.circle.radius > 0)
        result.circle.radius =
            std::nextafter(result.circle.radius, std::numeric_limits<long double>::infinity());
    if (!std::isfinite(result.circle.radius))
        throw std::overflow_error("MEC: enclosing radius is not finite");
    return result;
}
} // namespace cpl::geometry
