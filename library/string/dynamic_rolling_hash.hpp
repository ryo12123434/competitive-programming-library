#pragma once
#include "rolling_hash.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

/// @brief 区間hashと長さ・基数。異なる基数の値は同一文字列でも比較できない。
struct DynamicHashValue {
    std::uint64_t hash = 0;
    int length = 0;
    std::uint64_t base = 0;
    bool operator==(const DynamicHashValue &) const = default;
};
/**
 * @brief 長さ固定の1バイト変更・区間hash取得。既存RollingHashの法2^61-1と安全な積を再利用する。
 * @note 基数は各個体が保持する。既定は既存RollingHashと共有して選び、明示指定で再現も可能。
 * ハッシュ一致は同一性の証明ではない。異なる固定2列の長さLと独立な一様基数なら衝突確率は高々(L-1)/(mod-257)。
 * 固定基数・適応的入力にはこの確率保証はない。厳密判定が必要な用途には使わない。
 * @par Complexity 構築O(N)時間・空間、set/getはO(log(N+1))、connectはO(1)。
 * @code
 * #include "library/string/dynamic_rolling_hash.hpp"
 * #include <cassert>
 * int main(){DynamicRollingHash h("ababa",911382323);assert(h.get(0,3)==h.get(2,5));h.set(0,'x');assert(h.get(0,3)!=h.get(2,5));assert(h.connect(h.get(0,2),h.get(2,5))==h.get(0,5));}
 * @endcode
 */
class DynamicRollingHash {
    struct Node {
        std::uint64_t hash = 0;
        int length = 0;
    };
    int n_;
    std::size_t size_ = 1;
    std::uint64_t base_;
    std::vector<std::uint64_t> power_;
    std::vector<Node> tree_;
    Node combine(Node a, Node b) const {
        auto hash = RollingHash::mul(a.hash, power_[b.length]) + b.hash;
        if (hash >= RollingHash::mod)
            hash -= RollingHash::mod;
        return {hash, a.length + b.length};
    }
    static std::uint64_t default_base() {
        if (!RollingHash::base)
            RollingHash::base = RollingHash::generate_base();
        return RollingHash::base;
    }

public:
    /// @brief 文字列を構築。@pre 長さはintに収まる。基数は[256,mod-2]、違反はinvalid_argument。
    explicit DynamicRollingHash(std::string_view text, std::uint64_t base = default_base()) : base_(base) {
        if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("DynamicRollingHash: too long");
        if (base < 256 || base >= RollingHash::mod - 1)
            throw std::invalid_argument("DynamicRollingHash: invalid base");
        n_ = static_cast<int>(text.size());
        power_.resize(text.size() + 1, 1);
        for (int i = 0; i < n_; ++i)
            power_[i + 1] = RollingHash::mul(power_[i], base_);
        while (size_ < text.size())
            size_ *= 2;
        tree_.resize(2 * size_);
        for (int i = 0; i < n_; ++i)
            tree_[size_ + i] = {static_cast<unsigned char>(text[i]), 1};
        for (std::size_t p = size_ - 1; p; --p)
            tree_[p] = combine(tree_[2 * p], tree_[2 * p + 1]);
    }
    int size() const { return n_; }
    std::uint64_t base() const { return base_; }
    /// @brief 位置iの1バイトを置換。@pre i∈[0,N)。長さは変えない。
    void set(int i, char c) {
        assert(0 <= i && i < n_);
        std::size_t p = size_ + i;
        tree_[p] = {static_cast<unsigned char>(c), 1};
        while (p > 1) {
            p >>= 1;
            tree_[p] = combine(tree_[2 * p], tree_[2 * p + 1]);
        }
    }
    /// @brief 半開区間[l,r)の値。空ならhash0・長さ0。@pre 0<=l<=r<=N。
    DynamicHashValue get(int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        Node a, b;
        std::size_t left = size_ + l, right = size_ + r;
        while (left < right) {
            if (left & 1)
                a = combine(a, tree_[left++]);
            if (right & 1)
                b = combine(tree_[--right], b);
            left >>= 1;
            right >>= 1;
        }
        auto result = combine(a, b);
        return {result.hash, result.length, base_};
    }
    /// @brief 同じ基数の2値を順に連結。右の長さは[0,N]、合計長はint以内。違反は例外。
    DynamicHashValue connect(DynamicHashValue a, DynamicHashValue b) const {
        if (a.base != base_ || b.base != base_ || a.hash >= RollingHash::mod || b.hash >= RollingHash::mod ||
            a.length < 0 || b.length < 0 || b.length > n_)
            throw std::invalid_argument("DynamicRollingHash: incompatible value");
        if (a.length > std::numeric_limits<int>::max() - b.length)
            throw std::length_error("DynamicRollingHash: length overflow");
        auto value = combine({a.hash, a.length}, {b.hash, b.length});
        return {value.hash, value.length, base_};
    }
};
