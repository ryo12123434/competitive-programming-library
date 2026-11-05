#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

/// @brief Cartesian Tree の頂点情報。頂点IDは元配列添字、存在しない親・子・空配列の根は-1。
struct CartesianTreeResult {
    int root = -1;
    std::vector<int> parent, left, right;
};
/**
 * @brief 中間順が元配列順になる Min/Max Cartesian Tree を単調スタックで構築する。
 * @tparam T 比較可能な要素。@tparam Compare 小さい優先度を選ぶ狭義弱順序。
 * @param a 入力配列。@param compare 既定 less はMin、greater<T>{}ならMax。比較同値は小さい添字を上に置く。
 * @pre a.size()<=INT_MAX。入力は変更しない。
 * @par Complexity 比較 O(1) なら O(N) 時間・追加空間。再帰なし。
 * @code
 * #include "library/algorithm/cartesian_tree.hpp"
 * #include <cassert>
 * int main(){auto t=cartesian_tree(std::vector<int>{2,1,1});assert(t.root==1&&t.parent[2]==1);}
 * @endcode
 */
template <class T, class Compare = std::less<T>>
CartesianTreeResult cartesian_tree(const std::vector<T> &a, Compare compare = {}) {
    assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int n = static_cast<int>(a.size());
    CartesianTreeResult result;
    result.parent.assign(n, -1);
    result.left.assign(n, -1);
    result.right.assign(n, -1);
    std::vector<int> stack;
    for (int i = 0; i < n; ++i) {
        int last = -1;
        while (!stack.empty() && compare(a[i], a[stack.back()])) {
            last = stack.back();
            stack.pop_back();
        }
        if (!stack.empty()) {
            result.parent[i] = stack.back();
            result.right[stack.back()] = i;
        }
        if (last != -1) {
            result.parent[last] = i;
            result.left[i] = last;
        }
        stack.push_back(i);
    }
    if (!stack.empty())
        result.root = stack.front();
    return result;
}
