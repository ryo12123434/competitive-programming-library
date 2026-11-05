# Competitive Programming Library

AtCoder の GNU++23 環境を想定した、C++ 競技プログラミング用ライブラリです。

## ディレクトリ構成

```text
library/
  algorithm/
  data_structure/
  graph/
  geometry/
  math/
  string/
verify/
  algorithm/
  data_structure/
  graph/
  geometry/
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

`main`（別名 `atcmain`）、遅延セグ木・Treap・Mo・Sparse Table・整数幾何の設定と呼び出し、
`main`（別名 `atcmain`）、遅延セグ木・Treap・Mo・Sparse Table・整数幾何・HLD・Li Chao Treeの設定と呼び出し、
`*-help` のコメント形式の早見表を用意しています。
[一覧・導入方法・使い方](snippets/README.md)を参照してください。
リポジトリ外の作業フォルダでも、VSCode のユーザースニペットへコピーして利用できます。

## 整数幾何

`library/geometry/` に整数の点・向き・偏角ソート・閉線分判定・凸包・多角形の面積を用意しています。
[API・境界ケース・数値保証](library/geometry/README.md)を確認してください。
計算には符号付き128bit整数を使い、座標差は拡張してから求めます。面積の累積には別途上限があります。

## 整数幾何

`library/geometry/` に整数の点・向き・偏角ソート・閉線分判定・凸包・多角形の面積を用意しています。
[API・境界ケース・数値保証](library/geometry/README.md)を確認してください。
計算には符号付き128bit整数を使い、座標差は拡張してから求めます。面積の累積には別途上限があります。

## 木の経路・直線クエリ

- [HLD](library/graph/hld.md)：反復探索による O(N) 構築。頂点・辺の経路と部分木を区間に分解し、ACL Segtree / Lazy Segtree と組み合わせます。非可換演算用に経路順・区間の向きを保持します。
- [Li Chao Tree](library/data_structure/li_chao_tree.md)：クエリ座標を事前指定する座標圧縮方式。直線追加と最小値取得 O(log M)、保持空間 O(M)。評価値は `__int128`、空状態は `std::nullopt` です。


## 全方位DP・整数集合・回文

`rerooting.hpp`、`binary_trie.hpp`、`xor_basis.hpp`、`manacher.hpp` を用意しています。
各ヘッダの Doxygen に API・前提条件・使用例があります。対応するスニペットは `snippets/` にあります。
静的な区間順位には `wavelet_matrix.hpp`、履歴の巻き戻しには `rollback_union_find.hpp` を利用できます。

## 区間・文字列・グラフの拡張

発展的な区間・文字列・木クエリには次のヘッダも利用できます。API・制約・使用例は各ヘッダの Doxygen、
初期化用スニペットは `snippets/advanced.code-snippets` を参照してください。

- 区間：`segment_tree_beats.hpp`、`persistent_segment_tree.hpp`、`swag.hpp`、`disjoint_sparse_table.hpp`
- 文字列：`kmp.hpp`、`aho_corasick.hpp`、`suffix_automaton.hpp`、`booth.hpp`
- グラフ：`offline_dynamic_connectivity.hpp`、`virtual_tree.hpp`、`dsu_on_tree.hpp`、`two_edge_connected_components.hpp`

## ABC・ARC向けの補助アルゴリズム

ABC・ARC向けの追加分は、各ヘッダの Doxygen と `snippets/gaps.code-snippets` から参照できます。

- グラフ：Kruskal、閉路検出、オイラー路、Warshall–Floyd、なもり分解
- 数列・DP：Cartesian Tree、Sliding Window、Slope Trick、Monotone Minima、分割統治DP、Knapsack
- 数学・クエリ：GF(2)連立方程式、2D Fenwick、静的2D Range Tree、並列二分探索、更新付きMo、区間加算Fenwick

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


## 第4弾の拡張

[比較対象・API・計算量・制約・検証記録](docs/extension-round4.md)を参照してください。
