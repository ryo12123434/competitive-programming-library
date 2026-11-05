#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

/**
 * @brief 最初のt回の更新後に成立する、各クエリの最小tを並列二分探索する。
 * @param updates 更新回数M。@param queries クエリ数Q。
 * @param reset void()。状態を更新0回の状態に初期化する（各ラウンドで呼ぶ）。
 * @param apply void(int update_id)。0-indexedの更新を順に適用する。
 * @param check bool(int query_id)。現在の状態で成立するか判定する（状態を変更しない）。
 * @return 各クエリの最小 t in [0,M]。一度も成立しなければ M+1。0回ですでに成立する場合も扱う。
 * @pre 0<=M<INT_MAX、Q>=0。各判定は更新回数に関してfalseからtrueへ単調。
 * reset後に同じ更新列を適用すると同じ状態になること。一般の非単調な判定には使えない。
 * @note Q=0ならcallbackは一度も呼ばない。終了時は最後に走査した更新回数の状態が残る。
 * @par Complexity
 * callback O(1) なら O((M+Q) log(M+2)) 時間、O(M+Q) 追加空間。
 * resetのコストはラウンドごとに別途加算。O(log(M+2)) 回初期化する。
 * @code
 * #include "library/algorithm/parallel_binary_search.hpp"
 * #include <cassert>
#include <cstddef>
 * int main(){int total=0;std::vector<int>need{0,3,6};auto r=parallel_binary_search(5,3,[&]{total=0;},[&](int){++total;},[&](int q){return total>=need[q];});assert((r==std::vector<int>{0,3,6}));}
 * @endcode
 */
template <class Reset, class Apply, class Check>
std::vector<int> parallel_binary_search(int updates, int queries, Reset reset, Apply apply, Check check) {
    assert(0 <= updates && updates < std::numeric_limits<int>::max() && queries >= 0);
    std::vector<int> low(queries, -1), high(queries, updates + 1);
    if (!queries)
        return high;
    while (true) {
        std::vector<std::vector<int>> buckets(static_cast<std::size_t>(updates) + 1);
        int last = -1;
        for (int q = 0; q < queries; ++q)
            if (static_cast<long long>(low[q]) + 1 < high[q]) {
                int mid = static_cast<int>((static_cast<long long>(low[q]) + high[q]) / 2);
                buckets[mid].push_back(q);
                last = std::max(last, mid);
            }
        if (last < 0)
            break;
        reset();
        for (int t = 0; t <= last; ++t) {
            if (t)
                apply(t - 1);
            for (int q : buckets[t]) {
                if (check(q))
                    high[q] = t;
                else
                    low[q] = t;
            }
        }
    }
    return high;
}
