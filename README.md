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
```

- `library/`: カテゴリ別のライブラリ本体
- `verify/`: ライブラリに対応する検証コード
- `template/`: AtCoder 提出用テンプレート


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
ACL 自体に非 inline 関数があるため、ACL に依存する FPS は複数翻訳単位のリンク検査から除きます。
ACL 単体と FPS で同じ重複シンボルが発生することを専用テストで検査します。定義箇所と再現手順は [監査記録](AUDIT.md#fps-の複数翻訳単位リンク) に記載しています。
`verify/` の `regression` テストは固定 seed の乱択・境界条件を assert で検査するため、`NDEBUG` を定義せず実行してください。

各ヘッダの検証対応、既知の制約、今後の追加候補は [監査記録](AUDIT.md) を参照してください。

CI の検証依存は `online-judge-verify-helper==5.6.0`、`online-judge-tools==11.5.1`、`setuptools==80.10.2`、ACL commit `864245a00b00dd008d1abfdc239618fdb7d139da` に固定しています。
ローカルでも同じ版を使用すると、依存の更新による検証結果の差を避けられます。
