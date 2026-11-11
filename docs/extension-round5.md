# 第5弾：実装・検証・監査記録

最新リモートを取得し、main=`427ee52`、第4弾=`15cc5d9`を比較した。
第4弾は未マージでmainは95ヘッダのため、120ヘッダを持つ第4弾からローカルの
`feat/round5-library-extensions`を作成した。開始時の未コミット変更はなし。
RNAT側のリポジトリを操作せず、commit・push・PR作成・merge・履歴の破棄は行わない。

## Phase A

| ヘッダ/API | 計算量・仕様 |
| --- | --- |
| `directed_mst` | Chu–Liu/Edmonds。O(VE+V)時間、復元対応を保持するため最悪O(VE+V)空間。負辺・多重辺可、費用128bit、親と元辺IDを復元 |
| `steiner_tree` / `steiner_tree_with_edges` | O(3^K V + 2^K(V+E)log(V+1))時間、O(2^K V+V+E)空間。非負無向辺、重複端点除去、空端点は費用0 |
| `ComplementGraph` | 未訪問双方向リストでO(V+E)のBFS/成分分解。残留走査は元の隣接辺に課金し、補辺は明示生成しない |
| `RangeEdgeGraph` | 上下2本のセグ木。構築O(N)、区間辺追加O(log N)辺。区間間は中継点1個。最短路は既存Dijkstraを利用 |

有向MSTの復元は縮約前の採用辺に、縮約後に選んだ流入辺を元辺ID・元の到達先で上書きする。
Steiner DPは各maskで分割結合→多始点Dijkstra。decrease-key集合を使い、多重辺でもheap候補数をV以下に保つ。
RangeEdgeGraphの有限距離は既存DijkstraのINF未満。Steiner・有向MSTはsigned 128bitで合計する。

## Phase B

| ヘッダ/API | 計算量・仕様 |
| --- | --- |
| `PartiallyPersistentUnionFind` | 過去leader/same O(log N)、size O(log N+log Q)、merge償却O(log N)、保持O(N+Q)。時刻は厳密増加、失敗mergeも時刻を進める |
| `SparseFenwickTree2D<T,Coord>` | P点の構築O(P log²P)、保持O(P log P)、更新/矩形和O(log²P)。密な2D BITと異なり、全更新候補点を事前登録 |
| `DynamicBitset` | W=ceil(N/64)、bit演算・シフト・count・探索O(W+1)、単一bit操作O(1)。先頭文字がbit0、find_nextは指定位置より後 |
| `DualSegmentTree<S,F,mapping,composition,id>` | 構築O(N)、区間作用/一点取得/一点上書きO(log N)。composition(f,g)はgの後にf。区間集約不要 |
| `SegmentLiChaoTree` | O(M)保持、全域追加/取得O(log M)、区間追加O(log²M)。LiChaoTreeと実装を共有する型別名 |

部分永続UFはRollback/完全永続とは異なり、過去へ更新しない。時刻0は初期状態で、未来参照は例外。
疎BITの座標順は(y,x)、矩形は[y1,y2)×[x1,x2)。未登録点の更新はNDEBUGでも例外。
DynamicBitsetはNOT・シフト後も末尾の余分なbitを必ず0にする。異なる長さのbit演算は例外。
LiChaoTreeは挿入処理を共有関数へ移し、区間追加を追加した。既存APIの名前・引数・返り値は維持。
部分区間の直線があると親が空でも子に候補があるため、queryは空ノードでも下降する。

## 検証方針

各Phaseの `.verify-helper/tests/fixtures/round5_*.cpp` で全探索・独立な愚直解と比較し、
C++20+ASan/UBSan、GNU++23、NDEBUG有効時のAPI、単独ヘッダ、複数TU、使用例、スニペット、bundleを実行する。
公式データを取得してローカルの公式チェッカーで実行することと、外部サーバーへの提出は区別する。
AOJ DSL_2_E adapterで1-indexed閉区間→0-indexed半開区間の変換ミスを検出して修正し、ランダム回帰を追加した。

## Phase C

| ヘッダ/API | 計算量・仕様 |
| --- | --- |
| `SuffixArrayQuery(text,sa,lcp)` | ACLで作った配列を受け取り既存SparseTableでRMQ。構築O(N log N)、接尾辞LCP・部分文字列比較O(1)、パターン範囲O((P+1)log(N+1)) |
| `DynamicRollingHash` | 既存RollingHashの61bit剰余演算を再利用。構築O(N)、1文字変更・区間取得O(log N)、結合O(1)、保持O(N) |
| `enumerate_runs(forward,reversed)` | LCP/LCSの周期格子走査。O(N log N)時間・空間。最小周期と極大半開区間、重複なし |

接尾辞配列には空接尾辞を含めない。`encode`で符号付きcharによる順序の違いを避ける。
空文字列のACL `lcp_array`は呼ばず空配列を渡す。LCPクエリ自体は空接尾辞Nを扱える。
固定ACLの`<atcoder/string>`は6個の非inline定義が複数TUで衝突することを実際に再現した。
そのため新ヘッダはSA/LCP配列を受け取り、ACLを暗黙includeしない。新ヘッダの同時include・複数TUは通過する。
動的ハッシュは構築時の基数を保持し、既存の静的基数変更に影響されない。固定基数は再現用で衝突保証はない。
ランダム基数でも衝突確率は0ではなく、固定した異なる同長列についての多項式の根の個数による上界を記載した。

Runsでは周期pごとに隣接するp格子点をLCP/LCSで伸ばし、長さ2p以上の極大p周期区間を候補とする。
極大周期区間は必ずそのような格子点対を含む。候補の真の最小周期をq<pとすると、
長さ2p以上でのFine–Wilfの条件からqはpを割り切る。p周期の延長可能性はq周期と一致するため、
同じ両端を持つ候補の最小pを残せば真のrunとなる。候補を計数ソートし、比較ソートによる追加log因子を避ける。
全2進文字列（長さ0〜12）と独立な周期全探索、ランダム列、20万文字の定数列・Thue–Morse列で検証した。

## Phase D

| ヘッダ/API | 計算量・仕様 |
| --- | --- |
| `discrete_log_u64(a,b,m,budget)` | gcd除去付きBSGS。除去後の法Mに対して期待O(sqrt(M)+log²m)時間、O(sqrt(M))空間。ハッシュ衝突集中時O(M+log²m)。最小非負解、一般の正整数法、0^0=1 |
| `segmented_sieve(L,R,base_budget,width_budget)` | 閉区間。S=floor(sqrt(R)), W=対象区間長としてO(S log log(S+2)+W log log(S+2)+S)時間、O(S+W+出力)空間。負数・0・1は除外 |
| `fps_sqrt<Polynomial>(f,n)` | Newton法と既存FPS/剰余平方根。NTT可能な奇素数法でO(n log(n+1))と定数項の開平、作業O(n)。法2はO(n)。解なしはoptional |
| `PolynomialProductTree<Polynomial>` / `multipoint_evaluation` | Product Treeと剰余木。構築O(N log²N)、通常の多点評価O(N log²N)、保持O(N log N)。次数Dが大きい場合は別途O(M(D+N)) |
| `polynomial_interpolation<Polynomial>` | 微分値の多点評価と一括逆元、Product Treeを逆に合成。O(N log²N)と逆元1回、保持O(N log N)。重複xは値が同じでもnullopt |
| `ModLinearSystem<Mint>` / `matrix_rank_mod` / `determinant_mod` | M式N変数のRREFはO(MN min(M,N))体演算、O(MN)コピー。解空間基底は追加O((N-rank)N)。行列式はO(N³)、0×0は1 |

BSGSと区間篩は予算超過を`length_error`として解なし・空集合と区別する。法0は拒否する。
区間篩は既存eratosthenesを使うためsqrt(R)<=INT_MAX、既定ではsqrt(R)と幅がそれぞれ1000万以下。
FPS平方根は先頭非零次数が奇数なら解なし。次数2kならx^kを分離し、決定できない末尾は0にする。
多項式拡張は既存FormalPowerSeriesを型引数で再利用し、固定ACLの既知の複数TU衝突を暗黙に持ち込まない。
畳み込みは法とNTT長の制約を満たす必要がある。Gaussian eliminationは素数法を検査し、解なしでも係数行列の階数を返す。

## Phase E：既存120ヘッダの監査

基準コミットの120ヘッダ（algorithm 14、data_structure 26、graph 26、geometry 14、math 27、debug 4、string 9）を確認した。
内部用integer_traits.hppも含む。新規18ヘッダを加え、最終対象は138ヘッダ。
実装の境界・計算型・計算量・再帰・配列再確保と参照・同時include・assert内の副作用を確認し、既存回帰テストと照合した。
全入力での正しさを形式的に証明したという意味ではない。

再現して修正した既存の問題は以下。

- `coordinate_compression.hpp`と`union_find.hpp`：グローバルの`using namespace std`により、利用者の`int count`が`std::count`と衝突してコンパイル不能になる。
  `<bits/stdc++.h>`を必要な標準includeに置き換え、std::を明示。公開型・関数シグネチャ・処理は変更していない。
  利用者が推移的なstd導入に依存していた場合は、自分のコードでstd::または必要なusingを指定する。
- `doubling.hpp`と`bellman_ford.hpp`：assertだけで使用する検査変数が、`-DNDEBUG -Wall -Wextra -Werror`で未使用エラーになる。
  `[[maybe_unused]]`で検査を保持し、アルゴリズムは変更しない。修正前は全ヘッダの同設定でこの2件だけ失敗することを確認した。
- 既存Li Chao TreeにはPhase Bの区間追加機能を追加した。旧APIの回帰と区間追加混在を別々に検証した。

座標圧縮のランダム順位、UFの多重辺・自己ループを含む成分サイズと辺数、Doublingの64bit遷移、Bellman–Fordの64bit端点値も回帰へ追加した。

残る設計上の制約：

- 多くの旧APIはassertで事前条件を検査する。NDEBUG時も「正しい入力」という契約は残り、不正入力の安全性を全APIで保証しない。
- Implicit Treapは優先度に依存する再帰を使う。期待高さO(log N)、最悪O(N)という既存の制約を維持する。
- 整数幾何は全中間値がsigned 128bitに収まる座標・面積が必要。実数幾何は有限値と相対誤差帯のUncertainを呼び出し側で扱う。
- 固定ACLのconvolution側は`internal::countr_zero`・`internal::floor_sum_unsigned`・`convolution_ll`、
  string側は`internal::sa_naive`・`internal::sa_doubling`・`suffix_array`の非テンプレート2種・文字列版`lcp_array`・`z_algorithm`が非inlineで、実測でリンク衝突する。
  自作ヘッダ側の追加衝突は検出されていない。FPSの既存除外は依存側の明確な理由を持ち、新ヘッダに追加の除外は設けていない。
- docs生成でPython用依存解析がbundle.py・tools/stress.pyにtimeout警告を出す。通常のC++提出bundleとは別処理で、bundle回帰は通過する。
  今回は依存解析器・既存Pythonツールの構成を変更しない。

## 変更ファイル

新規ヘッダ18個：

- `library/data_structure/dual_segment_tree.hpp`
- `library/data_structure/dynamic_bitset.hpp`
- `library/data_structure/partially_persistent_union_find.hpp`
- `library/data_structure/segment_li_chao_tree.hpp`
- `library/data_structure/sparse_fenwick_tree_2d.hpp`
- `library/graph/complement_graph.hpp`
- `library/graph/directed_mst.hpp`
- `library/graph/range_edge_graph.hpp`
- `library/graph/steiner_tree.hpp`
- `library/math/discrete_log.hpp`
- `library/math/fps_sqrt.hpp`
- `library/math/gauss_elimination_mod.hpp`
- `library/math/multipoint_evaluation.hpp`
- `library/math/polynomial_interpolation.hpp`
- `library/math/segmented_sieve.hpp`
- `library/string/dynamic_rolling_hash.hpp`
- `library/string/runs.hpp`
- `library/string/suffix_array_query.hpp`

新規検証は`verify/{graph,data_structure,string,math}/round5_*.test.cpp`の17ファイル。
回帰は`.verify-helper/tests/fixtures/round5_{a,b,c,d,e}.cpp`、
統合検査は`test_round5.py`と`round5_manifest.json`。`test_snippets.py`のカタログに今回分を追加した。
スニペットは`snippets/round5.code-snippets`、導入案内は`snippets/README.md`、READMEには短いリンクのみ追加した。
既存実装の変更は上記5ヘッダに限定し、AGENTS・CI・bundle.py・tools/stress.pyは変更していない。

## 新規judge verifyの実行記録

公式入力と公式出力／チェッカーを取得し、ローカルで実行した結果。外部サーバーへの提出はしていない。

| 検証プログラム（verify/以下） | 成功ケース |
| --- | ---: |
| `data_structure/round5_dual_segment_tree_aoj.test.cpp` | 22 |
| `data_structure/round5_segment_li_chao_library_checker.test.cpp` | 13 |
| `data_structure/round5_sparse_fenwick_2d_library_checker.test.cpp` | 18 |
| `graph/round5_complement_graph_library_checker.test.cpp` | 25 |
| `graph/round5_directed_mst_library_checker.test.cpp` | 12 |
| `graph/round5_steiner_tree_library_checker.test.cpp` | 24 |
| `math/round5_determinant_library_checker.test.cpp` | 25 |
| `math/round5_discrete_log_library_checker.test.cpp` | 20 |
| `math/round5_fps_sqrt_library_checker.test.cpp` | 35 |
| `math/round5_interpolation_library_checker.test.cpp` | 9 |
| `math/round5_linear_system_library_checker.test.cpp` | 27 |
| `math/round5_matrix_rank_library_checker.test.cpp` | 36 |
| `math/round5_multipoint_library_checker.test.cpp` | 11 |
| `math/round5_segmented_sieve_aoj.test.cpp` | 4 |
| `string/round5_runs_library_checker.test.cpp` | 24 |
| `string/round5_suffix_query_library_checker.test.cpp` | 24 |

新規16対象・329ケース成功。新規のAtCoder ABC331 Fはデータ取得に必要なDropboxトークンがないため未検証。
同一adapterは、点更新・区間回文判定を愚直解と比較するローカル回帰を実行した。
新規17対象中、16対象の公式データを取得できた。RangeEdgeGraph・部分永続UF・DynamicBitsetは
適切な専用judgeへ無理に対応させず、明示グラフ／時刻ごとの成分／ビット配列との独立比較で補った。

再実行にはCIと同じACLコミット`864245a00b00dd008d1abfdc239618fdb7d139da`、
`online-judge-verify-helper==5.6.0`、`online-judge-tools==11.5.1`を使用する。

```sh
python -m unittest discover -s .verify-helper/tests -v
oj-verify all --timeout 1800
```

ACLのincludeパスを設定する。既存整数の独立参照テスト全体にはBoostヘッダも必要。
新規回帰の各Phaseは`test_round5.Round5Test.test_a_regression`などを個別実行できる。

## 最終検証結果

- 現行の回帰テストIDは168件すべて成功を確認した（skipなし）。一括実行で160件成功、
  既存GapsCTestの`<set>`不足によるsetupエラー1件を検出し、`fixtures/gaps_c.cpp`へincludeを追加した。
  修正後の同クラス5件と追加の監査3件を個別実行し、現行168件のIDと成功記録の一致を確認した。
- 全138ヘッダはC++20／GNU++23・NDEBUG・Wall/Wextra/Werrorで単独コンパイル成功。
  同時include・複数TUも成功（固定ACLに由来する既存FPS除外の再現検査を含む）。
- 各Phaseの回帰はC++20+ASan/UBSanとGNU++23で成功。使用例・スニペットJSON・prefix重複・API実行を確認した。
  通常bundleと`--expand-acl`の両方を検証し、後者はACL/リポジトリのincludeパスなしでも実行できた。
- 最終の`oj-verify all --timeout 1800`は136対象中132対象が検証済み。
  今回の最終全体実行では14対象266ケースを再実行して成功、118対象は有効な成功キャッシュを利用した。
  新規16対象329ケースは、その前のPhase別実行で公式データを新たに取得してすべて実行済み。
- 未検証4対象はAtCoder DP N、ARC070 C、ABC293 E、ABC331 F。いずれも`--dropbox-token is not given`による
  公式データ取得不能で、プログラムのWA/RE/TLEではない。コマンドの終了値はこのため1。
  セキュリティ設定や認証を変更して回避していない。
- docs生成はPython依存解析timeoutの既知警告を残す。公開サイトのデプロイは行っていない。
- Gitのindexは空、HEADは基準コミット`15cc5d9fa35ba367efc541d0c12087a4453c16c0`のまま。
  ローカル`feat/round5-library-extensions`に未コミット差分を保持し、commit/push/PR/mergeは行っていない。

最終`git status --short`は変更済み9ファイル・新規44ファイル（合計53）、ステージ済み0。
変更済みの既存ファイル：

- `README.md`
- `snippets/README.md`
- `.verify-helper/tests/test_snippets.py`
- `.verify-helper/tests/fixtures/gaps_c.cpp`
- `library/algorithm/doubling.hpp`
- `library/graph/bellman_ford.hpp`
- `library/data_structure/coordinate_compression.hpp`
- `library/data_structure/union_find.hpp`
- `library/data_structure/li_chao_tree.hpp`

新規44ファイルの内訳は、上記18ヘッダ・17verify・5回帰fixture・統合テストPython・manifest JSON・
スニペットJSON・この記録。追加機能の未実装項目はなく、公式データ未取得と既知の設計制約は前述のとおり。
