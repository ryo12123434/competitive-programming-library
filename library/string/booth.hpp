#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * @brief 辞書順最小の巡回シフトの開始位置を Booth 法で求める。
 * @tparam Sequence size(), operator[]、要素の == と狭義全順序 < を持つ列。
 * @param s 入力列。空なら 0。同じ最小表現が複数あるなら最小の開始位置。
 * @pre 長さは INT_MAX 以下。要素の比較は一貫した全順序に従うこと。
 * @note char は std::string の辞書順と同じ unsigned char の順で比較する。
 * @return 0-indexed の位置。空でなければ [0,s.size())。
 * @par Complexity
 * 比較 O(1) なら O(N) 時間、追加空間 O(1)。
 * @code
 * #include "library/string/booth.hpp"
 * #include <cassert>
 * int main() { assert(booth(std::string("baba"))==1); }
 * @endcode
 */
template <class Sequence>
int booth(const Sequence& s) {
    assert(s.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    std::size_t n = s.size(), i = 0, j = 1, k = 0;
    if (!n) return 0;
    while (i < n && j < n && k < n) {
        const auto& a = s[(i + k) % n]; const auto& b = s[(j + k) % n];
        if (a == b) { ++k; continue; }
        const bool less = [&]() {
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(a)>, char>)
                return static_cast<unsigned char>(b) < static_cast<unsigned char>(a);
            else return b < a;
        }();
        if (less) { i += k + 1; if (i == j) ++i; }
        else { j += k + 1; if (i == j) ++j; }
        k = 0;
    }
    return static_cast<int>(std::min(i, j));
}

/**
 * @brief 最小巡回表現の文字列を返す。空入力なら空文字列。
 * @param s 入力文字列。文字の順序は unsigned char の比較に従う。
 * @par Complexity
 * O(N) 時間・返り値の空間。
 * @code
 * #include "library/string/booth.hpp"
 * #include <cassert>
 * int main() { assert(minimal_rotation("baca")=="abac"); }
 * @endcode
 */
inline std::string minimal_rotation(std::string_view s) {
    auto p = static_cast<std::size_t>(booth(s));
    return std::string(s.substr(p)) + std::string(s.substr(0, p));
}
