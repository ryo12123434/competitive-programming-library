# タスク A：ABC 向けの 5 ライブラリ

基点は `16dfc15`。専用 worktree `/workspace/cpl-task-a`、ブランチ `task-a` で作業する。
既存ファイル、README、AGENTS.md、GitHub Actions、共通設定、タスク C のファイルは変更しない。

## 公開 API

すべてグローバル名前空間、0-indexed。各ヘッダの Doxygen にコンパイル可能な使用例がある。

| ヘッダ | 構築 | 主な取得 API | 時間・空間 |
|---|---|---|---|
| `library/graph/functional_graph.hpp` | `FunctionalGraph graph(next)` | `cycles()`、`on_cycle(v)`、`cycle_id(v)`、`distance_to_cycle(v)`、`cycle_entry(v)`、`reachable_count(v)`、`size()` | 前計算 O(N) 時間・空間、取得 O(1) |
| `library/graph/euler_tour.hpp` | `EulerTour tour(tree, root = 0)` | `parent(v)`、`depth(v)`、`subtree_size(v)`、`tin(v)`、`tout(v)`、`subtree_range(v)`、`vertex(index)`、`order()`、`size()` | 前計算 O(N) 時間・空間、取得 O(1) |
| `library/data_structure/prefix_sum_2d.hpp` | `PrefixSum2D<T> sum(grid)` | `query(u, d, l, r)`、`height()`、`width()` | 前計算 O((H+1)(W+1)) 時間・空間、矩形和 O(1) |
| `library/graph/tree_diameter.hpp` | `TreeDiameter<T> diameter(tree)` | `length()`、`endpoints()`、`get_path()`、重み付き辺型 `Edge{to, cost}` | 前計算 O(N) 時間・空間、取得 O(1) |
| `library/math/matrix.hpp` | `Matrix<T> a(h, w, value = T{})` または `Matrix<T> a(values)` | `a(y, x)`、`a * b`、`Matrix<T>::identity(n)`、`a.pow(exponent)`、`height()`、`width()` | 正の次元で積 O(HWK)、K 次正方行列の累乗 O(K³ log(exponent+1) + K²) |

`cycles()`、`order()`、`get_path()` は const 参照を返す。必要なら呼び出し側でコピーする。
行列積の結果の空間は O(HK+H)、累乗の追加空間は O(K²)。
空の次元の構築・乗算の正確な計算量は各ヘッダを参照する。

## 使用上の規約

- Functional Graph は `next[v]` が必ず `[0,N)`。空配列も可。終端 `-1` は不可。
  閉路列は遷移順・最小頂点始まり、閉路 ID はその最小頂点の昇順。
  到達頂点数には出発点自身を含める。k 回遷移は既存の Doubling を併用する。
- Euler Tour と Tree Diameter は、各辺を両方向に格納した非空の連結な無向木が前提。
  Euler Tour は隣接リストの順に訪問し、各頂点が 1 回だけ現れる。
  根の親は `-1`。`order()[tin(v)] == v`。
  部分木は `[tin(v), tout(v))`、根も変更可能。反復探索のため再帰深さに依存しない。
- PrefixSum2D は Imos2D と同じ引数順 `(上端, 下端, 左端, 右端)`。
  `query(u,d,l,r)` は `[u,d) × [l,r)` の和。空矩形は零、範囲外の切り詰めはしない。
  空入力は 0×0、空行 H 個は H×0。構築後は元配列を変更しても結果は変わらない。
  `T{}` を零元とする可換加法群で `+=`・`-=` が必要。
- Tree Diameter の非重み付き入力は `vector<vector<int>>`、辺重みは 1。
  重み付き入力は `vector<vector<TreeDiameter<T>::Edge>>`。負辺は不可。
  長さ 0 の経路も許し、全辺 0 なら単一頂点が直径になる場合がある。
  頂点数 1 なら長さ 0、端点 `(0,0)`、経路 `{0}`。
- Matrix は長方形にも対応。要素は `a(y,x)` で変更し、行そのもののサイズ変更は公開しない。
  積は左の列数と右の行数が一致すること。累乗は正方行列のみ。
  0 乗は単位行列、1 乗は元の値。0×0 の累乗も 0×0。
  指数は `unsigned long long`。負の値を渡して符号なしへ変換しない。
  ACL static/dynamic modint を使用可能だが、使用中に dynamic_modint の法を変更しない。
  `T{}` が加法単位元、`T{1}` が乗法単位元、`+=` と `*` が必要。
  加算の可換・結合則、乗算の結合則、分配則・零元の吸収則が前提。
- 数値計算は途中の加算・減算・乗算も型に収まる必要がある。
  動的なオーバーフロー検出や自動的な拡張整数への変換は行わない。
  前提条件の一部は assert で検出する。NDEBUG では入力検査は無効になる。

## スニペット

専用ファイルは `snippets/task-a/abc.code-snippets`。

| prefix | 展開内容 |
|---|---|
| `functional` | FunctionalGraph の構築 |
| `euler` | 根を指定した EulerTour の構築 |
| `prefix2d` | 型を指定した PrefixSum2D の構築 |
| `diameter` | 型を指定した TreeDiameter の構築 |
| `matrix` | 行数・列数・型を指定した零行列の構築 |
| `matrix-pow` | 非負整数乗の結果の取得 |

すべて関数内に展開する。先頭のコメントに示す include はグローバル位置に追加する。
型、変数名、入力配列、根、行数・列数、指数は Tab で編集する。

別の VSCode ワークスペースで使う場合は、このファイルを VSCode のユーザースニペット用フォルダへコピーする。
Windows では通常 `%APPDATA%\Code\User\snippets\task-a.code-snippets`。
ライブラリの include パスは別途リポジトリのルートを指定する。

## 検証の内容

- `.verify-helper/tests/test_task_a_libraries.py` と専用 C++ fixture を追加。
  全列挙・固定 seed のランダム比較、UBSan、assert 有効・無効、単独ヘッダ、複数翻訳単位、Doxygen 使用例、スニペット、bundle を確認する。
- Functional Graph：N <= 6 の全 50,069 遷移とランダム 300 例を各始点から直接シミュレーション。
  空グラフ、20 万頂点の鎖・単一閉路・全自己ループも確認する。
- Euler Tour：N <= 5 の全ラベル付き木 146 個のすべての根とランダムな木を再帰参照実装と比較。
  深さ 20 万の木・星でも確認する。
- Tree Diameter：小規模全木・ランダム木を Floyd-Warshall の全点対距離と比較し、経路の頂点・辺・長さも検証。
  重み 0・1・ランダム、単一頂点、20 万頂点、long long 上限の距離を含む。
- PrefixSum2D：全 2×2 三値配列、ランダム配列の全矩形を直接和と比較。
  空行・空矩形・負値・long long・modint・加減算代入だけを持つ型を含む。
- Matrix：長方形・空次元の積、全 2×2 三値行列の累乗とランダム modint 行列を直接計算・1 歩ずつ進む DP と比較。
  0・1 乗、1×1、指数の符号なし 64bit 上限、次元不一致・不揃いな配列の assert を確認。

外部 verify はタスク A 専用の新規ファイルに追加する。

| ファイル（verify/ 以下） | 問題と検証範囲 |
|---|---|
| `graph/task_a_euler_tour_library_checker.test.cpp` | Library Checker `vertex_add_subtree_sum`：FenwickTree と部分木区間を組み合わせる |
| `graph/task_a_tree_diameter_library_checker.test.cpp` | Library Checker `tree_diameter`：任意の有効経路を専用 checker で検証 |
| `data_structure/task_a_prefix_sum_2d_aoj.test.cpp` | AOJ `ITP1_7_C`：各セル・各行・各列・全体の和。任意矩形は独立全探索で補完 |
| `math/task_a_matrix_product_library_checker.test.cpp` | Library Checker `matrix_product`：modint の長方形積 |
| `math/task_a_matrix_power_library_checker.test.cpp` | Library Checker `pow_of_matrix`：modint の正方行列累乗 |
| `graph/task_a_regression_library_checker.test.cpp` | 上記の全探索・回帰テストを起動。A+B は起動・入出力用であり、Functional Graph の専用 judge ではない |

Functional Graph の全取得 API に直接対応する judge は用意していないため、独立シミュレーションとの比較で補完する。
Windows/MinGW 実機での動作は未確認。Python テストは Windows では UBSan の指定を省く。

### 実行結果

- GNU g++ 14.2.0、`-std=gnu++23`。ACL は commit `864245a00b00dd008d1abfdc239618fdb7d139da`。
  online-judge-verify-helper 5.6.0、online-judge-tools 11.5.1 を使用した。
- `python -m unittest discover -s .verify-helper/tests -v`：全 61 テスト成功。
  全 46 ヘッダの単独コンパイル、全 57 Doxygen 使用例、既存の複数翻訳単位テストと追加 5 ヘッダの複数翻訳単位テストが成功。
  追加テストは UBSan 有効、NDEBUG 有効・無効のコンパイル、既存を含む全 prefix の重複検査、追加全スニペットの既定値・編集例のコンパイル、通常 bundle の単独コンパイルが成功。
- 新規 6 verify：114 ケース成功（専用問題の 102 ケースと、A+B 起動の回帰テスト 12 ケース）。
- `oj-verify all`：終了コード 0。既存を含む全 51 verify・計 1,039 ケースが検証済み。
  この呼び出しでは 46 verify・937 ケースを実行し、先行実行済みの新規 5 verify・102 ケースは成功マーカーを利用した。
  judge の実際のテスト失敗・ネットワーク制限による未実施はない。
- ドキュメント生成は 106 ページ・15 静的ファイルの出力まで完了し、追加 5 ヘッダのページも生成された。
  既存の `bundle.py`・`tools/stress.py` に対する Python 依存解析タイムアウトと、そのドキュメント内 bundle の警告は残る。
  本タスクでは既存ファイルを変更せず、通常の提出用 bundle のテストは成功している。
- 既存の追跡済みファイルの差分は 0。追加 15 ファイルに限定したパッチを `/workspace/task-a.patch` に出力し、元の `work` への `git apply --check` が成功。
  コミット・push・main へのマージは行っていない。

## 統合時の作業

- 既存ファイルを変更せず追加ファイルだけを取り込める。タスク C の HLD・Li Chao Tree は含まない。
- 共有 README のカテゴリ・ライブラリ一覧への追記、共有スニペット導入説明への追記は統合時に行う。
- 現状はスニペットをサブフォルダに分けて、既存の固定カタログテストとの競合を避けている。
  共通カタログへ移動する場合は `test_snippets.py` の固定一覧と API テストを統合する。
  現在も専用テストで全フォルダの prefix 重複と新スニペットのコンパイルを検証する。
- AGENTS.md・GitHub Actions・共通設定・既存ライブラリの修正は不要。
