#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

/**
 * @brief 配列を暗黙の添字で管理する Implicit Treap。
 *
 * 挿入・削除・区間作用・区間積・反転・rotate を扱う。
 * reverse に対して非可換な op も扱えるよう、順方向・逆方向の積を両方保持する。
 *
 * @tparam S 区間積を表す型
 * @tparam op S 上の結合的演算
 * @tparam e op の単位元を返す関数
 * @tparam F S に作用する写像の型
 * @tparam mapping 長さ len の区間積 x に f を作用させる関数
 * @tparam composition f(g(x)) を表す写像合成。ACL と同じ順序
 * @tparam id F の恒等写像を返す関数
 * @note 区間 API はすべて半開区間 [l, r)。
 * @note 計算量は op / mapping / composition が O(1) の場合。木の高さは疑似乱数の優先度に依存し、最悪 O(N)。
 * @par Complexity
 * 構築・to_vector・clear・破棄 O(N)、size / empty / all_prod O(1)。
 * その他の主要操作は期待 O(log N)。保持空間 O(N)。
 *
 * @note 関数のシグネチャは S op(S,S)、S e()、S mapping(F,S,int)、F composition(F,F)、F id()。
 * @note op は区間の連結を表し e() は空区間の値。mapping(f,x,len) は len 個の要素への一様な作用後の区間積を返す。
 * @pre op(op(a,b),c) == op(a,op(b,c))、op(e(),a) == op(a,e()) == a。
 * @pre mapping(id(),x,n) == x、mapping(f,op(x,y),n+m) == op(mapping(f,x,n),mapping(f,y,m))。
 * @pre composition(f,g) は g の後に f を作用させる合成。mapping(composition(f,g),x,n) == mapping(f,mapping(g,x,n),n)。
 * @pre 写像の合成は結合的で id() が単位元。一様な作用は要素順の反転と可換であること。
 * @pre S / F はコピー・代入可能。要素数は int に収まり、途中の演算も各型に収まること。
 * @note コピー・ムーブはできない。空の構築は Treap tree;、初期列からは下の例のように生成する。
 * @note 再帰の深さは木の高さ。
 *
 * @code
 * #include "library/data_structure/implicit_treap.hpp"
 * #include <cassert>
 * long long op(long long a, long long b) { return a + b; }
 * long long e() { return 0; }
 * long long mapping(long long f, long long x, int len) {
 *     return x + f * len;
 * }
 * long long composition(long long f, long long g) { return f + g; }
 * long long id() { return 0; }
 *
 * using Treap = ImplicitTreap<
 *     long long, op, e,
 *     long long, mapping, composition, id
 * >;
 * int main() {
 *     Treap tree(std::vector<long long>{1, 2, 3});
 *     tree.apply(0, 2, 4);
 *     tree.reverse(0, 3);
 *     assert(tree.prod(0, 3) == 14);
 * }
 * @endcode
 */
template <class S, S (*op)(S, S), S (*e)(), class F,
          S (*mapping)(F, S, int), F (*composition)(F, F), F (*id)()>
class ImplicitTreap {
private:
    struct Node {
        S value;
        S prod;
        S rprod;
        F lazy;
        std::uint64_t priority;
        int size;
        bool rev;
        bool has_lazy;
        Node* l;
        Node* r;

        Node(const S& value_, std::uint64_t priority_)
            : value(value_), prod(value_), rprod(value_), lazy(id()),
              priority(priority_), size(1), rev(false), has_lazy(false),
              l(nullptr), r(nullptr) {}
    };

    using Tree = Node*;

    Tree root_ = nullptr;
    std::uint64_t rng_state_ = 88172645463325252ULL;

    static int size(Tree t) {
        return t ? t->size : 0;
    }

    static S prod(Tree t) {
        return t ? t->prod : e();
    }

    static S rprod(Tree t) {
        return t ? t->rprod : e();
    }

    std::uint64_t rng() {
        rng_state_ ^= rng_state_ << 7;
        rng_state_ ^= rng_state_ >> 9;
        return rng_state_;
    }

    static void apply_all(Tree t, const F& f) {
        if (!t) return;

        t->value = mapping(f, t->value, 1);
        t->prod = mapping(f, t->prod, t->size);
        t->rprod = mapping(f, t->rprod, t->size);

        if (t->has_lazy) {
            t->lazy = composition(f, t->lazy);
        } else {
            t->lazy = f;
            t->has_lazy = true;
        }
    }

    static void toggle_reverse(Tree t) {
        if (!t) return;

        std::swap(t->l, t->r);
        std::swap(t->prod, t->rprod);
        t->rev ^= true;
    }

    static void push(Tree t) {
        if (!t) return;

        if (t->rev) {
            toggle_reverse(t->l);
            toggle_reverse(t->r);
            t->rev = false;
        }

        if (t->has_lazy) {
            apply_all(t->l, t->lazy);
            apply_all(t->r, t->lazy);
            t->lazy = id();
            t->has_lazy = false;
        }
    }

    static void pull(Tree t) {
        if (!t) return;

        t->size = 1 + size(t->l) + size(t->r);
        t->prod = op(prod(t->l), op(t->value, prod(t->r)));
        t->rprod = op(rprod(t->r), op(t->value, rprod(t->l)));
    }

    static std::pair<Tree, Tree> split(Tree t, int k) {
        if (!t) return {nullptr, nullptr};

        push(t);

        if (size(t->l) >= k) {
            auto [a, b] = split(t->l, k);
            t->l = b;
            pull(t);
            return {a, t};
        }

        auto [a, b] = split(t->r, k - size(t->l) - 1);
        t->r = a;
        pull(t);
        return {t, b};
    }

    static Tree merge(Tree a, Tree b) {
        if (!a) return b;
        if (!b) return a;

        if (a->priority > b->priority) {
            push(a);
            a->r = merge(a->r, b);
            pull(a);
            return a;
        }

        push(b);
        b->l = merge(a, b->l);
        pull(b);
        return b;
    }

    static void destroy(Tree t) {
        if (!t) return;

        destroy(t->l);
        destroy(t->r);
        delete t;
    }

    static void rebuild(Tree t) {
        if (!t) return;

        rebuild(t->l);
        rebuild(t->r);
        pull(t);
    }

    static void collect(Tree t, std::vector<S>& out) {
        if (!t) return;

        push(t);
        collect(t->l, out);
        out.push_back(t->value);
        collect(t->r, out);
        pull(t);
    }

public:
    /**
     * @brief 空の列を構築する。
     * @par Complexity
     * O(1)。
     */
    ImplicitTreap() = default;

    /**
     * @brief 初期列から木を構築する。
     * @param a 0-indexed の要素列。空列も指定できる。
     * @par Complexity
     * O(N) 時間・空間。
     */
    explicit ImplicitTreap(const std::vector<S>& a) {
        build(a);
    }

    /**
     * @brief 全ノードを解放する。
     * @par Complexity
     * O(N)。
     */
    ~ImplicitTreap() {
        clear();
    }

    ImplicitTreap(const ImplicitTreap&) = delete;
    ImplicitTreap& operator=(const ImplicitTreap&) = delete;

    /**
     * @brief 現在の要素数を返す。
     * @return 要素数（空列は 0）。
     * @par Complexity
     * O(1)。
     */
    int size() const {
        return size(root_);
    }

    /**
     * @brief 空列か判定する。
     * @return 要素がなければ true。
     * @par Complexity
     * O(1)。
     */
    bool empty() const {
        return root_ == nullptr;
    }

    /**
     * @brief 全要素を削除して空列にする。
     * @par Complexity
     * O(N)。
     */
    void clear() {
        destroy(root_);
        root_ = nullptr;
    }

    /**
     * @brief 既存の列を破棄し、vector の内容で木を再構築する。
     * @param a 初期列（要素数は int に収まること）。
     * @par Complexity
     * 元の要素数 M、新しい要素数 N として O(M + N) 時間、O(N) 保持空間。
     */
    void build(const std::vector<S>& a) {
        clear();
        if (a.empty()) return;

        std::vector<Tree> st;
        st.reserve(a.size());

        for (const S& x : a) {
            Tree cur = new Node(x, rng());
            Tree last = nullptr;

            while (!st.empty() && st.back()->priority < cur->priority) {
                last = st.back();
                st.pop_back();
            }

            cur->l = last;
            if (!st.empty()) st.back()->r = cur;
            st.push_back(cur);
        }

        root_ = st.front();
        rebuild(root_);
    }

    /**
     * @brief pos の直前に x を挿入する。
     * @param pos 挿入位置
     * @param x 挿入する値
     * @pre 0 <= pos <= size()
     * @par Complexity
     * 期待 O(log N)
     */
    void insert(int pos, const S& x) {
        assert(0 <= pos && pos <= size());

        auto [a, b] = split(root_, pos);
        root_ = merge(merge(a, new Node(x, rng())), b);
    }

    /**
     * @brief pos 番目の要素を削除する。
     * @param pos 削除する位置
     * @pre 0 <= pos < size()
     * @par Complexity
     * 期待 O(log N)
     */
    void erase(int pos) {
        assert(0 <= pos && pos < size());

        auto [a, bc] = split(root_, pos);
        auto [b, c] = split(bc, 1);

        destroy(b);
        root_ = merge(a, c);
    }

    /**
     * @brief pos 番目の要素を x に置き換える。
     * @param pos 更新する位置
     * @param x 新しい値
     * @pre 0 <= pos < size()
     * @par Complexity
     * 期待 O(log N)
     */
    void set(int pos, const S& x) {
        assert(0 <= pos && pos < size());

        auto [a, bc] = split(root_, pos);
        auto [b, c] = split(bc, 1);

        push(b);
        b->value = b->prod = b->rprod = x;
        b->lazy = id();
        b->has_lazy = false;
        b->rev = false;

        root_ = merge(a, merge(b, c));
    }

    /**
     * @brief pos 番目の値を取得する。
     * @param pos 0-indexed の位置。
     * @return 作用を反映した要素値。
     * @pre 0 <= pos < size()。
     * @par Complexity
     * 期待 O(log N)。
     */
    S get(int pos) {
        assert(0 <= pos && pos < size());
        return prod(pos, pos + 1);
    }

    /**
     * @brief 区間 [l, r) の積を返す。
     * @param l 区間の左端
     * @param r 区間の右端
     * @return op による左からの区間積。空区間は e()。
     * @pre 0 <= l <= r <= size()
     * @par Complexity
     * 期待 O(log N)
     */
    S prod(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return e();

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        S res = b->prod;
        root_ = merge(a, merge(b, c));
        return res;
    }

    /**
     * @brief 列全体の区間積を取得する。
     * @return 左からの区間積。空列は e()。
     * @par Complexity
     * O(1)。
     */
    S all_prod() const {
        return prod(root_);
    }

    /**
     * @brief 区間 [l, r) に写像 f を作用させる。
     * @param l 区間の左端
     * @param r 区間の右端
     * @param f 適用する写像
     * @pre 0 <= l <= r <= size()
     * @par Complexity
     * 期待 O(log N)
     */
    void apply(int l, int r, const F& f) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return;

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        apply_all(b, f);
        root_ = merge(a, merge(b, c));
    }

    /**
     * @brief 区間 [l, r) の要素順を反転する。
     * @param l 区間の左端
     * @param r 区間の右端
     * @pre 0 <= l <= r <= size()
     * @par Complexity
     * 期待 O(log N)
     */
    void reverse(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return;

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        toggle_reverse(b);
        root_ = merge(a, merge(b, c));
    }

    /**
     * @brief std::rotate と同様に [l, m), [m, r) を [m, r), [l, m) へ並べ替える。
     * @param l 区間の左端
     * @param m 回転の境界
     * @param r 区間の右端
     * @pre 0 <= l <= m <= r <= size()
     * @par Complexity
     * 期待 O(log N)
     */
    void rotate(int l, int m, int r) {
        assert(0 <= l && l <= m && m <= r && r <= size());
        if (l == m || m == r) return;

        auto [abc, d] = split(root_, r);
        auto [ab, c] = split(abc, m);
        auto [a, b] = split(ab, l);

        root_ = merge(a, merge(c, merge(b, d)));
    }

    /**
     * @brief 現在の列を通常の配列に取り出す。
     * @return 遅延作用と反転を反映した 0-indexed の要素列。
     * @par Complexity
     * O(N) 時間・出力空間。
     */
    std::vector<S> to_vector() {
        std::vector<S> res;
        res.reserve(size());
        collect(root_, res);
        return res;
    }

    /**
     * @brief get(pos) と同じ。値を返すため代入による更新はできない。
     * @param pos 0-indexed の位置。
     * @return 要素値。
     * @pre 0 <= pos < size()。
     * @par Complexity
     * 期待 O(log N)。
     */
    S operator[](int pos) {
        return get(pos);
    }
};
