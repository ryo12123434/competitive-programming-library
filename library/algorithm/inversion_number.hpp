#pragma once

#include <algorithm>
#include <vector>
#include "../data_structure/fenwick_tree.hpp"

/**
 * @brief i < j かつ A[i] > A[j] を満たす組の数を返す。同じ値同士は数えない。
 * @pre A.size() が int に収まり、T の operator< と operator== が整合すること。
 * @note 最大 N*(N-1)/2 となるため、戻り値は long long。
 * @par 計算量 O(N log N) 時間、O(N) 空間。
 */
template <class T>
long long count_inversions(const std::vector<T>& A) {
    std::vector<T> B = A;
    std::sort(B.begin(), B.end());
    B.erase(std::unique(B.begin(), B.end()), B.end());
    FenwickTree<int> fw(static_cast<int>(B.size()));
    long long result = 0;
    for (int i = 0; i < static_cast<int>(A.size()); ++i) {
        int rank = static_cast<int>(std::lower_bound(B.begin(), B.end(), A[i]) - B.begin());
        result += i - fw.sum(rank + 1);
        fw.add(rank, 1);
    }
    return result;
}
