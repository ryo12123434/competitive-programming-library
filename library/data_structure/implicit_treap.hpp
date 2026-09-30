#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

// Implicit Treap
//
// 配列を暗黙の添字で管理する平衡二分木。
// 挿入・削除・区間作用・区間積・反転・rotate を期待 O(log N) で行う。
// vector からの構築は O(N)、to_vector() は O(N)。
//
// S:
//   区間積を表す型。
// op(a, b):
//   S 上の結合的演算。
// e():
//   op の単位元。
// F:
//   S に作用する写像の型。
// mapping(f, x, len):
//   長さ len の区間積 x に f を作用させた結果。
// composition(f, g):
//   f(g(x)) を表す合成。ACL と同じ順序。
// id():
//   F の恒等写像。
//
// 区間 API はすべて半開区間 [l, r)。
// reverse に対して非可換な op も扱えるよう、順方向・逆方向の積を両方保持する。
//
// 使用例: 区間加算・区間和
//
// long long op(long long a, long long b) { return a + b; }
// long long e() { return 0; }
// long long mapping(long long f, long long x, int len) {
//     return x + f * len;
// }
// long long composition(long long f, long long g) {
//     return f + g;
// }
// long long id() { return 0; }
//
// using Treap = ImplicitTreap<
//     long long, op, e,
//     long long, mapping, composition, id
// >;
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
    ImplicitTreap() = default;

    explicit ImplicitTreap(const std::vector<S>& a) {
        build(a);
    }

    ~ImplicitTreap() {
        clear();
    }

    ImplicitTreap(const ImplicitTreap&) = delete;
    ImplicitTreap& operator=(const ImplicitTreap&) = delete;

    int size() const {
        return size(root_);
    }

    bool empty() const {
        return root_ == nullptr;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
    }

    // O(N)
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

    // pos の直前に x を挿入する。
    // 0 <= pos <= size()
    void insert(int pos, const S& x) {
        assert(0 <= pos && pos <= size());

        auto [a, b] = split(root_, pos);
        root_ = merge(merge(a, new Node(x, rng())), b);
    }

    // pos 番目を削除する。
    void erase(int pos) {
        assert(0 <= pos && pos < size());

        auto [a, bc] = split(root_, pos);
        auto [b, c] = split(bc, 1);

        destroy(b);
        root_ = merge(a, c);
    }

    // pos 番目を x に置き換える。
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

    S get(int pos) {
        assert(0 <= pos && pos < size());
        return prod(pos, pos + 1);
    }

    // [l, r) の積を返す。
    S prod(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return e();

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        S res = b->prod;
        root_ = merge(a, merge(b, c));
        return res;
    }

    S all_prod() const {
        return prod(root_);
    }

    // [l, r) に f を作用させる。
    void apply(int l, int r, const F& f) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return;

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        apply_all(b, f);
        root_ = merge(a, merge(b, c));
    }

    // [l, r) を反転する。
    void reverse(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r) return;

        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);

        toggle_reverse(b);
        root_ = merge(a, merge(b, c));
    }

    // std::rotate と同じく [l, m), [m, r) を [m, r), [l, m) にする。
    void rotate(int l, int m, int r) {
        assert(0 <= l && l <= m && m <= r && r <= size());
        if (l == m || m == r) return;

        auto [abc, d] = split(root_, r);
        auto [ab, c] = split(abc, m);
        auto [a, b] = split(ab, l);

        root_ = merge(a, merge(c, merge(b, d)));
    }

    std::vector<S> to_vector() {
        std::vector<S> res;
        res.reserve(size());
        collect(root_, res);
        return res;
    }

    S operator[](int pos) {
        return get(pos);
    }
};
