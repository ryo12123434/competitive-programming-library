#pragma once
#include <numeric>
#include <stdexcept>
#include <vector>

/**
 * @brief Totally Monotone行列の各行の最小値を取る列IDを返すSMAWK。同値は入力順で最も左。
 * @param rows 行IDの順序付き集合。返り値のi番はrows[i]の最適な列ID。
 * @param columns 列IDの順序付き集合。行があるのに空ならinvalid_argument。
 * @param value value(row_id,column_id)で値を評価するコールバック。
 * @pre 同値時に左を優先する比較で、すべての部分行列の行最小位置が単調であること。
 * 行数・列数はintに収めること。
 * 行全体の最適位置が単調なだけなら既存Monotone Minimaを使う。Monge行列は十分条件。
 * @par Complexity O(H+W)回の評価・比較、O(H+W)追加空間。再帰深さO(log(H+1))。
 * @code
 * #include "library/algorithm/smawk.hpp"
 * #include <cassert>
 * int main(){auto a=smawk(std::vector<int>{0,1,2},std::vector<int>{0,1,2},[](int i,int j){return (i-j)*(i-j);});assert((a==std::vector<int>{0,1,2}));}
 * @endcode
 */
template <class Eval>
std::vector<int> smawk(const std::vector<int> &rows, const std::vector<int> &columns, Eval value) {
    if (rows.empty())
        return {};
    if (columns.empty())
        throw std::invalid_argument("SMAWK: no columns");
    std::vector<int> answer(rows.size()), indices(rows.size());
    std::iota(indices.begin(), indices.end(), 0);
    auto solve = [&](auto &&self, const std::vector<int> &row, const std::vector<int> &cols) -> void {
        if (row.empty())
            return;
        std::vector<int> reduced;
        for (int c : cols) {
            while (!reduced.empty() && value(rows[row[reduced.size() - 1]], c) <
                                           value(rows[row[reduced.size() - 1]], reduced.back()))
                reduced.pop_back();
            if (reduced.size() < row.size())
                reduced.push_back(c);
        }
        std::vector<int> odd;
        for (std::size_t i = 1; i < row.size(); i += 2)
            odd.push_back(row[i]);
        self(self, odd, reduced);
        std::size_t begin = 0;
        for (std::size_t i = 0; i < row.size(); i += 2) {
            if (i)
                while (reduced[begin] != answer[row[i - 1]])
                    ++begin;
            std::size_t end = begin;
            if (i + 1 < row.size())
                while (reduced[end] != answer[row[i + 1]])
                    ++end;
            else
                end = reduced.size() - 1;
            int best = reduced[begin];
            for (std::size_t j = begin + 1; j <= end; ++j)
                if (value(rows[row[i]], reduced[j]) < value(rows[row[i]], best))
                    best = reduced[j];
            answer[row[i]] = best;
        }
    };
    solve(solve, indices, columns);
    return answer;
}
