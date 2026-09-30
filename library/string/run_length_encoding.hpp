#pragma once

#include <string>
#include <utility>
#include <vector>

// 文字列を連続する同一文字ごとに (文字, 個数) へ圧縮する。
// 計算量: O(|s|)
// 空文字列に対しては空の vector を返す。
inline std::vector<std::pair<char, int>> RLE(const std::string& s) {
    std::vector<std::pair<char, int>> res;
    if (s.empty()) return res;

    char c = s[0];
    int cnt = 1;

    for (int i = 1; i < static_cast<int>(s.size()); ++i) {
        if (s[i] == c) {
            ++cnt;
        } else {
            res.push_back({c, cnt});
            c = s[i];
            cnt = 1;
        }
    }

    res.push_back({c, cnt});
    return res;
}
