#pragma once

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include <atcoder/convolution>

/**
 * @brief 形式的冪級数 (Formal Power Series)。
 *
 * 係数を昇べき順に保持し、f[i] は x^i の係数を表す。
 * 多項式積には atcoder::convolution を利用する。
 *
 * @tparam Mint atcoder::static_modint など、四則演算・inv()・pow() を持つ型
 * @pre 係数数・打ち切る項数は int に収まり、乗算は ACL の法と畳み込み長の制約を満たすこと。
 * @note pre(n) は先頭 n 項を取り出し、次数を n 項で打ち切る操作を表す。
 * @par Complexity
 * 次数 N に対して、加減算 O(N)、乗算 O(N log N)、
 * inv / log / exp は O(N log N)、pow は O(N log N + log k)。
 */
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
        const Mint factor = x;
        for (Mint& a : *this) a *= factor;
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

    /**
     * @brief x^k を掛ける。
     * @param k シフトする次数
     * @return 自身への参照
     * @pre k >= 0
     * @par Complexity
     * O(N + k)
     */
    FPS& operator<<=(int k) {
        assert(k >= 0);
        this->insert(this->begin(), k, Mint(0));
        return *this;
    }

    /**
     * @brief x^k で割り、低次 k 項を捨てる。
     * @param k 捨てる低次項数
     * @return 自身への参照
     * @pre k >= 0
     * @par Complexity
     * O(N)
     */
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

    /**
     * @brief rhs で割った商を求める。
     * @param rhs 除数
     * @return 自身への参照
     * @pre rhs は空でなく、最高次係数が 0 でないこと。
     */
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

    /**
     * @brief rhs で割った剰余を求める。
     * @param rhs 除数
     * @return 自身への参照
     * @pre rhs は空でなく、最高次係数が 0 でないこと。
     */
    FPS& operator%=(const FPS& rhs) {
        assert(!rhs.empty());
        assert(rhs.back() != Mint(0));

        normalize();
        const FPS q = (*this) / rhs;
        *this -= q * rhs;
        return normalize();
    }

    /**
     * @brief f の形式微分を返す。
     * @param f 微分する FPS
     * @return f'(x)
     * @par Complexity
     * O(N)
     */
    friend FPS diff(const FPS& f) {
        if (f.empty()) return {};

        const int n = static_cast<int>(f.size());
        FPS res(n - 1);
        for (int i = 1; i < n; ++i) {
            res[i - 1] = f[i] * i;
        }
        return res;
    }

    /**
     * @brief f の形式積分を返す。積分定数は 0。
     * @param f 積分する FPS
     * @return integral f(x) dx
     * @pre 1..N が法の下で可逆であること。素数法では N < mod。
     * @par Complexity
     * O(N) 回の四則演算と逆元計算 1 回。
     */
    friend FPS integrate(const FPS& f) {
        const int n = static_cast<int>(f.size());
        FPS res(static_cast<std::size_t>(n) + 1, Mint(1));
        for (int i = 0; i < n; ++i) res[i + 1] = res[i] * Mint(i + 1);
        Mint inv_fact = res[n].inv();
        for (int i = n; i >= 1; --i) {
            res[i] = f[i - 1] * res[i - 1] * inv_fact;
            inv_fact *= Mint(i);
        }
        res[0] = Mint(0);
        return res;
    }

    /**
     * @brief 1 / f mod x^deg を返す。
     * @param f 逆数を求める FPS
     * @param deg 打ち切る項数
     * @return 1 / f mod x^deg
     * @pre deg >= 0。deg > 0 なら f[0] != 0。
     * @par Complexity
     * O(deg log deg)
     */
    friend FPS inv(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(!f.empty() && f[0] != Mint(0));

        FPS res{f[0].inv()};

        for (int n = 1; n < deg;) {
            const int m = n > deg - n ? deg : n * 2;
            res = (res + res - res * res * f.pre(m)).pre(m);
            n = m;
        }

        res.resize(deg);
        return res;
    }

    friend FPS inv(const FPS& f) {
        return inv(f, static_cast<int>(f.size()));
    }

    /**
     * @brief log(f) mod x^deg を返す。
     * @param f 対数を求める FPS
     * @param deg 打ち切る項数
     * @return log(f) mod x^deg
     * @pre deg >= 0。deg > 0 なら f[0] == 1。1..deg-1 が法の下で可逆であること。
     * @par Complexity
     * O(deg log deg)
     */
    friend FPS log(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(!f.empty() && f[0] == Mint(1));

        FPS res = integrate((diff(f.pre(deg)) * inv(f, deg)).pre(deg - 1));
        res.resize(deg);
        return res;
    }

    friend FPS log(const FPS& f) {
        return log(f, static_cast<int>(f.size()));
    }

    /**
     * @brief exp(f) mod x^deg を返す。
     * @param f 指数関数を求める FPS
     * @param deg 打ち切る項数
     * @return exp(f) mod x^deg
     * @pre deg >= 0。f が空、または f[0] == 0。1..deg-1 が法の下で可逆であること。
     * @par Complexity
     * O(deg log deg)
     */
    friend FPS exp(const FPS& f, int deg) {
        assert(deg >= 0);
        if (deg == 0) return {};
        assert(f.empty() || f[0] == Mint(0));

        FPS res{Mint(1)};

        for (int n = 1; n < deg;) {
            const int m = n > deg - n ? deg : n * 2;
            res = (res * (f.pre(m) - log(res, m) + Mint(1))).pre(m);
            n = m;
        }

        res.resize(deg);
        return res;
    }

    friend FPS exp(const FPS& f) {
        return exp(f, static_cast<int>(f.size()));
    }

    /**
     * @brief f^k mod x^deg を返す。
     * @param f 累乗する FPS
     * @param k 指数
     * @param deg 打ち切る項数
     * @return f^k mod x^deg
     * @pre k >= 0、deg >= 0。log/exp を使う場合は 1..deg-1 が法の下で可逆であること。
     * @par Complexity
     * O(deg log deg + log k)
     */
    friend FPS pow(const FPS& f, long long k, int deg) {
        assert(k >= 0);
        assert(deg >= 0);

        if (deg == 0) return {};

        if (k == 0) {
            FPS res(deg, Mint(0));
            res[0] = Mint(1);
            return res;
        }

        const int limit = std::min<int>(f.size(), deg);
        int first = 0;
        while (first < limit && f[first] == Mint(0)) {
            ++first;
        }

        if (first == limit) {
            return FPS(deg, Mint(0));
        }

        if (first > 0 && k >= (static_cast<long long>(deg) + first - 1) / first) {
            return FPS(deg, Mint(0));
        }

        const long long shift_ll = 1LL * first * k;
        if (shift_ll >= deg) return FPS(deg, Mint(0));

        const int shift = static_cast<int>(shift_ll);
        const int need = deg - shift;
        const Mint lead = f[first];

        FPS g = (f.pre(first + need) >> first) / lead;
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
