#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

/**
 * @brief 点更新を含む区間クエリを、時間軸付きの3次元Moで処理する。
 * @tparam T コピー・代入可能な配列要素。更新前後の値を内部で記録する。
 * @note 0-indexed、半開区間。add_update と add_query の登録順が時系列。
 * 区間内更新は旧値をeraseして新値をadd、取消時は逆に処理する。集計は要素の追加順に依存しないこと。
 * 既存 Mo の左右共通runに対応する用途向けで、順序依存の区間集約には使わない。
 * @par Complexity
 * N要素・Qクエリ・U更新・幅B。整列 O(Q log Q)、区間操作 O(QB+N^2/B+N)、
 * 時間操作 O(U min(Q,ceil(N/B)^2))。既定Bはmax(1,N^(2/3))程度、Q,U=O(N)なら O(N^(5/3)) 操作。
 * callback・Tの操作は別途。保持・実行時追加空間 O(N+Q+U)。登録は償却O(1)。
 * @code
 * #include "library/algorithm/mo_with_updates.hpp"
 * #include <cassert>
 * int main(){MoWithUpdates<int> mo({1,2,3});mo.add_query(0,3);mo.add_update(1,5);mo.add_query(0,2);std::vector<int>answers(2);int sum=0;
 * mo.run([&](int,int x){sum+=x;},[&](int,int x){sum-=x;},[&](int id){answers[id]=sum;});assert((answers==std::vector<int>{6,6}));}
 * @endcode
 */
template <class T> class MoWithUpdates {
public:
    /// @brief 初期配列をコピーして構築。@pre 要素数はINT_MAX以下。
    explicit MoWithUpdates(const std::vector<T> &a) : initial_(a), recorded_(a) {
        assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        n_ = static_cast<int>(a.size());
    }
    /// @brief 現時刻の[l,r)クエリを登録し、登録順IDを返す。@pre 0<=l<=r<=N、クエリ数<=INT_MAX。
    int add_query(int l, int r) {
        assert(0 <= l && l <= r && r <= n_ &&
               queries_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(queries_.size());
        queries_.push_back({l, r, static_cast<int>(updates_.size())});
        return id;
    }
    /// @brief indexの値をvalueに変える操作を登録し、更新IDを返す。@pre indexは[0,N)、更新数<=INT_MAX。
    int add_update(int index, const T &value) {
        assert(0 <= index && index < n_ &&
               updates_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
        int id = static_cast<int>(updates_.size());
        updates_.push_back({index, recorded_[index], value});
        recorded_[index] = value;
        return id;
    }
    /**
     * @brief add(index,const T& value),erase(index,const T& value),output(query_id)で実行。
     * @param block_size 正なら指定幅、0なら既定幅。@pre block_size>=0、集計状態を空に初期化済み。
     * @note 各runで初期配列から再実行する。元配列は変更しない。値参照はcallback後に保持しないこと。
     * outputは登録順とは限らない。Q=0ならcallbackを呼ばず、最後の集計状態は呼び出し側に残る。
     */
    template <class Add, class Erase, class Output>
    void run(Add add, Erase erase, Output output, int block_size = 0) const {
        assert(block_size >= 0);
        if (queries_.empty())
            return;
        int block =
            block_size ? block_size : std::max(1, static_cast<int>(std::pow(std::max(1, n_), 2.0 / 3.0)));
        std::vector<int> order(queries_.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            auto x = queries_[a], y = queries_[b];
            int lx = x.l / block, ly = y.l / block;
            if (lx != ly)
                return lx < ly;
            int rx = x.r / block, ry = y.r / block;
            if (rx != ry)
                return lx % 2 ? rx > ry : rx < ry;
            return rx % 2 ? x.time > y.time : x.time < y.time;
        });
        auto values = initial_;
        int l = 0, r = 0, time = 0;
        auto change = [&](const Update &u, const T &value) {
            bool inside = l <= u.index && u.index < r;
            if (inside)
                erase(u.index, values[u.index]);
            values[u.index] = value;
            if (inside)
                add(u.index, values[u.index]);
        };
        for (int id : order) {
            auto q = queries_[id];
            while (time < q.time) {
                const auto &u = updates_[time++];
                change(u, u.after);
            }
            while (time > q.time) {
                const auto &u = updates_[--time];
                change(u, u.before);
            }
            while (l > q.l) {
                --l;
                add(l, values[l]);
            }
            while (r < q.r) {
                add(r, values[r]);
                ++r;
            }
            while (l < q.l) {
                erase(l, values[l]);
                ++l;
            }
            while (r > q.r) {
                --r;
                erase(r, values[r]);
            }
            output(id);
        }
    }

private:
    struct Update {
        int index;
        T before, after;
    };
    struct Query {
        int l, r, time;
    };
    int n_;
    std::vector<T> initial_, recorded_;
    std::vector<Update> updates_;
    std::vector<Query> queries_;
};
