#pragma once

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @brief 半開区間 [l, r) の集合を管理する。
 *
 * 区間は互いに重ならず、隣接する区間も自動的にマージされる。
 *
 * @tparam T 区間端点の型
 * @par Complexity
 * insert / erase は O((K + 1) log N)。
 * N は現在の区間数、K は操作で削除・結合される区間数。
 *
 * @pre T は狭義弱順序の比較と減算に対応する型（例: int、long long）。浮動小数点では端点・被覆長とも有限値とし、丸め誤差に注意。
 * @note 既定構築で空集合。端点も被覆長も途中の演算を含め各型に収まること。
 * @par Complexity
 * 保持空間 O(N)、空集合の構築 O(1)。
 *
 * @code
 * #include "library/data_structure/interval_set.hpp"
 * #include <cassert>
 * int main() {
 *     IntervalSet<long long> intervals;
 *     intervals.insert(1, 4);
 *     intervals.erase(2, 3);
 *     assert(intervals.covered_length == 2);
 *     assert(intervals.mex(1) == 2);
 * }
 * @endcode
 */
template <class T>
struct IntervalSet {
    std::set<std::pair<T, T>> st;

    /**
     * @brief 被覆長の型。整数端点では long long、それ以外では T。
     * @pre 長さの合計はこの型に収まること。整数端点は 64 bit 以下。
     * @note st を直接変更すると被覆長や区間の正規化を維持できない。
     */
    using length_type = std::conditional_t<std::is_integral_v<T>, long long, T>;
    /// @brief 被覆長の合計。O(1) で参照できる。
    length_type covered_length{};

    /**
     * @brief 区間 [l, r) を追加する。
     * @param l 区間の左端
     * @param r 区間の右端
     * @note l >= r のときは何もしない。
     * @par Complexity
     * O((K + 1) log N)
     */
    void insert(T l, T r) {
        if (!(l < r)) return;

        auto it = st.lower_bound({l, l});
        if (it != st.begin()) {
            auto pit = std::prev(it);
            if (!(pit->second < l)) it = pit;
        }

        while (it != st.end() && !(r < it->first)) {
            l = std::min(l, it->first);
            r = std::max(r, it->second);
            covered_length -= length(it->first, it->second);
            it = st.erase(it);
        }

        st.insert({l, r});
        covered_length += length(l, r);
    }

    /**
     * @brief 区間 [l, r) を削除する。
     * @param l 区間の左端
     * @param r 区間の右端
     * @note l >= r のときは何もしない。
     * @par Complexity
     * O((K + 1) log N)
     */
    void erase(T l, T r) {
        if (!(l < r)) return;

        auto it = st.lower_bound({l, l});
        if (it != st.begin()) {
            auto pit = std::prev(it);
            if (l < pit->second) it = pit;
        }

        std::vector<std::pair<T, T>> leftovers;

        while (it != st.end() && it->first < r) {
            const auto [a, b] = *it;

            if (b <= l) {
                ++it;
                continue;
            }

            if (a < l) leftovers.push_back({a, l});
            if (r < b) leftovers.push_back({r, b});

            covered_length -= length(a, b);
            it = st.erase(it);
        }

        for (const auto& p : leftovers) {
            st.insert(p);
            covered_length += length(p.first, p.second);
        }
    }

    /**
     * @brief [l, r) と被覆区間が交差するかを O(log N) で判定する。l >= r なら false。
     * @param l 左端（含む）。
     * @param r 右端（含まない）。
     * @return 被覆区間と共通部分があれば true。
     */
    bool intersects(T l, T r) const {
        if (!(l < r)) return false;
        auto it = st.lower_bound({l, l});
        if (it != st.end() && it->first < r) return true;
        return it != st.begin() && l < std::prev(it)->second;
    }

    /**
     * @brief x が被覆区間に含まれるかを O(log N) で判定する。
     * @param x 判定する値。
     * @return ある [l, r) に l <= x < r として含まれれば true。
     */
    bool contains(T x) const {
        auto it = st.lower_bound({x, x});
        if (it != st.end() && it->first == x) return true;
        return it != st.begin() && x < std::prev(it)->second;
    }

    /**
     * @brief x 以上の最小の未被覆値を O(log N) で返す。隣接区間は併合済み。
     * @param x 探索の下限（既定値 0）。
     * @return x が未被覆なら x、被覆されていればその区間の右端。
     */
    T mex(T x = 0) const {
        auto it = st.lower_bound({x, x});
        if (it != st.end() && it->first == x) return it->second;
        if (it != st.begin() && x < std::prev(it)->second) return std::prev(it)->second;
        return x;
    }

private:
    static length_type length(T l, T r) {
        if constexpr (std::is_integral_v<T>) {
            return static_cast<long long>(static_cast<__int128>(r) - static_cast<__int128>(l));
        } else {
            return r - l;
        }
    }
};
