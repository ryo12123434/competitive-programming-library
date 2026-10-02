#pragma once

#include <cassert>
#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

/**
 * @brief 法を 2^61 - 1 とする文字列の Rolling Hash。全インスタンスで乱数の基数を共有する。
 * @note ハッシュの一致には衝突の可能性がある。長さも比較すること。構築後に base を変更しないこと。
 * @note base を inline static にして、複数の翻訳単位から読み込めるようにしている。
 */
struct RollingHash {
    static constexpr std::uint64_t mod = (1ULL << 61) - 1;
    using uint128_t = __uint128_t;
    std::vector<std::uint64_t> hash, power;

    /// @brief [256, mod - 2] の範囲から基数を生成する。
    static std::uint64_t generate_base() {
        std::mt19937_64 mt(std::chrono::steady_clock::now().time_since_epoch().count());
        return std::uniform_int_distribution<std::uint64_t>(256, mod - 2)(mt);
    }
    inline static std::uint64_t base = 0;

    /// @brief a*b を法 mod で O(1) で計算する。@pre 0 <= a, b < mod。
    static std::uint64_t mul(uint128_t a, uint128_t b) {
        uint128_t t = a * b;
        t = (t >> 61) + (t & mod);
        if (t >= mod) t -= mod;
        return static_cast<std::uint64_t>(t);
    }

    /// @brief 累積ハッシュと基数の累乗を O(|s|) 時間・空間で構築する。
    explicit RollingHash(const std::string& s) : hash(s.size() + 1, 0), power(s.size() + 1, 1) {
        if (base == 0) base = generate_base();
        for (std::size_t i = 0; i < s.size(); ++i) {
            hash[i + 1] = mul(hash[i], base) + static_cast<unsigned char>(s[i]);
            if (hash[i + 1] >= mod) hash[i + 1] -= mod;
            power[i + 1] = mul(power[i], base);
        }
    }

    /// @brief [l, r) のハッシュを O(1) で返す。@pre 0 <= l <= r <= |s|。
    std::uint64_t get(int l, int r) const {
        assert(0 <= l && l <= r && static_cast<std::size_t>(r) < hash.size());
        std::uint64_t result = hash[r] + mod - mul(hash[l], power[r - l]);
        if (result >= mod) result -= mod;
        return result;
    }

    /// @brief h1 の後ろに h2 を連結したハッシュを O(1) で返す。
    /// @pre ハッシュは mod 未満。p は h2_len までの base の累乗を格納すること。
    static std::uint64_t connect(std::uint64_t h1, std::uint64_t h2, int h2_len,
                                 const std::vector<std::uint64_t>& p) {
        assert(h2_len >= 0 && static_cast<std::size_t>(h2_len) < p.size());
        std::uint64_t result = mul(h1, p[h2_len]) + h2;
        if (result >= mod) result -= mod;
        return result;
    }
};
