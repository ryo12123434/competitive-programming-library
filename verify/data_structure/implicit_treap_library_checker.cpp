// Problem: Library Checker - Dynamic Sequence Range Affine Range Sum
// URL: https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum
// Target: library/data_structure/implicit_treap.hpp

#include <iostream>
#include <utility>
#include <vector>

#include <atcoder/modint>

#include "../../library/data_structure/implicit_treap.hpp"

using mint = atcoder::modint998244353;

struct Affine {
    mint a;
    mint b;
};

mint op(mint x, mint y) {
    return x + y;
}

mint e() {
    return 0;
}

mint mapping(Affine f, mint x, int len) {
    return f.a * x + f.b * len;
}

Affine composition(Affine f, Affine g) {
    return {f.a * g.a, f.a * g.b + f.b};
}

Affine id() {
    return {1, 0};
}

using Treap = ImplicitTreap<
    mint, op, e,
    Affine, mapping, composition, id
>;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    std::vector<mint> a(n);
    for (auto& x : a) {
        int v;
        std::cin >> v;
        x = v;
    }

    Treap tr(a);

    while (q--) {
        int type;
        std::cin >> type;

        if (type == 0) {
            int i, x;
            std::cin >> i >> x;
            tr.insert(i, x);
        } else if (type == 1) {
            int i;
            std::cin >> i;
            tr.erase(i);
        } else if (type == 2) {
            int l, r;
            std::cin >> l >> r;
            tr.reverse(l, r);
        } else if (type == 3) {
            int l, r, b, c;
            std::cin >> l >> r >> b >> c;
            tr.apply(l, r, Affine{b, c});
        } else {
            int l, r;
            std::cin >> l >> r;
            std::cout << tr.prod(l, r).val() << '\n';
        }
    }
}
