#pragma once

#include <algorithm>
#include <set>
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
 */
template <class T>
struct IntervalSet {
    std::set<std::pair<T, T>> st;

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
            it = st.erase(it);
        }

        st.insert({l, r});
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

            it = st.erase(it);
        }

        for (const auto& p : leftovers) {
            st.insert(p);
        }
    }
};
