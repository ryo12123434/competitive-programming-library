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

## 検証

ACL の `atcoder/` を C++ の include パスに置き、`online-judge-verify-helper` をインストールしてください。

```sh
python -m unittest discover -s .verify-helper/tests -v
oj-verify all
```

ヘッダの単独コンパイル・複数翻訳単位でのリンクと、`bundle.py` の提出ファイル生成も検証します。
ACL 自体に非 inline 関数があるため、ACL に依存する FPS は複数翻訳単位のリンク検査から除きます。
`verify/` の `regression` テストは固定 seed の乱択・境界条件を assert で検査するため、`NDEBUG` を定義せず実行してください。

各ヘッダの検証対応、既知の制約、今後の追加候補は [監査記録](AUDIT.md) を参照してください。
