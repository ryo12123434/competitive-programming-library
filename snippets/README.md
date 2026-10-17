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

## 旧名からの移行

| 旧 prefix | 新 prefix |
| --- | --- |
| `atcmain` | `main`（`atcmain` も同じスニペットの別名として維持） |
| `cp-lazy-assign-min-config` | `lazy-assign-min` |
| `cp-lazy-assign-max-config` | `lazy-assign-max` |
| `cp-lazy-assign-sum-config` | `lazy-assign-sum` |
| `cp-lazy-add-min-config` | `lazy-add-min` |
| `cp-lazy-add-max-config` | `lazy-add-max` |
| `cp-lazy-add-sum-config` | `lazy-add-sum` |
| `cp-lazy-init` | `lazy-init` |
| `cp-treap-add-config` | `treap-add` |
| `cp-treap-affine-config` | `treap-affine` |
| `cp-treap-init` | `treap-init` |
| `cp-mo-sum` | `mo-sum` |
| `cp-mo-directional` | `mo-directional` |
| `cp-sparse-min-config` | `sparse-min` |
| `cp-sparse-init` | `sparse-init` |

旧 `cp-*` の別名は登録していません。既存スニペットの C++ 本文とタブストップは維持しています。

## 検証

```sh
python -m unittest discover -s .verify-helper/tests -p test_snippets.py -v
```

全ファイルの JSON、重複キー・prefix、別名、説明・scope、プレースホルダーの展開を検査します。
展開結果を GNU++23 でコンパイル・実行し、全6種類の遅延セグ木を愚直配列と比較します。
メインテンプレートは `LOCAL` 有効・無効の両方を検証します。実行には `g++` と ACL が必要です。
