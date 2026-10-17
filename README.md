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

## VSCode スニペット

`main`（別名 `atcmain`）、遅延セグ木・Treap・Mo・Sparse Table の設定と初期化、
`*-help` のコメント形式の早見表を用意しています。
[一覧・導入方法・使い方](snippets/README.md)を参照してください。
リポジトリ外の作業フォルダでも、VSCode のユーザースニペットへコピーして利用できます。


## 提出用 bundler

`bundle.py` は `library/` 以下の自作ヘッダを展開し、提出用の 1 ファイルにまとめます。

`#ifdef LOCAL` 内の include は、条件分岐ごとそのまま残します。`LOCAL` 未定義の提出版では
デバッグ用ヘッダが不要です。展開後のコードを `-DLOCAL` で動かす場合は、そのヘッダを参照できる include パスを指定します。
`#ifndef LOCAL` の else 側や `#if defined(LOCAL)` など、LOCAL を直接判定する同等の分岐も扱います。
他の条件式は評価せず、通常の自作ヘッダと `--expand-acl` による ACL の展開は従来どおりです。

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

