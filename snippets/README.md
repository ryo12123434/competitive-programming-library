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

Windows の PowerShell で、このリポジトリのルートから全5ファイルを導入する例です。

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
ラムダは既定値・型や引数の変更・引数なし・再帰を検証し、グリッドは空・境界内外・変数名変更を検証します。
メインテンプレートは bundle 前後とも `LOCAL` 有効・無効を検証し、提出版はリポジトリの include パスなしでもコンパイルします。
実行には `g++` と ACL が必要です。
