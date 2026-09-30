#pragma once

#include <algorithm>
#include <string>
#include <vector>

// 0 <= n を base 進数の文字列に変換する。
// 2 <= base <= 36
// 計算量: O(log_base(n))
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

// base 進数の文字列を 10 進整数に変換する。
// 2 <= base <= 36
// 前提: s は正しい base 進表現で、結果が long long に収まる。
// 計算量: O(|s|)
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

// 0 <= n の base 進表現を上位桁から vector<int> で返す。
// 例: to_digits(13, 2) -> {1, 1, 0, 1}
// 2 <= base
// 計算量: O(log_base(n))
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

// 0 <= n の base 進表現を下位桁から vector<int> で返す。
// 例: to_digits_rev(13, 2) -> {1, 0, 1, 1}
// 2 <= base
// 計算量: O(log_base(n))
inline std::vector<int> to_digits_rev(long long n, int base) {
    if (n == 0) return {0};

    std::vector<int> res;
    while (n > 0) {
        res.push_back(n % base);
        n /= base;
    }

    return res;
}
