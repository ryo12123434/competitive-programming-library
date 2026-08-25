# Competitive Programming Library

AtCoder の GNU++23 環境を想定した、C++ 競技プログラミング用ライブラリです。
提出コードへコピーして使いやすい、依存関係の少ない実装を目指します。

## ディレクトリ構成

```text
library/
  data_structure/
  graph/
  math/
  string/
verify/
  data_structure/
  graph/
  math/
  string/
template/
```

- `library/`: カテゴリ別のライブラリ本体
- `verify/`: ライブラリに対応する検証コード
- `template/`: AtCoder 提出用テンプレート

## 方針

- C++23（AtCoder の GNU++23）を使用する
- ライブラリは提出コードへコピーしやすい形にする
- 各ライブラリに計算量と使用上の注意を記載する
- 追加したライブラリには、可能な限り対応する検証コードを用意する

具体的なアルゴリズムは今後追加します。
