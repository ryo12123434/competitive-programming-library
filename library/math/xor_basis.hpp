#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>

/**
 * @brief GF(2) 上の XOR 線形基底。元集合の個数ではなく独立なベクトルの rank を管理する。
 * @tparam UInt uint32_t / uint64_t 等の64bit以下の符号なし整数（bool は不可）。大小も符号なし順。
 * @par Complexity
 * B=型のビット数。保持空間 O(B)、insert / can_represent / max_xor / min_xor O(B)、merge O(B^2)。
 * @code
 * #include "library/math/xor_basis.hpp"
 * #include <cassert>
 * int main() {
 *     XorBasis<> basis;
 *     assert(basis.insert(3) && basis.insert(5) && !basis.insert(6));
 *     assert(basis.rank() == 2 && basis.can_represent(6));
 *     assert(basis.max_xor() == 6 && basis.min_xor(7) == 1);
 *     assert(basis.distinct_count() == 4);
 * }
 * @endcode
 */
template<class UInt = std::uint64_t>
class XorBasis {
    static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> &&
                  !std::is_same_v<UInt, bool> && sizeof(UInt) <= 8);
    static constexpr int bits = std::numeric_limits<UInt>::digits;
    std::array<UInt, bits> basis_{};
    int rank_ = 0;
public:
    /// @brief x を追加し、rank が増えたら true。0・従属・重複では false。
    bool insert(UInt x) {
        for (int bit = bits-1; bit >= 0; --bit) {
            if (!((x >> bit) & UInt{1})) continue;
            if (!basis_[bit]) { basis_[bit] = x; ++rank_; return true; }
            x ^= basis_[bit];
        }
        return false;
    }
    /// @brief 次元（0～B）を O(1) で返す。
    int rank() const { return rank_; }
    /// @brief 既存ベクトルの部分集合 XOR で x を表現可能か。空部分集合は0。
    bool can_represent(UInt x) const { return min_xor(x) == 0; }
    /// @brief x と任意の部分集合 XOR の最大値。空基底なら x。
    UInt max_xor(UInt x = 0) const {
        for (int bit = bits-1; bit >= 0; --bit) if ((x ^ basis_[bit]) > x) x ^= basis_[bit];
        return x;
    }
    /// @brief x と任意の部分集合 XOR の最小値。空基底なら x。
    UInt min_xor(UInt x = 0) const {
        for (int bit = bits-1; bit >= 0; --bit) if ((x ^ basis_[bit]) < x) x ^= basis_[bit];
        return x;
    }
    /// @brief 空基底へ戻す。O(B)。
    void clear() { basis_.fill(0); rank_ = 0; }
    /// @brief other の張る空間を追加する。自分自身の指定も可。O(B^2)。
    void merge(const XorBasis& other) { for (UInt x : other.basis_) insert(x); }
    /// @brief 異なる部分集合 XOR 値の数 2^rank。rank==64 も表せる GNU unsigned __int128。O(1)。
    unsigned __int128 distinct_count() const { return static_cast<unsigned __int128>(1) << rank_; }
};
