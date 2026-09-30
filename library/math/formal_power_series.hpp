#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

#include <atcoder/convolution>

// Formal Power Series
//
// 係数を昇べき順に保持する。
// f[i] は x^i の係数。
//
// Mint:
// - atcoder::static_modint など、四則演算・inv()・pow() を持つ型を想定する。
// - 多項式積には atcoder::convolution を利用する。
//
// 主な計算量（次数 N）:
// - 加減算: O(N)
// - 乗算: O(N log N)
// - inv / log / exp / pow: O(N log N)
//
// pre(n) は先頭 n 項、区間ではなく次数打ち切りを表す。
template <class Mint>
struct FormalPowerSeries : std::vector<Mint> {
    using std::vector<Mint>::vector;
    using FPS = FormalPowerSeries;

    FormalPowerSeries(const std::vector<Mint>& v) : std::vector<Mint>(v) {}

    FPS pre(int n) const {
        assert(n >= 0);
        return FPS(this->begin(), this->begin() + std::min<int>(this->size(), n));
    }

    FPS rev() const {
        FPS res = *this;
        std::reverse(res.begin(), res.end());
        return res;
    }

    FPS& normalize() {
        while (!this->empty() && this->back() == Mint(0)) {
            this->pop_back();
        }
        return *this;
    }

    FPS operator-() const {
        FPS res = *this;
        for (Mint& x : res) x = -x;
        return res;
    }

    FPS operator+(const Mint& x) const { return FPS(*this) += x; }
    FPS operator-(const Mint& x) const { return FPS(*this) -= x; }
    FPS operator*(const Mint& x) const { return FPS(*this) *= x; }
    FPS operator/(const Mint& x) const { return FPS(*this) /= x; }

    FPS operator+(const FPS& rhs) const { return FPS(*this) += rhs; }
    FPS operator-(const FPS& rhs) const { return FPS(*this) -= rhs; }
    FPS operator*(const FPS& rhs) const { return FPS(*this) *= rhs; }
    FPS operator/(const FPS& rhs) const { return FPS(*this) /= rhs; }
    FPS operator%(const FPS& rhs) const { return FPS(*this) %= rhs; }

    FPS operator<<(int k) const { return FPS(*this) <<= k; }
    FPS operator>>(int k) const { return FPS(*this) >>= k; }

    FPS& operator+=(const Mint& x) {
        if (this->empty()) this->resize(1);
        (*this)[0] += x;
        return *this;
    }

    FPS& operator-=(const Mint& x) {
        if (this->empty()) this->resize(1);
        (*this)[0] -= x;
        return *this;
    }

    FPS& operator+=(const FPS& rhs) {
        if (rhs.size() > this->size()) this->resize(rhs.size());
        for (int i = 0; i < static_cast<int>(rhs.size()); ++i) {
            (*this)[i] += rhs[i];
        }
        return normalize();
    }

    FPS& operator-=(const FPS& rhs) {
        if (rhs.size() > this->size()) this->resize(rhs.size());
        for (int i = 0; i < static_cast<int>(rhs.size()); ++i) {
            (*this)[i] -= rhs[i];
        }
        return normalize();
    }

    FPS& operator*=(const Mint& x) {
        for (Mint& a : *this) a *= x;
        return *this;
    }

    FPS& operator/=(const Mint& x) {
        assert(x != Mint(0));
        const Mint inv_x = x.inv();
        for (Mint& a : *this) a *= inv_x;
        return *this;
    }

    FPS& operator*=(const FPS& rhs) {
        if (this->empty() || rhs.empty()) {
            this->clear();
            return *this;
        }

        const auto res = atcoder::convolution(
            static_cast<const std::vector<Mint>&>(*this),
            static_cast<const std::vector<Mint>&>(rhs)
        );
        this->assign(res.begin(), res.end());
        return *this;
    }

    // x^k を掛ける。
    FPS& operator<<=(int k) {
        assert(k >= 0);
        this->insert(this->begin(), k, Mint(0));
        return *this;
    }

    // x^k で割り、低次 k 項を捨てる。
    FPS& operator>>=(int k) {
        assert(k >= 0);
        if (k >= static_cast<int>(this->size())) {
            this->clear();
            return *this;
        }
        this->erase(this->begin(), this->begin() + k);
        return *this;
    }

    Mint eval(const Mint& x) const {
        Mint res = 0;
        for (int i = static_cast<int>(this->size()) - 1; i >= 0; --i) {
            res = res * x + (*this)[i];
        }
        return res;
    }

    // 商を求める。rhs は末尾係数が 0 でないこと。
    FPS& operator/=(const FPS& rhs) {
        assert(!rhs.empty());
        assert(rhs.back() != Mint(0));

        normalize();
        if (this->size() < rhs.size()) {
            this->clear();
            return *this;
        }

        const int need =
            static_cast<int>(this->size()) - static_cast<int>(rhs.size()) + 1;

        *this = (this->rev().pre(need) * inv(rhs.rev(), need)).pre(need).rev();
        return normalize();
    }

    // 剰余を求める。rhs は末尾係数が 0 でないこと。
    FPS& operator%=(const FPS& rhs) {
        assert(!rhs.empty());
        assert(rhs.back() != Mint(0));

        normalize();
        const FPS q = (*this) / rhs;
        *this -= q * rhs;
        return normalize();
    }

    // f'(x)
    friend FPS diff(const FPS& f) {
        if (f.empty()) return {};

        const int n = static_cast<int>(f.size());
        FPS res(n - 1);
        for (int i = 1; i < n; ++i) {
            res[i - 1] = f[i] * i;
        }
        return res;
    }

    // integral f(x) dx、積分定数は 0。
    friend FPS integrate(const FPS& f) {
        const int n = static_cast<int>(f.size());
        FPS res(n + 1, Mint(0));

        for (int i = 0; i < n; ++i) {
            res[i + 1] = f[i] / Mint(i + 1);
        }
        return res;
    }

    // 1 / f mod x^deg
    // 前提: deg == 0 または f[0] != 0
    friend FPS inv(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(!f.empty() && f[0] != Mint(0));

        FPS res{f[0].inv()};

        for (int n = 1; n < deg; n <<= 1) {
            const int m = std::min(n << 1, deg);
            res = (res + res - res * res * f.pre(m)).pre(m);
        }

        res.resize(deg);
        return res;
    }

    friend FPS inv(const FPS& f) {
        return inv(f, static_cast<int>(f.size()));
    }

    // log(f) mod x^deg
    // 前提: deg == 0 または f[0] == 1
    friend FPS log(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(!f.empty() && f[0] == Mint(1));

        FPS res = integrate(diff(f) * inv(f, deg));
        res.resize(deg);
        return res;
    }

    friend FPS log(const FPS& f) {
        return log(f, static_cast<int>(f.size()));
    }

    // exp(f) mod x^deg
    // 前提: f が空、または f[0] == 0
    friend FPS exp(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(f.empty() || f[0] == Mint(0));

        FPS res{Mint(1)};

        for (int n = 1; n < deg; n <<= 1) {
            const int m = std::min(n << 1, deg);
            res = (res * (f.pre(m) - log(res, m) + Mint(1))).pre(m);
        }

        res.resize(deg);
        return res;
    }

    friend FPS exp(const FPS& f) {
        return exp(f, static_cast<int>(f.size()));
    }

    // f^k mod x^deg
    // 前提: k >= 0
    friend FPS pow(const FPS& f, long long k, int deg) {
        assert(k >= 0);
        assert(deg >= 0);

        if (deg == 0) return {};

        if (k == 0) {
            FPS res(deg, Mint(0));
            res[0] = Mint(1);
            return res;
        }

        int first = 0;
        while (first < static_cast<int>(f.size()) && f[first] == Mint(0)) {
            ++first;
        }

        if (first == static_cast<int>(f.size())) {
            return FPS(deg, Mint(0));
        }

        if (first > 0 && k >= (deg + first - 1LL) / first) {
            return FPS(deg, Mint(0));
        }

        const long long shift_ll = 1LL * first * k;
        if (shift_ll >= deg) return FPS(deg, Mint(0));

        const int shift = static_cast<int>(shift_ll);
        const int need = deg - shift;
        const Mint lead = f[first];

        FPS g = (f >> first) / lead;
        FPS res = exp(log(g, need) * Mint(k), need) * lead.pow(k);
        res <<= shift;
        res.resize(deg);
        return res;
    }

    friend FPS pow(const FPS& f, long long k) {
        return pow(f, k, static_cast<int>(f.size()));
    }

    friend FPS gcd(FPS a, FPS b) {
        a.normalize();
        b.normalize();

        while (!b.empty()) {
            a %= b;
            std::swap(a, b);
        }

        if (!a.empty()) {
            a /= a.back();
        }
        return a;
    }
};
