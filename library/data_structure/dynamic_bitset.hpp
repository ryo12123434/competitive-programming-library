#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief 任意長の動的ビット列。文字列の先頭がbit0で、std::bitsetの文字列規約とは逆。
 * @note bit演算の両辺は同じ長さ。長さ不一致はinvalid_argument。末尾の未使用bitは常に0。
 * find_next(i)はiより「後」の最小の立ったbit、見つからなければnullopt。
 * @par Complexity W=ceil(N/64)。構築・bit演算・シフト・count・検索は最悪O(W+1)、単一bit操作O(1)。
 * 保持空間O(W)、文字列変換O(N)時間・出力空間。シフトはin-placeで追加空間O(1)。
 * @code
 * #include "library/data_structure/dynamic_bitset.hpp"
 * #include <cassert>
 * int main(){DynamicBitset b(65);b.set(0).set(64);assert(b.count()==2&&b.find_next(0)==64);b<<=1;assert(b.count()==1&&b.test(1));assert((~b).count()==64);}
 * @endcode
 */
class DynamicBitset {
    using U = std::uint64_t;
    std::size_t n_;
    std::vector<U> words_;
    void trim() {
        if (n_ % 64)
            words_.back() &= (U(1) << (n_ % 64)) - 1;
    }
    void compatible(const DynamicBitset &b) const {
        if (n_ != b.n_)
            throw std::invalid_argument("DynamicBitset: different lengths");
    }
    std::optional<std::size_t> find_from(std::size_t p) const {
        if (p >= n_)
            return std::nullopt;
        std::size_t i = p / 64;
        U v = words_[i] & (~U(0) << (p % 64));
        while (true) {
            if (v)
                return i * 64 + std::countr_zero(v);
            if (++i == words_.size())
                return std::nullopt;
            v = words_[i];
        }
    }

public:
    /// @brief nビットをvalueで初期化する。n=0も許す。O(ceil(n/64))時間・空間。
    explicit DynamicBitset(std::size_t n = 0, bool value = false)
        : n_(n), words_(n / 64 + (n % 64 != 0), value ? ~U(0) : 0) {
        trim();
    }
    /// @brief 先頭をbit0とする文字列から構築。0/1以外はinvalid_argument。
    explicit DynamicBitset(std::string_view bits) : DynamicBitset(bits.size()) {
        for (std::size_t i = 0; i < n_; ++i) {
            if (bits[i] != '0' && bits[i] != '1')
                throw std::invalid_argument("DynamicBitset: invalid digit");
            if (bits[i] == '1')
                set(i);
        }
    }
    /// @brief ビット数。立っているビットの個数ではない。O(1)。
    std::size_t size() const { return n_; }
    /// @brief bit pの値。@pre p<size()。
    bool test(std::size_t p) const {
        assert(p < n_);
        return words_[p / 64] >> (p % 64) & 1;
    }
    /// @brief bit pをvalueに変更して自身を返す。@pre p<size()。
    DynamicBitset &set(std::size_t p, bool value = true) {
        assert(p < n_);
        if (value)
            words_[p / 64] |= U(1) << (p % 64);
        else
            words_[p / 64] &= ~(U(1) << (p % 64));
        return *this;
    }
    /// @brief bit pを0にする。@pre p<size()。
    DynamicBitset &reset(std::size_t p) { return set(p, false); }
    /// @brief 全ビットを0にして自身を返す。
    DynamicBitset &reset() {
        std::fill(words_.begin(), words_.end(), 0);
        return *this;
    }
    /// @brief bit pを反転する。@pre p<size()。
    DynamicBitset &flip(std::size_t p) {
        assert(p < n_);
        words_[p / 64] ^= U(1) << (p % 64);
        return *this;
    }
    /// @brief 全ビットを反転し、末尾の余分なビットを除く。
    DynamicBitset &flip() {
        for (auto &x : words_)
            x = ~x;
        trim();
        return *this;
    }
    DynamicBitset &operator&=(const DynamicBitset &b) {
        compatible(b);
        for (std::size_t i = 0; i < words_.size(); ++i)
            words_[i] &= b.words_[i];
        return *this;
    }
    DynamicBitset &operator|=(const DynamicBitset &b) {
        compatible(b);
        for (std::size_t i = 0; i < words_.size(); ++i)
            words_[i] |= b.words_[i];
        return *this;
    }
    DynamicBitset &operator^=(const DynamicBitset &b) {
        compatible(b);
        for (std::size_t i = 0; i < words_.size(); ++i)
            words_[i] ^= b.words_[i];
        return *this;
    }
    friend DynamicBitset operator&(DynamicBitset a, const DynamicBitset &b) {
        a &= b;
        return a;
    }
    friend DynamicBitset operator|(DynamicBitset a, const DynamicBitset &b) {
        a |= b;
        return a;
    }
    friend DynamicBitset operator^(DynamicBitset a, const DynamicBitset &b) {
        a ^= b;
        return a;
    }
    DynamicBitset operator~() const {
        auto copy = *this;
        copy.flip();
        return copy;
    }
    DynamicBitset &operator<<=(std::size_t k) {
        if (k >= n_)
            return reset();
        std::size_t shift = k / 64;
        unsigned bits = k % 64;
        for (std::size_t i = words_.size(); i-- > 0;) {
            U value = 0;
            if (i >= shift) {
                value = words_[i - shift] << bits;
                if (bits && i > shift)
                    value |= words_[i - shift - 1] >> (64 - bits);
            }
            words_[i] = value;
        }
        trim();
        return *this;
    }
    DynamicBitset &operator>>=(std::size_t k) {
        if (k >= n_)
            return reset();
        std::size_t shift = k / 64;
        unsigned bits = k % 64;
        for (std::size_t i = 0; i < words_.size(); ++i) {
            U value = 0;
            if (i + shift < words_.size()) {
                value = words_[i + shift] >> bits;
                if (bits && i + shift + 1 < words_.size())
                    value |= words_[i + shift + 1] << (64 - bits);
            }
            words_[i] = value;
        }
        trim();
        return *this;
    }
    friend DynamicBitset operator<<(DynamicBitset b, std::size_t k) {
        b <<= k;
        return b;
    }
    friend DynamicBitset operator>>(DynamicBitset b, std::size_t k) {
        b >>= k;
        return b;
    }
    /// @brief 立っているビットの個数。
    std::size_t count() const {
        std::size_t result = 0;
        for (auto x : words_)
            result += std::popcount(x);
        return result;
    }
    /// @brief 1つでも立っていればtrue。空列はfalse。
    bool any() const {
        for (auto x : words_)
            if (x)
                return true;
        return false;
    }
    /// @brief 全ビット0ならtrue。空列もtrue。
    bool none() const { return !any(); }
    /// @brief 最初の立ったビット。なければnullopt。
    std::optional<std::size_t> find_first() const { return find_from(0); }
    /// @brief pより後の最初の立ったビット。pが範囲外・なければnullopt。
    std::optional<std::size_t> find_next(std::size_t p) const {
        if (n_ == 0 || p >= n_ - 1)
            return std::nullopt;
        return find_from(p + 1);
    }
    /// @brief bit0から添字順に0/1を並べる。std::bitsetの出力順とは逆。
    std::string to_string() const {
        std::string s(n_, '0');
        for (std::size_t i = 0; i < n_; ++i)
            if (test(i))
                s[i] = '1';
        return s;
    }
    bool operator==(const DynamicBitset &) const = default;
};
