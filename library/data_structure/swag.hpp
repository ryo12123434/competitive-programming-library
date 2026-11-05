#pragma once

#include <cstddef>
#include <utility>
#include <vector>

/**
 * @brief 2本のスタックで Queue の全要素を順番に集約する SWAG。
 * @tparam S コピー可能な値の型。S op(S,S) は結合的、S e() は両側単位元。可換性は不要。
 * @tparam op 前側・後側の集約をこの順に結合する関数。
 * @tparam e 空 Queue の集約を返す関数。
 * @note 添字は持たない。push した順（先頭から末尾）で集約し、pop は先頭を削除する。
 * @par Complexity
 * S のコピー・演算 O(1) なら push/pop は償却 O(1)、fold/size/empty は O(1)。
 * 単発 pop の最悪 O(N)。空間 O(過去の最大要素数)。値の算術オーバーフローは利用者が避けること。
 * @code
 * #include "library/data_structure/swag.hpp"
 * #include <cassert>
 * long long sum(long long a,long long b){return a+b;} long long zero(){return 0;}
 * int main() { SWAG<long long,sum,zero> q; q.push(2);q.push(3);q.pop();assert(q.fold()==3); }
 * @endcode
 */
template <class S, S (*op)(S, S), S (*e)()>
class SWAG {
public:
    /// @brief 空 Queue を構築。O(1)。
    SWAG() = default;
    /// @brief 末尾へ value を追加。償却 O(1)。
    void push(const S& value) {
        S aggregate = back_.empty() ? value : op(back_.back().aggregate, value);
        back_.push_back({value, std::move(aggregate)});
    }
    /// @brief 先頭を1要素削除。空なら false（変更なし）、成功なら true。償却 O(1)。
    bool pop() {
        if (front_.empty()) {
            while (!back_.empty()) {
                const S& value = back_.back().value;
                S aggregate = front_.empty() ? value : op(value, front_.back().aggregate);
                front_.push_back({value, std::move(aggregate)}); back_.pop_back();
            }
        }
        if (front_.empty()) return false;
        front_.pop_back(); return true;
    }
    /// @brief 全要素の順序付き集約。空なら e()。O(1)。
    S fold() const { return op(front_.empty() ? e() : front_.back().aggregate, back_.empty() ? e() : back_.back().aggregate); }
    /// @brief 要素数。O(1)。
    std::size_t size() const { return front_.size() + back_.size(); }
    /// @brief 空かどうか。O(1)。
    bool empty() const { return front_.empty() && back_.empty(); }
private:
    struct Node { S value, aggregate; };
    std::vector<Node> front_, back_;
};
