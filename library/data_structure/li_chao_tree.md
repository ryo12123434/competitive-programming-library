# Li Chao Tree：直線追加・最小値

`library/data_structure/li_chao_tree.hpp` の `LiChaoTree` は、座標圧縮した整数座標で
`a*x+b` の最小値を求めます。公開 API はグローバル名前空間です。

| 方式 | 初期化 | 追加・取得 | メモリ・制約 |
|---|---|---|---|
| 動的・整数範囲指定 | 座標範囲 [L,R] を指定 | O(log(R-L+1)) | 挿入でノードを確保。未知のクエリ座標にも対応できるが、全64bit範囲の中点や範囲幅の実装に注意が必要 |
| **採用：座標圧縮** | クエリ座標を列挙・ソート | O(log(M+1)) | O(M) を先に確保。クエリ座標は事前指定が必要 |

ABC でクエリを先読みできる場合の使いやすさと、メモリ・数値計算の安定性を優先して後者を採用しました。
構築は入力座標 K 個に対して O(K log(K+1)) 時間、O(K+M) ピーク空間。
保持空間は O(M)、M は重複除去後の座標数です。追加・取得は反復処理で追加空間 O(1) です。

```cpp
#include "library/data_structure/li_chao_tree.hpp"
#include <cassert>
int main() {
    LiChaoTree tree(std::vector<long long>{5, -3, 0, 5});
    assert(tree.size() == 3); // ソート・重複除去する
    assert(!tree.query(0));  // 直線未追加
    tree.add_line(2, 1);
    tree.add_line(-1, 4);
    tree.add_line(-1, 4);    // 同一直線の重複追加も可
    auto result = tree.query(5);
    assert(result && *result == -1);
}
```

- `add_line(a,b)` は全登録座標で有効な直線を追加します。同じ傾き・切片にも対応します。
- `query(x)` は登録済み座標でのみ使用可能です。未登録座標は assert 違反です。
- 直線未追加なら `std::nullopt`、それ以外は `std::optional<LiChaoTree::Value>`。
  値 0 は有効なので `if (result)` で存在を判定し、値は `*result` で取り出します。
- 空の座標配列も構築可能です。その場合 `add_line` は何もしません。有効な query 座標はありません。
- `size()` は異なる登録座標の数で、直線数ではありません。

## 数値保証と制限

a・b・x はすべて符号付き64bitの `long long` 全範囲を扱えます。
**乗算前**に `__int128` へ拡張し、返り値 `Value` も符号付き `__int128` です。
評価値の絶対値は高々 `2^126 + 2^63` であり、符号付き128bitに収まります。
比較に差の減算は使わず、中点も座標値ではなく添字 `l+(r-l)/2` で求めるため、
`LLONG_MIN` と `LLONG_MAX` を同じ木へ登録しても安全です。

INF の番兵は使いません。`long long` に変換する場合は、呼び出し側でその型の最小値・最大値と
比較してから明示的に変換してください。`Value` の標準 ostream への直接出力はできません。
GNU++23（MinGW を含む）の128bit整数対応を前提とします。MSVC 単体は対象外です。

最大値、区間限定直線追加、直線の削除・ID取得、事後の座標追加は今回の API に含めません。
最大値用に傾きを単純に符号反転すると `LLONG_MIN` でオーバーフローするため、
全64bitの保証が必要なら最大値版の比較処理を別途実装する必要があります。
