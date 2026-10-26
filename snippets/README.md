# VSCode スニペット・早見表

C++ ファイルで prefix を入力し、補完候補を選んで展開します。候補が出ない場合は `Ctrl+Space`、
またはコマンドパレットの「Snippets: Insert Snippet」を使います。`Tab` で編集項目を順に移動し、
最後に `$0` の位置へ移ります。すべて `scope: "cpp"`、GNU++23 用です。

## 導入

`snippets/` に置くだけでは VSCode は自動で読み込みません。次のどちらかに
必要な `.code-snippets` ファイルをコピーしてください。Markdown ファイルはコピー不要です。

- **すべての作業フォルダで使う**：ユーザースニペットのフォルダへコピーする。
  Windows は `%APPDATA%\Code\User\snippets`、Linux は `~/.config/Code/User/snippets`、
  macOS は `~/Library/Application Support/Code/User/snippets`。
- **特定のワークスペースだけで使う**：そのフォルダの `.vscode/` にコピーする。

Windows の PowerShell で、このリポジトリのルートから直下のスニペットを導入する例です。

```powershell
$snippetDir = Join-Path $env:APPDATA 'Code\User\snippets'
New-Item -ItemType Directory -Force $snippetDir | Out-Null
Copy-Item .\snippets\*.code-snippets $snippetDir
```

別の VSCode プロファイル、Insiders、リモート環境では保存先が異なる場合があります。
「Snippets: Configure Snippets」から利用中の環境の保存先を開いてください。
同じスニペットをユーザー領域と `.vscode/` の両方に置くと補完が重複します。

更新時はコピー先も上書きします。以前の `competitive-programming.code-snippets` と
`competitive-programming-lazy.code-snippets` はコピー先から削除してください。
別途 `cpp.json` などに登録していた `atcmain` も、本ファイル群と併用するなら重複登録を解除します。

スニペットは include パスや ACL をインストールしません。リポジトリ外から自作ヘッダを使う場合は、
コンパイラに `-I <このリポジトリのルート>`、IntelliSense の includePath に同じルートを設定します。
ACL のヘッダもコンパイラから見える状態にします。`main` のデバッグ用 include は `-DLOCAL` 時に有効です。

## 一覧

| ファイル | prefix | 挿入場所・用途 |
| --- | --- | --- |
| `template.code-snippets` | `main` / `atcmain` | 空のファイルにメインテンプレート全体 |
| 同上 | `lambda` | 関数内：通常のラムダ式 |
| 同上 | `lambda-rec` | 関数内：`auto&& self` を使う再帰ラムダ式 |
| 同上 | `grid-check` | ループ内：グリッドの範囲外なら `continue` |
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
| `geometry.code-snippets` | `geometry` | グローバル：必要なヘッダと `Point` の型を定義 |
| 同上 | `polar-sort` / `hull` | 関数内：偏角ソート／凸包 |
| 同上 | `segment-cross` / `ccw` / `polygon-area` | 関数内：閉線分交差／3点の向き／符号付き面積の2倍 |
| `hld.code-snippets` | `hld` | 関数内：無向木から構築・根の指定 |
| 同上 | `hld-path` / `hld-ordered` | 関数内：ACL セグ木による経路和／経路順の一次関数合成 |
| `li-chao-tree.code-snippets` | `li-chao` | 関数内：座標圧縮木の構築・直線追加・最小値取得 |
| `manacher.code-snippets` | `manacher` | 関数内：回文半径・区間回文判定 |
| `xor-basis.code-snippets` | `xor-basis` | 関数内：符号なし整数のXOR基底 |
| `binary-trie.code-snippets` | `binary-trie` | 関数内：XOR用多重集合 |
| `rerooting.code-snippets` | `rerooting` | 関数内：全頂点への距離和。辺重みをTabで編集 |
| `wavelet-matrix.code-snippets` | `wavelet` | 関数内：静的整数列の区間順位・頻度・k番目 |
| `rollback-union-find.code-snippets` | `rollback-uf` | 関数内：snapshot・merge・巻き戻し |

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

## HLD・Li Chao Tree の逆引き

HLD の値は `hld.tin(v)` の位置へ置きます。`hld-path` は和用の `seg` が構築済みであることが前提です。
辺の値は根から深い端点の位置に置き、`path_segments(u,v,true)` を使います。
部分木は `subtree_range(v)`、部分木内の辺だけなら `subtree_range(v,true)` です。
可換な和では区間の向きを無視できますが、非可換演算は正順・逆順の集約が必要です。
`hld-ordered` は [HLD の完全な ACL 設定例](../library/graph/hld.md)の `Affine`・`Node`・`combine` を使います。
単に集約済みの値を反転する処理に置き換えないでください。

`li-chao` はクエリ座標配列 `xs` を事前に用意します。返り値は `optional<__int128>` です。
`if (answer)` で存在を調べ、`*answer` で値を取得します。未登録座標へのクエリは不可です。
`long long` へ変換・出力する場合の範囲確認は呼び出し側で行います。
[初期化・数値保証・方式比較](../library/data_structure/li_chao_tree.md)を参照してください。

## 整数幾何の逆引き

共通の名前空間は `cpl::geometry`、通常の座標型は `long long` です。
まずグローバルで `geometry` を展開すると、6ヘッダと `using P = cpl::geometry::Point<long long>;` を用意できます。
個別の呼び出しスニペットには、必要な include をコメントで明示しています。
ヘッダを関数内に include せず、グローバル側に用意してください。

| やりたいこと | API | snippet / include |
| --- | --- | --- |
| 点を作る・辞書順比較 | `P p{x,y}`、`p < q` | `geometry` / `point.hpp` |
| 内積・外積・向き | `dot(a,b)`、`cross(a,b)`、`cross(a,b,c)`、`ccw(a,b,c)` | `ccw` / `point.hpp`・`ccw.hpp` |
| 距離の2乗 | `norm2(p)`、`dist2(a,b)` | `point.hpp` |
| 原点からの偏角順 | `polar_angle_sort(points)` | `polar-sort` / `polar_angle_sort.hpp` |
| 閉線分上・閉線分交差 | `on_segment(p,s)`、`segments_intersect(s,t)` | `segment-cross` / `segment.hpp` |
| 凸包 | `convex_hull(points, false)` | `hull` / `convex_hull.hpp` |
| 符号付き面積の2倍・向き | `signed_area2(polygon)`、`polygon_orientation(polygon)` | `polygon-area` / `polygon.hpp` |

関数内で呼ぶ例です。`points` は `std::vector<P>` とします。

```cpp
auto boundary = cpl::geometry::convex_hull(points, true); // 辺上の共線点も残す
cpl::geometry::Wide twice = cpl::geometry::signed_area2(boundary);
```

偏角は正の x 軸から反時計回りで、ゼロが先、同方向は短い順です。
凸包は辞書順最小点から反時計回り、始点の再掲なし。全点共線なら `false` は両端、`true` は辞書順全点です。
線分は端点接触・共線の重なり・長さ0も扱います。面積は境界順の単純多角形を想定し、正なら反時計回りです。

各座標の絶対値 `<= 2^62-1` は点演算・各種判定の十分な保証範囲です。
Point の加減算の結果は座標型に収め、面積は各外積と累積の途中も128bitに収めてください。
`Wide` を `cout` に直接渡せない点にも注意します。[数値保証の詳細](../library/geometry/README.md)を参照してください。

## main のデバッグ設定

`-DLOCAL` を付けると、`debug.hpp`・`timer.hpp`・`random.hpp` を読み込みます。
テーブル表示は `debug.hpp` に統合済みなので、互換用の `table_debug.hpp` は重ねて include しません。
`LOCAL` 未定義時の `debug(...)`・`debug_table(...)`・`CPL_SCOPED_TIMER(...)` は何も実行せず、
引数も評価しません。副作用のある式を渡す場合、提出時にはその副作用も消える点に注意してください。

`cpl::debug::Timer` や `cpl::random::Random` などの**クラスを直接使う宣言・式は自動では消えません**。
このテンプレートは `LOCAL` 未定義時にヘッダ自体を読み込まないため、提出時だけ無効にしたいコードは
次のように `#ifdef LOCAL` で囲みます。乱数生成は、ヘッダを常時 include すれば提出時にも実行されます。
Timer 自体には `LOCAL` 無効時に時計を読まず 0 を返す実装がありますが、呼び出し式の削除とは異なります。

関数内に書く例です。

```cpp
CPL_SCOPED_TIMER("solve"); // LOCAL 時にスコープ終了までの時間を表示
#ifdef LOCAL
cpl::debug::Timer timer;
cpl::random::Random rng(42);
debug(rng.integer(-10, 10), timer.elapsed_ms());
#endif
```

`bundle.py` は `#ifdef LOCAL` 内の include と条件分岐をそのまま残します。
展開後の提出コードは `LOCAL` 未定義ならデバッグ用ヘッダがなくてもコンパイルできます。
展開後も `-DLOCAL` で動かす場合は、従来どおりリポジトリの include パスが必要です。
通常の自作ライブラリは従来どおり展開します。

## 検証

```sh
python -m unittest discover -s .verify-helper/tests -p test_snippets.py -v
```

全ファイルの JSON、重複キー・prefix、別名、説明・scope、プレースホルダーの展開を検査します。
展開結果を GNU++23 でコンパイル・実行し、全6種類の遅延セグ木を愚直配列と比較します。
通常ラムダとグリッドは既定値・型や引数の変更・空や境界のケースを検証します。
再帰ラムダは現在の入力用の枠に引数・本体を補い、引数なし・再帰呼び出しも検証します。
メインテンプレートは bundle 前後とも `LOCAL` 有効・無効を検証し、提出版はリポジトリの include パスなしでもコンパイルします。
実行には `g++` と ACL が必要です。
