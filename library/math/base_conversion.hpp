#pragma once

#include <algorithm>
#include <string>
#include <vector>

/**
 * @brief 非負整数 n を base 進数の文字列へ変換する。
 * @param n 変換する非負整数
 * @param base 基数
 * @return base 進表現の文字列
 * @pre 0 <= n、2 <= base <= 36
 * @par Complexity
 * O(1 + log_base(n+1)) 時間・出力空間。
 *
 * @note 0 は "0"。10 以上の桁は大文字 A..Z。
 *
 * @code
 * #include "library/math/base_conversion.hpp"
 * #include <cassert>
 * int main() {
 *     assert(to_base(255, 16) == "FF");
 *     assert(from_base("ff", 16) == 255);
 *     assert((to_digits(13, 2) == std::vector<int>{1, 1, 0, 1}));
 *     assert((to_digits_rev(13, 2) == std::vector<int>{1, 0, 1, 1}));
 * }
 * @endcode
 */
inline std::string to_base(long long n, int base) {
    const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (n == 0) return "0";

    std::string res;
    while (n > 0) {
        res.push_back(digits[n % base]);
        n /= base;
    }

    std::reverse(res.begin(), res.end());
    return res;
}

/**
 * @brief base 進数の文字列を整数へ変換する。
 * @param s base 進表現の文字列
 * @param base 基数
 * @return 変換後の整数
 * @pre 2 <= base <= 36、s は正しい base 進表現で、結果が long long に収まること。
 * @par Complexity
 * O(|s|)
 *
 * @note 英字は大文字・小文字とも対応。符号・空白は扱わず、入力の妥当性検査はしない。追加空間 O(1)。
 */
inline long long from_base(const std::string& s, int base) {
    long long res = 0;

    for (char c : s) {
        int d;
        if ('0' <= c && c <= '9') {
            d = c - '0';
        } else if ('A' <= c && c <= 'Z') {
            d = c - 'A' + 10;
        } else {
            d = c - 'a' + 10;
        }

        res = res * base + d;
    }

    return res;
}

/**
 * @brief n の base 進表現を上位桁から返す。
 * @param n 変換する非負整数
 * @param base 基数
 * @return 各桁を上位桁から並べた列
 * @pre 0 <= n、2 <= base
 * @note 例: to_digits(13, 2) -> {1, 1, 0, 1}
 * @par Complexity
 * O(1 + log_base(n+1)) 時間・出力空間。
 *
 * @note 0 は {0}。各桁は [0, base)。
 */
inline std::vector<int> to_digits(long long n, int base) {
    if (n == 0) return {0};

    std::vector<int> res;
    while (n > 0) {
        res.push_back(n % base);
        n /= base;
    }

    std::reverse(res.begin(), res.end());
    return res;
}

/**
 * @brief n の base 進表現を下位桁から返す。
 * @param n 変換する非負整数
 * @param base 基数
 * @return 各桁を下位桁から並べた列
 * @pre 0 <= n、2 <= base
 * @note 例: to_digits_rev(13, 2) -> {1, 0, 1, 1}
 * @par Complexity
 * O(1 + log_base(n+1)) 時間・出力空間。
 *
 * @note 0 は {0}。各桁は [0, base)。
 */
inline std::vector<int> to_digits_rev(long long n, int base) {
    if (n == 0) return {0};

    std::vector<int> res;
    while (n > 0) {
        res.push_back(n % base);
        n /= base;
    }

    return res;
}
