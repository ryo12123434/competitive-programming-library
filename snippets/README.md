# VSCode スニペット・早見表

C++ ファイルで prefix を入力し、補完候補を選んで展開します。候補が出ない場合は `Ctrl+Space`、
またはコマンドパレットの「Snippets: Insert Snippet」を使います。`Tab` で編集項目を順に移動し、
最後に `$0` の位置へ移ります。GNU++23 用です。

## 一覧

| ファイル | prefix | 挿入場所・用途 |
| --- | --- | --- |
| `template.code-snippets` | `main` / `atcmain` | 空のファイルにメインテンプレート全体 |
| `lazy-segtree.code-snippets` | `lazy-assign-min` / `lazy-assign-max` / `lazy-assign-sum` | グローバル：ACL の区間代入設定 |
| 同上 | `lazy-add-min` / `lazy-add-max` / `lazy-add-sum` | グローバル：ACL の区間加算設定 |
| 同上 | `lazy-init` | 関数内：葉の長さを 1 として初期化 |
| 同上 | `lazy-help` | コメント：更新・取得・型・計算量 |
| `implicit-treap.code-snippets` | `treap-add` / `treap-affine` | グローバル：区間加算／アフィン変換・区間和 |
| 同上 | `treap-init` | 関数内：初期列から構築 |
| 同上 | `treap-help` | コメント：挿入・削除・反転・rotate・作用の条件 |
| `mo.code-snippets` | `mo-sum` / `mo-directional` | 関数内：区間和／左右別コールバックの枠 |
| 同上 | `mo-help` | コメント：登録・実行・状態の初期化・計算量 |
| `sparse-table.code-snippets` | `sparse-min` | グローバル：最小値用の演算・型 |
| 同上 | `sparse-init` | 関数内：初期配列から構築・非空区間の取得 |
| 同上 | `sparse-help` | コメント：演算の条件・区間規約・計算量 |

`*-help` は実行処理を追加しないコメントの早見表です。関数内・グローバルのどちらにも挿入できます。
生成される変数名はプレースホルダーで変更できますが、早見表内の `tree` / `seg` などは代表名です。
ホバーによる引数の説明は、ヘッダの Doxygen コメントと C/C++ 拡張の IntelliSense を使います。

## よく使う手順・注意点

- **遅延セグ木**：設定をグローバルへ挿入し、関数内で `n` を用意して `lazy-init` を展開します。
  初期化側の名前空間・型名を設定とそろえます。例えば `lazy-add-sum` なら
  `lazy_add_sum` / `LazyAddSum`。葉は `{値, 1}`、取得値は `.mn` / `.mx` / `.sum` です。
  代入の操作なしは `std::nullopt` であり、値 `0` の代入は有効な更新です。
- **Treap**：`treap-add` または `treap-affine` → `treap-init`。
  アフィン変換の場合、初期列の要素型を `treap_affine::S` に変更します。
  写像 `{a, b}` は `x → a*x+b`。`composition(f,g)` は `g` の後に `f` です。
  `tree[p]` は値を返すため、書き換えは `set(p,x)` を使います。
- **Mo**：`library/algorithm/mo.hpp` を include し、配列 `a` と `[l,r)` の `queries` を用意します。
  `mo-directional` のコールバックは用途に合わせて実装します。
  `run` ごとに集計状態を空区間に戻し、答えは登録順の `query_id` へ保存します。
- **Sparse Table**：`sparse-min` → 初期配列 `a` → `sparse-init`。
  空区間の取得は不可。演算は結合的かつ冪等である必要があり、区間和には使えません。

添字は 0-indexed、区間は `[l,r)` です。整数の演算では中間値を含むオーバーフローに注意してください。
同じ設定を複数回挿入する場合は、名前空間と型名を変えて定義の衝突を避けます。
