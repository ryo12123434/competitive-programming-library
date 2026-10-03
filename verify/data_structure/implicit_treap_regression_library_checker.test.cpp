#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// 非可換な文字列連結を使い、反転・区間作用・set・rotate・再構築を列と比較する。
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "../../library/data_structure/implicit_treap.hpp"

std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return ""; }
std::string mapping(int f, std::string x, int) {
    for (char& c : x) c = static_cast<char>('a' + (c - 'a' + f) % 26);
    return x;
}
int composition(int f, int g) { return (f + g) % 26; }
int id() { return 0; }
using Treap = ImplicitTreap<std::string, op, e, int, mapping, composition, id>;

int main() {
    std::mt19937 rng(1712);
    Treap tree;
    std::vector<std::string> reference;
    assert(tree.empty() && tree.prod(0, 0).empty());
    for (int step = 0; step < 2000; ++step) {
        int n = static_cast<int>(reference.size()), action = rng() % 9;
        int l = rng() % (n + 1), r = rng() % (n + 1);
        if (l > r) std::swap(l, r);
        if (action == 0 && n < 40) {
            std::string x(1, static_cast<char>('a' + rng() % 26));
            tree.insert(l, x); reference.insert(reference.begin() + l, x);
        } else if (action == 1 && n > 0) {
            int p = rng() % n;
            tree.erase(p); reference.erase(reference.begin() + p);
        } else if (action == 2) {
            tree.reverse(l, r); std::reverse(reference.begin() + l, reference.begin() + r);
        } else if (action == 3) {
            int shift = rng() % 26;
            tree.apply(l, r, shift);
            for (int p = l; p < r; ++p) reference[p] = mapping(shift, reference[p], 1);
        } else if (action == 4 && n > 0) {
            int p = rng() % n;
            std::string x(1, static_cast<char>('a' + rng() % 26));
            tree.set(p, x); reference[p] = x;
        } else if (action == 5) {
            int m = l + rng() % (r - l + 1);
            tree.rotate(l, m, r); std::rotate(reference.begin() + l, reference.begin() + m, reference.begin() + r);
        } else if (action == 6) {
            tree.build(reference);
        } else if (action == 7 && rng() % 10 == 0) {
            tree.clear(); reference.clear();
        }
        assert(tree.size() == static_cast<int>(reference.size()));
        assert(tree.to_vector() == reference);
        std::string all;
        for (const auto& x : reference) all += x;
        assert(tree.all_prod() == all);
        n = static_cast<int>(reference.size());
        l = rng() % (n + 1); r = rng() % (n + 1);
        if (l > r) std::swap(l, r);
        std::string substring;
        for (int p = l; p < r; ++p) substring += reference[p];
        assert(tree.prod(l, r) == substring);
        if (n > 0) assert(tree.get(n - 1) == reference.back());
    }
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
