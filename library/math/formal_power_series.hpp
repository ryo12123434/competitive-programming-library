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
 * 項数 N に対して、加減算 O(N)、乗算 O(N log N)、
 * inv / log / exp は O(N log N)、pow は O(N log N + log k)。
 *
 * @note Mint は ACL の convolution に対応する static_modint を使用する。係数演算の法は固定する。
 * @pre inv() を呼ぶ係数は可逆であること。素数法なら非零係数は可逆。
 * @note inv / log / exp / pow / diff / integrate / gcd はメンバではなく、FPS を引数とする非メンバ関数（ADL で呼び出す）。
 * @note 空列も零多項式を表す。normalize は高次の零を除くが、他の全演算が正規化するわけではない。
 * @note pre(n) は不足する項を零埋めしない。inv / log / exp / pow の返り値は指定した項数になる。
 * @note STL vector と同じ構築・係数アクセスも利用できる。以下の例は定数項 1 の逆数と形式対数。
 * @note 計算量は法を固定し係数演算を O(1) とする。逆元計算のコストは別途含む。保持空間 O(N)。
 *
 * @code
 * #include "library/math/formal_power_series.hpp"
 * #include <cassert>
 * #include <atcoder/modint>
 * int main() {
 *     using Mint = atcoder::modint998244353;
 *     using FPS = FormalPowerSeries<Mint>;
 *     FPS f{1, 2, 3};
 *     auto inverse = inv(f, 4);
 *     auto logarithm = log(f, 4);
 *     assert(inverse.size() == 4 && inverse[0] == Mint(1));
 *     assert(exp(logarithm, 4) == (FPS{1, 2, 3, 0}));
 * }
 * @endcode
 */
template <class Mint>
struct FormalPowerSeries : std::vector<Mint> {
    using std::vector<Mint>::vector;
    using FPS = FormalPowerSeries;

    /**
     * @brief 通常の係数配列から構築する。
     * @param v 定数項から昇べき順に並べた係数。
     * @par Complexity
     * O(N) 時間・空間。
     */
    FormalPowerSeries(const std::vector<Mint>& v) : std::vector<Mint>(v) {}

    /**
     * @brief 先頭 n 項までをコピーする。零埋めは行わない。
     * @param n 取り出す項数（次数ではない）。
     * @return min(n, size()) 項の FPS。
     * @pre n >= 0。
     * @par Complexity
     * O(min(n, N)) 時間・出力空間。
     */
    FPS pre(int n) const {
        assert(n >= 0);
        return FPS(this->begin(), this->begin() + std::min<int>(this->size(), n));
    }

    /**
     * @brief 係数列の順序を反転したコピーを返す。
     * @return size() 項の逆順の係数列。
     * @par Complexity
     * O(N) 時間・出力空間。
     */
    FPS rev() const {
        FPS res = *this;
        std::reverse(res.begin(), res.end());
        return res;
    }

    /**
     * @brief 高次側の零係数を削除する。零多項式は空列になる。
     * @return 自身への参照。
     * @par Complexity
     * 削除する項数 K に対して O(K + 1)。
     */
    FPS& normalize() {
        while (!this->empty() && this->back() == Mint(0)) {
            this->pop_back();
        }
        return *this;
    }

    /**
     * @brief 全係数の符号を反転する。
     * @return 項数を保った符号反転後のコピー。
     * @par Complexity
     * O(N) 時間・出力空間。
     */
    FPS operator-() const {
        FPS res = *this;
        for (Mint& x : res) x = -x;
        return res;
    }

    /**
     * @brief 定数項に x を加算したコピーを返す。
     * @param x スカラー値。
     * @return 演算後の FPS。
     * @par Complexity
     * O(N) 時間、O(N + 1) 出力空間。
     */
    FPS operator+(const Mint& x) const { return FPS(*this) += x; }
    /**
     * @brief 定数項から x を減算したコピーを返す。
     * @param x スカラー値。
     * @return 演算後の FPS。
     * @par Complexity
     * O(N) 時間、O(N + 1) 出力空間。
     */
    FPS operator-(const Mint& x) const { return FPS(*this) -= x; }
    /**
     * @brief 全係数に x を乗算したコピーを返す。
     * @param x スカラー値。
     * @return 演算後の FPS。
     * @par Complexity
     * O(N) 時間、O(N + 1) 出力空間。
     */
    FPS operator*(const Mint& x) const { return FPS(*this) *= x; }
    /**
     * @brief 全係数を x で除算したコピーを返す。
     * @param x スカラー値。
     * @return 演算後の FPS。
     * @par Complexity
     * O(N) 時間と逆元計算 1 回、O(N) 出力空間。
     * @pre x は非零で可逆。
     */
    FPS operator/(const Mint& x) const { return FPS(*this) /= x; }

    /**
     * @brief 多項式和を返す。
     * @param rhs 右辺の多項式。
     * @return 演算結果。
     * @par Complexity
     * 両辺の最大項数を N として O(N) 時間、O(N) 出力・追加空間。
     */
    FPS operator+(const FPS& rhs) const { return FPS(*this) += rhs; }
    /**
     * @brief 多項式差を返す。
     * @param rhs 右辺の多項式。
     * @return 演算結果。
     * @par Complexity
     * 両辺の最大項数を N として O(N) 時間、O(N) 出力・追加空間。
     */
    FPS operator-(const FPS& rhs) const { return FPS(*this) -= rhs; }
    /**
     * @brief 多項式積（全項の畳み込み）を返す。
     * @param rhs 右辺の多項式。
     * @return 演算結果。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 出力・追加空間。
     */
    FPS operator*(const FPS& rhs) const { return FPS(*this) *= rhs; }
    /**
     * @brief 多項式除算の商を返す。
     * @param rhs 右辺の多項式。
     * @return 演算結果。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 出力・追加空間。
     * @pre rhs は空でなく、最高次係数が非零かつ可逆。
     * @note FPS の逆数ではなく通常の多項式除算。逆数は inv(f, deg) を用いる。
     */
    FPS operator/(const FPS& rhs) const { return FPS(*this) /= rhs; }
    /**
     * @brief 多項式除算の剰余を返す。
     * @param rhs 右辺の多項式。
     * @return 演算結果。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 出力・追加空間。
     * @pre rhs は空でなく、最高次係数が非零かつ可逆。
     * @note FPS の逆数ではなく通常の多項式除算。逆数は inv(f, deg) を用いる。
     */
    FPS operator%(const FPS& rhs) const { return FPS(*this) %= rhs; }

    /**
     * @brief x^k を掛けたコピーを返す。
     * @param k シフトする非負の項数。
     * @return シフト後の FPS。
     * @pre k >= 0。
     * @par Complexity
     * O(N + k) 時間・出力空間。
     */
    FPS operator<<(int k) const { return FPS(*this) <<= k; }
    /**
     * @brief 低次 k 項を捨てたコピーを返す。
     * @param k シフトする非負の項数。
     * @return シフト後の FPS。
     * @pre k >= 0。
     * @par Complexity
     * O(N) 時間・出力空間。
     */
    FPS operator>>(int k) const { return FPS(*this) >>= k; }

    /**
     * @brief 定数項に x を加算する。
     * @param x スカラー値。
     * @return 自身への参照。
     * @par Complexity
     * O(1)。
     * @note 空列には定数項を一つ追加する。高次の零は削除しない。
     */
    FPS& operator+=(const Mint& x) {
        if (this->empty()) this->resize(1);
        (*this)[0] += x;
        return *this;
    }

    /**
     * @brief 定数項から x を減算する。
     * @param x スカラー値。
     * @return 自身への参照。
     * @par Complexity
     * O(1)。
     * @note 空列には定数項を一つ追加する。高次の零は削除しない。
     */
    FPS& operator-=(const Mint& x) {
        if (this->empty()) this->resize(1);
        (*this)[0] -= x;
        return *this;
    }

    /**
     * @brief 多項式和で自身を更新する。
     * @param rhs 右辺の多項式。
     * @return 自身への参照。
     * @par Complexity
     * 両辺の最大項数を N として O(N) 時間、O(N) 追加空間。
     * @note 高次の零係数を削除する。
     */
    FPS& operator+=(const FPS& rhs) {
        if (rhs.size() > this->size()) this->resize(rhs.size());
        for (int i = 0; i < static_cast<int>(rhs.size()); ++i) {
            (*this)[i] += rhs[i];
        }
        return normalize();
    }

    /**
     * @brief 多項式差で自身を更新する。
     * @param rhs 右辺の多項式。
     * @return 自身への参照。
     * @par Complexity
     * 両辺の最大項数を N として O(N) 時間、O(N) 追加空間。
     * @note 高次の零係数を削除する。
     */
    FPS& operator-=(const FPS& rhs) {
        if (rhs.size() > this->size()) this->resize(rhs.size());
        for (int i = 0; i < static_cast<int>(rhs.size()); ++i) {
            (*this)[i] -= rhs[i];
        }
        return normalize();
    }

    /**
     * @brief 全係数に x を乗算する。
     * @param x スカラー値。
     * @return 自身への参照。
     * @par Complexity
     * O(N)。
     */
    FPS& operator*=(const Mint& x) {
        const Mint factor = x;
        for (Mint& a : *this) a *= factor;
        return *this;
    }

    /**
     * @brief 全係数を x で除算する。
     * @param x スカラー値。
     * @return 自身への参照。
     * @par Complexity
     * O(N) と逆元計算 1 回。
     * @pre x は非零で可逆。
     */
    FPS& operator/=(const Mint& x) {
        assert(x != Mint(0));
        const Mint inv_x = x.inv();
        for (Mint& a : *this) a *= inv_x;
        return *this;
    }

    /**
     * @brief 多項式積（全項の畳み込み）で自身を更新する。
     * @param rhs 右辺の多項式。
     * @return 自身への参照。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 追加空間。
     * @note 両辺が非空なら項数は size() + rhs.size() - 1。高次の零は削除しない。
     */
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

    /**
     * @brief 多項式を x で評価する（Horner 法）。
     * @param x 代入する係数型の値。
     * @return f(x)。空列なら 0。
     * @par Complexity
     * O(N) 時間、追加空間 O(1)。
     */
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
     *
     * @pre rhs の最高次係数は可逆であること。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 追加空間。
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
     *
     * @pre rhs の最高次係数は可逆であること。
     * @par Complexity
     * 両辺の最大項数を N として O(N log N) 時間、O(N) 追加空間。
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

    /**
     * @brief inv(f, f.size()) と同じ。
     * @param f 対象の FPS。
     * @return f.size() 項で打ち切った結果。空列には空列を返す。
     * @pre 項数を明示する inv と同じ係数・逆元の条件。
     * @par Complexity
     * N = f.size() として O(N log N)。
     */
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

    /**
     * @brief log(f, f.size()) と同じ。
     * @param f 対象の FPS。
     * @return f.size() 項で打ち切った結果。空列には空列を返す。
     * @pre 項数を明示する log と同じ係数・逆元の条件。
     * @par Complexity
     * N = f.size() として O(N log N)。
     */
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

    /**
     * @brief exp(f, f.size()) と同じ。
     * @param f 対象の FPS。
     * @return f.size() 項で打ち切った結果。空列には空列を返す。
     * @pre 項数を明示する exp と同じ係数・逆元の条件。
     * @par Complexity
     * N = f.size() として O(N log N)。
     */
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
     *
     * @note k == 0 は零多項式にも定数項 1 の deg 項を返す。deg == 0 は空列。
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

    /**
     * @brief pow(f, k, f.size()) と同じ。
     * @param f 累乗する FPS。
     * @param k 非負の指数。
     * @return f.size() 項で打ち切った累乗。
     * @pre 項数を明示する pow と同じ条件。
     * @par Complexity
     * O(N log N + log(k + 1))。
     */
    friend FPS pow(const FPS& f, long long k) {
        return pow(f, k, static_cast<int>(f.size()));
    }

    /**
     * @brief 多項式の最大公約多項式を Euclid 法で求める。
     * @param a 一方の多項式（値渡しで元の列は変更しない）。
     * @param b 他方の多項式。
     * @return 最高次係数を 1 に正規化した GCD。両辺が零なら空列。
     * @pre 除算で用いる非零係数は可逆。
     * @par Complexity
     * 最大項数 N に対して O(N^2 log N) 時間の上界、O(N) 追加空間。
     */
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
