#pragma once
#include <atcoder/dsu>
#include <cassert>
#include <vector>

/**
 * @brief ACL dsuに連結成分ごとのモノイド値を付加する。素集合管理を再実装しない。
 * @tparam S 値型。S op(S,S)は結合的、S e()は両側単位元。
 * @note 非可換でもmerge(a,b)の「a側の値→b側の値」の順で結合する。代表元の選択と順序は独立。
 * get/setの対象は頂点値ではなく成分値。setによる任意の頂点値変更は提供しない。
 * @par Complexity 構築O(N)時間・空間。leader/same/size/merge/get/setは償却O(alpha(N))。
 * 値の演算・コピーO(1)を仮定する。頂点は0-indexed。
 * @code
 * #include "library/data_structure/union_find_monoid.hpp"
 * #include <cassert>
 * long long sum(long long a,long long b){return a+b;}long long zero(){return 0;}
 * int main(){UnionFindMonoid<long long,sum,zero>u(std::vector<long long>{2,3});u.merge(0,1);assert(u.get(1)==5);u.set(0,7);assert(u.get(1)==7);}
 * @endcode
 */
template <class S, S (*op)(S, S), S (*e)()> class UnionFindMonoid {
    atcoder::dsu dsu_;
    std::vector<S> values_;
    static std::vector<S> initial(int n) {
        assert(n >= 0);
        return std::vector<S>(n, e());
    }

public:
    /// @brief 各頂点を独立した成分とし、値をe()で初期化。@pre n>=0。
    explicit UnionFindMonoid(int n) : UnionFindMonoid(initial(n)) {}
    /// @brief 各頂点の初期値から構築。@pre 長さはintに収まる。
    explicit UnionFindMonoid(const std::vector<S> &values)
        : dsu_(static_cast<int>(values.size())), values_(values) {}
    int leader(int v) { return dsu_.leader(v); }
    bool same(int a, int b) { return dsu_.same(a, b); }
    int size(int v) { return dsu_.size(v); }
    /// @brief a側・b側の順に値を結合し、新しい代表元を返す。同一成分なら値を変えない。
    int merge(int a, int b) {
        int x = leader(a), y = leader(b);
        if (x == y)
            return x;
        S combined = op(values_[x], values_[y]);
        int root = dsu_.merge(x, y);
        values_[root] = combined;
        return root;
    }
    /// @brief vの成分値を返す。@pre v∈[0,N)。
    S get(int v) { return values_[leader(v)]; }
    /// @brief vの成分値全体を上書きする。@pre v∈[0,N)。
    void set(int v, const S &value) { values_[leader(v)] = value; }
};
