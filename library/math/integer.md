# 整数処理

GNU++23 / C++20 の GCC・Clang 向けです。公開APIはグローバル名前空間にあり、
使うヘッダだけ include します。`bool`・浮動小数点型・`cpp_int` は整数演算テンプレートの対象外です。

| ヘッダ | API | 仕様・計算量 |
| --- | --- | --- |
| `int128_io.hpp` | `to_string_int128` / `to_string_uint128` | 全128bit域を10進文字列へ。D桁に O(D) 時間・空間 |
| 同上 | `parse_int128` / `parse_uint128` | 形式不正・範囲外は `nullopt`。L文字に O(L) 時間、O(1) 追加空間 |
| 同上 | `read_int128(in,x)` / `read_uint128(in,x)` | 空白区切りの1トークン。失敗時は x を変えず `failbit`。O(L) 時間・一時空間 |
| 同上 | `print_int128(out,x)` / `print_uint128(out,x)` | 10進出力。改行なし。ストリーム参照を返す。O(D) 時間・一時空間 |
| `integer_division.hpp` | `floor_div(a,b)` / `ceil_div(a,b)` | 同じ整数型の床・天井除算。0除算・最小値/-1は `nullopt`。O(1) |
| `integer_sqrt.hpp` | `isqrt(x)` | 非負整数の平方根の床。同じ型で返す。B bitに O(B) 時間、O(1) 空間 |
| `safe_arithmetic.hpp` | `wide_mul(a,b)` / `compare_products(a,b,c,d)` | 64bit以下の同一型の積を128bitで計算・比較。O(1) |
| 同上 | `checked_add(a,b)` / `checked_mul(a,b)` | 同一整数型で範囲内なら値、範囲外なら `nullopt`。128bitも対応、O(1) |
| `integer_digits.hpp` | `digit_count(x)` / `digit_sum(x)` | 絶対値の10進桁数・桁和。最小負数も対応。0は1桁・桁和0。O(D) 時間、O(1) 空間 |

`parse` は ASCII の数字だけを許し、前後や途中の空白は許しません。先頭 `+` と先頭0は許します。
符号付き版は `-0` も許し、符号なし版は `-0` を含む負号を拒否します。
入出力に独自の `operator<<` / `operator>>` は追加しません。出力は `hex` / `showpos` にかかわらず10進で、
`width` / `fill` / 左右寄せは文字列と同じ規則です。

演算テンプレートは標準整数と GNU `__int128_t` / `__uint128_t` を扱います。
型の混在は推論時に拒否するため、必要なら `floor_div<long long>(a,b)` のように明示します。
その明示変換自体が安全かは呼び出し側で確認してください。`isqrt` に負値を渡すと事前条件違反です。
128bit同士の積が128bitに収まる保証はありません。`checked_mul` で検出するか、`cpp_int` を使います。

## Boost の多倍長整数

型エイリアスだけのヘッダは設けず、Boostの既存機能を直接使います。
必要なときだけ `<boost/multiprecision/cpp_int.hpp>` を include してください。
メインテンプレートや整数ユーティリティからは Boost に依存しません。

```cpp
#include <boost/multiprecision/cpp_int.hpp>
#include "library/math/int128_io.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

int main() {
    using boost::multiprecision::cpp_int;
    std::istringstream in("-123456789012345678901234567890");
    cpp_int value;
    in >> value; // std::cin >> value; / std::cout << value; も使用できる
    std::ostringstream out;
    out << value;
    std::string text = value.str(); // 符号付き10進文字列
    auto digits = text.size() - (text.front() == '-');
    assert(digits == 30 && out.str() == text);

    __int128_t small = -123;
    cpp_int big(to_string_int128(small)); // 正規化済み10進文字列を渡す
    auto back = parse_int128(big.str());  // 範囲外はnullopt
    assert(back && *back == small);
    big <<= 200;
    assert(!parse_int128(big.str()));

    __uint128_t maximum = ~__uint128_t(0);
    cpp_int unsigned_big(to_string_uint128(maximum));
    assert(parse_uint128(unsigned_big.str()) == maximum);
}
```

`cpp_int` の演算・文字列変換の時間とメモリは値の桁数に依存します。
利用環境に Boost のヘッダが必要です。128bitとの変換例は範囲検査を明示するため文字列を経由します。
`cpp_int` に任意の先頭0付き文字列を直接渡すと基数が自動判定されるため、10進として扱う入力とは区別してください。

`debug(...)` / `cpl::debug::repr(...)` は128bit整数と、それを含むコンテナを表示できます。
`cpp_int` は既存のストリーム表示を利用します。debugヘッダから Boost を include することはありません。

スニペットは `snippets/integer.code-snippets` です。`bigint` はグローバルへ、
その他は関数内へ挿入し、先頭コメントの include をグローバルへ追加します。
`main` の `i128` / `u128` は型エイリアスのみで、直接 `cin >> x` / `cout << x` を有効にするものではありません。

## 検証

Boost のヘッダを用意し、リポジトリのルートで実行します。

```sh
python -m unittest discover -s .verify-helper/tests -p test_integer.py -v
```

Boost が見つからない環境では、参照比較・Boost使用例のテストを明示的にスキップします。
全件を検証する場合はスキップがないことを確認してください。整数処理の本体に Boost は不要です。
