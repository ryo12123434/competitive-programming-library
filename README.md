# Competitive Programming Library

AtCoder の GNU++23 環境を想定した、C++ 競技プログラミング用ライブラリです。

## ディレクトリ構成

```text
library/
  algorithm/
  data_structure/
  graph/
  math/
  string/
verify/
  algorithm/
  data_structure/
  graph/
  math/
  string/
template/
snippets/
```

- `library/`: カテゴリ別のライブラリ本体
- `verify/`: ライブラリに対応する検証コード
- `template/`: AtCoder 提出用テンプレート
- `snippets/`: VSCode の C++ 用スニペット

## VSCode の IntelliSense とスニペット

`library/` の全ヘッダに日本語の Doxygen コメントとコンパイル可能な使用例を記載しています。
クラス・struct の説明にはインスタンス生成例、関数の説明には引数・区間の規約や特殊な戻り値を記載しています。
演算を指定するテンプレートでは必要な関数のシグネチャと代数的条件も確認できます。

Microsoft の C/C++ 拡張機能を導入し、`compilerPath` を GNU++23 に対応する `g++`、C++ 標準（`cppStandard`）を `gnu++23`、`includePath` をこのリポジトリのルートと ACL の `atcoder/` の親ディレクトリに設定してください。
リポジトリ外の AtCoder 作業フォルダからも `#include "library/algorithm/mo.hpp"` のように参照できます。
ビルド時にも同じディレクトリを `-I` または `CPLUS_INCLUDE_PATH` で指定します。IntelliSense の設定だけではコンパイラの探索パスは変わりません。

クラス名・関数名のホバーで説明を、呼び出し中の **パラメーターヒント**（`Ctrl+Shift+Space`）でシグネチャと引数の説明を参照できます。
表示する Doxygen タグや使用例の整形は拡張機能の版・表示先に依存します。表示が省略される場合はヘッダを開いて `@code` の使用例を確認してください。
コメントは前提条件や代数的条件を検査せず、演算・写像の関数定義を自動生成する機能もありません。複雑な初期化には以下のスニペットを利用できます。

[competitive-programming.code-snippets](snippets/competitive-programming.code-snippets) は C++ のみに適用する JSON ファイルです。
普段の作業フォルダを含む全ワークスペースで使うには、コマンドパレットの **Preferences: Configure User Snippets** から **New Global Snippets file** を選び、作成したファイルにこの JSON 全体をコピーします。
または既存のユーザースニペットディレクトリにこのファイルをコピーします。

- Windows: `%APPDATA%\Code\User\snippets\`
- macOS: `~/Library/Application Support/Code/User/snippets/`
- Linux: `~/.config/Code/User/snippets/`

特定のワークスペースだけで使う場合は、その作業フォルダの `.vscode/competitive-programming.code-snippets` にコピーします。
VSCode のプロファイル・Remote 環境では、その環境で **Configure User Snippets** を開く方法が確実です。コピー後の更新は手動で行います。

| 入力する prefix | 内容 | 挿入位置 |
| --- | --- | --- |
| `cp-treap-add-config` | 区間加算・区間和の型と関数 | グローバル（`main` より前） |
| `cp-treap-affine-config` | ACL modint のアフィン写像・区間和、合成順序 | グローバル |
| `cp-treap-init` | 初期配列から Treap を生成 | 関数内 |
| `cp-mo-sum` | クエリ登録・空区間の初期化・区間和の保存 | 関数内 |
| `cp-mo-directional` | 左右別の追加・削除と結果保存の編集用の枠 | 関数内 |
| `cp-sparse-min-config` | 非空区間の最小値用の演算と型 | グローバル |
| `cp-sparse-init` | 初期配列から構築し `[l,r)` を問い合わせ | 関数内 |

prefix を入力して `Ctrl+Space` で候補を選び、`Tab` で型・関数設定・変数名などのプレースホルダーを編集します。
設定スニペットを先に挿入し、生成側の型名と係数型を合わせてください。アフィン設定の後の `cp-treap-init` では係数型を `treap_affine::S` にします。
Mo はヘッダを include し、配列 `a` とクエリ列 `queries`（例: `std::vector<std::pair<int, int>>`）を用意してから挿入します。
`cp-mo-directional` の既定値は空の処理なので、各コールバックを実装してから利用してください。
Sparse Table の演算を変える場合は結合則と冪等則を満たす必要があります。和の計算には使えません。


## 提出用 bundler

`bundle.py` は `library/` 以下の自作ヘッダを展開し、提出用の 1 ファイルにまとめます。

通常は bundled ファイルを生成します。

```sh
python bundle.py main.cpp
python bundle.py main.cpp -o submission.cpp
```

標準出力へ直接出すこともできます。

```sh
python bundle.py main.cpp --stdout
python bundle.py main.cpp -o -
```

Windows では次のようにすると、展開した提出コードをそのままクリップボードへコピーできます。

```powershell
python bundle.py main.cpp --stdout | clip
```

ACL も含む提出ファイルを生成する場合は、`--expand-acl` を指定します。

```sh
python bundle.py main.cpp --expand-acl -o submission.cpp
python bundle.py main.cpp --expand-acl --stdout
python bundle.py main.cpp --expand-acl --acl-dir /path/to/ac-library -o submission.cpp
```

通常は `library/` の include だけを展開し、`<atcoder/...>` はそのまま残します。
`--expand-acl` を指定すると、ACL 内部の依存も再帰的に展開して重複を除きます。標準ライブラリの include は残します。
ACL は `CPATH`、`CPLUS_INCLUDE_PATH`、リポジトリ内の `ac-library/` と `atcoder/`、`/usr/local/include`、`/usr/include` の順に探します。
`--acl-dir` では ACL の checkout、include ディレクトリ、または `atcoder/` 自体を指定できます。
展開に必要な ACL ヘッダが見つからない場合はエラーにし、出力ファイルを書き換えません。

## 検証

ACL の `atcoder/` を C++ の include パスに置き、`online-judge-verify-helper` をインストールしてください。

```sh
python -m unittest discover -s .verify-helper/tests -v
oj-verify all
```

ヘッダの単独コンパイル・複数翻訳単位でのリンクと、`bundle.py` の提出ファイル生成も検証します。
全ヘッダの `@code` 使用例をそのままコンパイル・実行し、スニペットの JSON と既定値・主要プレースホルダーを展開したコードも検査します。
ACL 自体に非 inline 関数があるため、ACL に依存する FPS は複数翻訳単位のリンク検査から除きます。
ACL 単体と FPS で同じ重複シンボルが発生することを専用テストで検査します。定義箇所と再現手順は [監査記録](AUDIT.md#fps-の複数翻訳単位リンク) に記載しています。
`verify/` の `regression` テストは固定 seed の乱択・境界条件を assert で検査するため、`NDEBUG` を定義せず実行してください。

各ヘッダの検証対応、既知の制約、今後の追加候補は [監査記録](AUDIT.md) を参照してください。

CI の検証依存は `online-judge-verify-helper==5.6.0`、`online-judge-tools==11.5.1`、`setuptools==80.10.2`、ACL commit `864245a00b00dd008d1abfdc239618fdb7d139da` に固定しています。
ローカルでも同じ版を使用すると、依存の更新による検証結果の差を避けられます。
