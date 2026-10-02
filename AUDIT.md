# ライブラリ監査

全 31 ヘッダ、対応する verify、提出用 bundler、ドキュメント生成設定を確認しました。
最近追加された Mo / Doubling / LCA / 0-1 BFS / Sparse Table / Lowlink は、既存の独立な参照実装との比較と境界条件の検証を再実行しています。

## 修正

- `modpow`: 法 1・指数 0 の結果、負の底の正規化、大きな法での積のオーバーフローを修正。積に `__int128` を使用します。
- Bellman-Ford: 中間距離を `__int128` に変更し、非常に大きい経路や負閉路での符号付きオーバーフローを回避。最終距離の型と INF は維持しています。
- FPS: `f *= f[0]` のような係数参照のエイリアスを修正。積分を逆元計算 1 回に変更し、`log` / `pow` は指定次数より先の不要な係数を処理しないようにしました。次数の倍増・切り上げ計算のオーバーフローも回避しています。
- 静的双方向連結リスト: 隣接ノードの再挿入による自己ループを修正。既存ノードを移動するときは元の前後を再接続します。自分自身や既に隣接する位置への挿入は何もしません。
- Fenwick Tree・篩・いもす・階乗前計算: 大きなサイズでの添字加算や `n + 1` の符号付きオーバーフローを回避。
- `modpow` / `compress` に `inline` を付け、複数翻訳単位での重複定義を修正。使用する `<functional>` / `<utility>` を明示しました。
- Combination の前計算範囲、重み付き UF の演算範囲、FPS の法・畳み込み長・分母の制約、Treap の代数的条件と期待計算量を明記しました。

## API の方針

既存の公開 API 名・引数・距離型を維持しています。`solve_dijk` / `solve` / `query` / `prod` を見た目の統一のために改名していません。
配列・頂点は 0-indexed、区間は既存の半開区間 `[l, r)` を維持します。Sparse Table は非空区間と冪等演算専用です。
Dijkstra は `long long`、0-1 BFS は `int` の INF を使い、到達不能な経路は空配列を返します。Bellman-Ford は負閉路がある場合の距離を保証しません。
辺の表現は各アルゴリズムに必要な情報を維持し、Lowlink の親辺の判定には多重辺を区別できる辺 ID を使います。
グラフに辺を追加した後は再度 `solve` / `solve_dijk` / `build` が必要です。Dijkstra の結果取得には添字と実行済み状態の assert を追加しました。

## 各ライブラリの検証

以下のファイル名は対応する `verify/<category>/` 以下の `.test.cpp` を指します。
`regression_library_checker` などの回帰テストは A+B を入出力用に使い、固定 seed の乱択・境界条件を assert で検査します。

| カテゴリ / ヘッダ | 主な judge verify | 回帰・追加検証 |
| --- | --- | --- |
| algorithm / doubling | doubling_library_checker | doubling_regression_library_checker: 終端・循環・64 bit の遷移数 |
| algorithm / inversion_number | inversion_number_aoj | regression_library_checker: 全対比較・重複・大きな転倒数 |
| algorithm / lis | lis_library_checker | regression_library_checker: 全探索による長さ、添字列、狭義・広義 |
| algorithm / mo | mo_library_checker | mo_regression_library_checker: 空区間・重複・左右の操作順・再実行 |
| data_structure / coordinate_compression | coordinate_compression_library_checker | additional_regression_library_checker: 空配列・整数端点・重複 |
| data_structure / fenwick_tree | fenwick_tree_library_checker | additional_regression_library_checker: 負の更新・空区間・全区間の直接和 |
| data_structure / imos_2d | imos_2d_aoj | regression_library_checker: 切り詰め・乱択長方形・0×0 |
| data_structure / implicit_treap | implicit_treap_library_checker | implicit_treap_regression_library_checker: 非可換積・反転・区間作用・set・rotate・再構築 |
| data_structure / interval_set | — | regression_library_checker: 素朴な被覆、端点・長さ・小数 |
| data_structure / ordered_set | ordered_set_library_checker | additional_regression_library_checker: OrderedMultiset と std::multiset の比較 |
| data_structure / sparse_table | sparse_table_library_checker | sparse_table_regression_library_checker: min/max/gcd・非可換冪等演算・全非空区間 |
| data_structure / static_doubly_linked_list | — | additional_regression_library_checker: 列との移動比較・隣接再挿入・別成分間の移動 |
| data_structure / union_find | union_find_library_checker | regression_library_checker: 連結性・サイズ・多重辺と自己ループの辺数 |
| data_structure / weighted_union_find | weighted_union_find_aoj | additional_regression_library_checker: 独立なポテンシャルと全頂点対の差 |
| graph / bellman_ford | bellman_ford_aoj | regression_library_checker: 負閉路・到達不能・整数端点・負辺 DAG と Floyd-Warshall の比較 |
| graph / dijkstra | dijkstra_library_checker | regression_library_checker: Bellman-Ford との比較・経路の正当性・大きな重み |
| graph / lca | lca_library_checker、doubling_library_checker | tree_and_connectivity_regression_library_checker: 親を辿る参照実装・根の変更・20 万頂点の鎖 |
| graph / lowlink | lowlink_library_checker、lowlink_articulation_aoj、lowlink_bridges_aoj | tree_and_connectivity_regression_library_checker: 辺・頂点削除の参照実装、多重辺・自己ループ・非連結・再 build・深い鎖 |
| graph / topological_sort | — | regression_library_checker: 全頂点・全辺の順序、循環時の部分結果 |
| graph / zero_one_bfs | zero_one_bfs_aoj | tree_and_connectivity_regression_library_checker: Floyd-Warshall、0 辺・0 閉路・経路・再 solve |
| math / base_conversion | — | regression_library_checker: 基数 2..36、0・LLONG_MAX・小文字・桁からの復元 |
| math / combination | combination_library_checker | regression_library_checker: Pascal の三角形・直接の積・前計算 0 |
| math / divisors | divisors_aoj | regression_library_checker: 全候補の直接列挙 |
| math / eratosthenes | — | regression_library_checker: 0 と全候補の試し割り |
| math / formal_power_series | formal_power_series_inv_library_checker | formal_power_series_regression_library_checker: 素朴な積・長除法・微積分・gcd・inv/log/exp/pow・参照エイリアス・短い次数 |
| math / linear_sieve | — | regression_library_checker: 試し割りとの一致・因数からの復元 |
| math / mod_pow | mod_pow_aoj | regression_library_checker: 法 1・負の底・整数端点・直接の繰り返し積 |
| math / prime_factorize | prime_factorize_aoj | regression_library_checker: 篩との一致・因数からの復元 |
| string / rolling_hash | rolling_hash_aoj | regression_library_checker: 部分文字列と結合、additional_regression_library_checker: 128 bit 剰余との積比較 |
| string / run_length_encoding | — | additional_regression_library_checker: 空文字・同一文字・全バイト値を含む列の復元 |
| string / trie | trie_aoj | regression_library_checker: 空文字・重複・accept・LCP の直接比較 |

LIS・最短路・二重辺連結成分のように複数の正解がある Library Checker 問題は、公式 checker を使うことを実行ログで確認しました。
AOJ のトポロジカル順序を単純な完全一致で比較する verify は追加せず、辺の順序条件を直接検査しています。

## bundle.py とドキュメント生成

`online-judge-verify-helper` 5.6.0 の Python 依存解析は importlab で標準ライブラリの依存も辿り、Linux では 1 秒の固定タイムアウトを使います。
同じ解析を制限なしで測ると、この環境では約 1.15 秒・69 ノードでした。提出用 C++ の展開処理が停止しているわけではありません。
docs の `exclude` は依存解析より後に適用されるため、この警告を解消する設定にはなりません。解析時間の上限を変更する標準設定もありません。
警告だけを回避する大きな変更や外部ツールのパッチは加えていません。

小さな機能修正として、include / pragma once の末尾にある単一行のブロックコメントを認識するようにしました。
標準ライブラリの unittest で依存・重複・循環・外部 include・未解決依存・CLI エラーを検査し、展開したファイルだけでコンパイル・実行します。
実際の algorithm / FPS / Treap の回帰プログラムもリポジトリ外に展開してコンパイル・実行しています。
ヘッダの単独コンパイル・複数翻訳単位の検査と bundler テストを CI に追加しました。

## CI の依存固定

Cloud セットアップのスクリプトとインストール済みの版を照合しました。GitHub Actions は helper と ACL の最新版を取得していたため、検証済みの版に合わせました。

| 依存 | Cloud / 更新後の GitHub Actions |
| --- | --- |
| online-judge-verify-helper | 5.6.0 |
| online-judge-tools | 11.5.1 |
| setuptools | 80.10.2 |
| ACL | 864245a00b00dd008d1abfdc239618fdb7d139da |

CI は ACL のコミットを直接 shallow fetch して HEAD を照合し、Python の依存には `pip check` を実行します。
検証キャッシュのキーに workflow 自体も含め、依存版を変更したときに以前の検証済み時刻を再利用しないようにしました。
間接依存、runner、コンパイラ、Ruby の依存までは完全には固定していません。

## FPS の複数翻訳単位リンク

固定した ACL コミットで、FPS を含む 2 翻訳単位と、`<atcoder/convolution>` だけを含む 2 翻訳単位を別々にリンクしました。
GNU++23、`-O0` / `-O2` の両方で終了コード 1 となり、次の同じ 3 シンボルが `multiple definition` になりました。

| 重複する関数 | ACL の定義箇所 |
| --- | --- |
| `atcoder::internal::countr_zero(unsigned int)` | [internal_bit.hpp:33](https://github.com/atcoder/ac-library/blob/864245a00b00dd008d1abfdc239618fdb7d139da/atcoder/internal_bit.hpp#L33) |
| `atcoder::internal::floor_sum_unsigned(unsigned long long, unsigned long long, unsigned long long, unsigned long long)` | [internal_math.hpp:182](https://github.com/atcoder/ac-library/blob/864245a00b00dd008d1abfdc239618fdb7d139da/atcoder/internal_math.hpp#L182) |
| `atcoder::convolution_ll(const std::vector<long long>&, const std::vector<long long>&)` | [convolution.hpp:269](https://github.com/atcoder/ac-library/blob/864245a00b00dd008d1abfdc239618fdb7d139da/atcoder/convolution.hpp#L269) |

いずれもヘッダ内の非テンプレート・非 inline の外部定義です。include guard は翻訳単位ごとに働くため、この重複を防ぎません。
`convolution` は `internal_bit` と `modint` を読み込み、`modint` が `internal_math` を読み込みます。
自作 FPS を読み込まなくても発生するため、FPS の関数定義や最適化オプションが原因ではありません。
対照実験では、リポジトリ外の ACL コピーのこの 3 定義だけに `inline` を付けると、各翻訳単位で FPS を使用するプログラムがリンク・実行できました。
実際に使用する ACL は変更していません。依存ヘッダの書き換えや namespace を変える回避策をライブラリに組み込まず、FPS の除外を維持します。

再現用ソースは次のとおりです。ACL の include パスは固定コミットの checkout を指定してください。

```cpp
// /tmp/fps-first.cpp
#include "library/math/formal_power_series.hpp"
int other();
int main() { return other(); }
```

```cpp
// /tmp/fps-second.cpp
#include "library/math/formal_power_series.hpp"
int other() { return 0; }
```

```sh
g++ -std=gnu++23 -O2 -I . -I /path/to/ac-library /tmp/fps-first.cpp /tmp/fps-second.cpp -o /tmp/fps-linked
```

両方の include を `<atcoder/convolution>` に置き換えても同じ重複になります。
`test_fps_multiple_translation_unit_exclusion` はこの ACL 単体と FPS の重複シンボル集合が完全に一致することを検査します。
ACL 更新でリンクに成功するようになった場合、または FPS 独自の重複が追加された場合はテストが失敗し、除外の見直しを促します。

静的双方向連結リストは `<cassert>` / `<vector>` の明示的な include と `std::` 修飾に変更しました。公開型・メソッド・動作は維持しています。

## 検証結果

- `oj-verify all`: 初回監査に続き、依存固定と静的リスト整理後も検証済み時刻のキャッシュを外し、39 プログラム・796 ケースがすべて成功しました。新規 Python 3.12 venv と固定コミットの新規 ACL checkout を使用しています。
- `python -m unittest discover -s .verify-helper/tests -v`: マージ前確認で 8 テスト成功。全 31 ヘッダの単独コンパイル、FPS を除く 30 ヘッダの複数翻訳単位リンク、FPS / ACL の同じ 3 シンボルの衝突を検査しました。
- 静的リストを含む追加データ構造回帰テストと FPS 回帰テストを GNU++23 / `-O2 -Wall -Wextra -Werror` でコンパイル・実行し、成功しました。
- workflow の YAML と install スクリプトの構文、インストール済み Python 依存との版一致、`pip check`、ACL の HEAD と作業ツリーも確認しました。
- 初回監査の全 13 回帰プログラムは AddressSanitizer / UndefinedBehaviorSanitizer 付きで成功。最後のいもす変更にも再実行しました。
- 実際の verify 3 本を `bundle.py` で展開し、リポジトリ外でコンパイル・実行して同じ結果を確認しました。
- ネットワーク・judge 接続による検証不能はありません。docs の依存解析診断は残りますが、`oj-verify all` の終了コードは 0 です。

## 制約と残るリスク

- 有限距離・集計値・ポテンシャル・被覆長は各 API の値型に収める必要があります。距離の INF は有限値に使えません。
- RollingHash は衝突の可能性があります。Treap の計算量は期待値であり、再帰の深さも乱数に依存します。
- FPS は ACL の法・畳み込み長の制約と分母の可逆性が必要です。ACL 自体の非 inline 定義があるため、複数翻訳単位のリンク検査では FPS を除いています。
- 公開された内部配列や集合を直接変更すると不変条件を壊す場合があります。静的リストの connect で循環を作らないことも呼び出し側の条件です。
- bundle.py は一般的な C++ プリプロセッサではなく、条件付き include の評価や複数行にまたがるディレクティブまでは扱いません。
- docs の Python 依存解析タイムアウトと `migration/snippets.cpp` の参照用断片に関する解析警告は、judge のテスト失敗と区別して扱います。

## 次に追加する候補

| 優先順位 | ライブラリ | 理由・既存機能との関係 |
| --- | --- | --- |
| 1 | Heavy-Light Decomposition | AtCoder の木のパスクエリで汎用的。LCA を越えてパス・部分木を区間に分解し、集計は ACL segtree/lazy_segtree を再利用できる。Euler Tour の区間番号も一緒に設計する。 |
| 2 | Rerooting DP | 全頂点を根とする木 DP に使える。LCA・Lowlink・ACL に同等の汎用機能がない。 |
| 3 | Rollback Union-Find とオフライン動的連結性 | 既存 UF・ACL dsu は変更の巻き戻しを扱わない。辺の追加・削除を伴うクエリへの拡張になる。 |
| 4 | Li Chao Tree | 一次関数の最小値・最大値と DP 最適化に使える。既存の区間集計構造や ACL に同等品がない。 |
| 5 | Disjoint Sparse Table | 静的な非冪等・非可換な結合演算も O(1) で問い合わせ可能。既存 Sparse Table の制約を補うが、ACL segtree でも O(log N) で解けるため優先度を下げる。 |

基本的な BFS は 0-1 BFS の重み 1 で対応できます。SCC・最大流・最小費用流・セグ木・modint・畳み込みは ACL を優先し、同等実装の追加を急ぐ必要はありません。
この監査ではロードマップのライブラリは実装していません。
