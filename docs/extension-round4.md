# 第4弾：比較・設計・検証記録

基準は `origin/main` の `427ee52d5606b56325fa4f4339c6dfe6c5b0933a`（95ヘッダ）。
既存の公開APIを維持し、比較対象のコードを転載せず、標準アルゴリズムを独立実装する。

## 比較対象

2026-11-06に以下のコミットの追跡ファイル一覧と関連するAPI・説明を実際に確認した。

| ライブラリ | 確認コミット | 特に参照した機能・パス |
| --- | --- | --- |
| [maspy](https://github.com/maspypy/library) | `35d52a50a676addf1c5be4bb45939a359b5bfbe6` | `nt/is_prime.hpp`, `nt/factor.hpp`, `setfunc/`, `flow/bipartite_matching.hpp`, `convex/smawk.hpp`, `ds/fastset.hpp`, `geo/` |
| [drken](https://github.com/drken1215/algorithm) | `fec569ece425439cd097de738179963c8868840e` | `Math/is_prime_Miller_Rabin.cpp`, `Math/pollard_rho.cpp`, `Flow/hopcroft_karp.cpp`, `Graph/biconnected_components.cpp`, `Tree/`, `Optimization/`, `Polynomial/` |
| [tatyam](https://github.com/tatyam-prime/kyopro_library) | `cdd6fed5763773fbbfe216f2f1ad1badb59436a3` | `UnionFindMonoid.cpp`。他の比較対象ほど網羅的な機能一覧ではない |
| [Nyaan](https://github.com/NyaanNyaan/library) | `b3981adc80a800b2584980b01821324ea6c77183` | `prime/`, `set-function/`, `fps/kitamasa.hpp`, `fps/berlekamp-massey.hpp`, `graph/biconnected-components.hpp`, `tree/block-cut-tree.hpp`, `segment-tree/dynamic-segment-tree.hpp` |

ACLは畳み込み・modint・一般最大流を提供するが、今回の数論・集合変換そのものは提供しない。
二部専用マッチングは単位容量ネットワーク構築を省き、最小頂点被覆も直接復元する用途。
既存の試し割り・線形篩、Li Chao Tree、Manacher、永続セグ木は維持する。

## Phase A：数論・集合変換

| ヘッダ | 主なAPI | 計算量・注意 |
| --- | --- | --- |
| `miller_rabin.hpp` | `is_prime_u64(n)` | 64bit全域の決定的7底。O(log n) 回の固定幅剰余乗算、空間O(1) |
| `pollard_rho.hpp` | `factorize_u64(n,seed,attempts)`, `factorize_u64_counts` | Brent法。乱数写像モデルで最小素因数pの分割は期待O(sqrt(p))。最悪成功時間は保証しない |
| `multiplicative_functions.hpp` | `MultiplicativeFunctions(n).phi/mu`, `euler_phi_u64`, `mobius_u64` | 一括は既存LinearSieveを再利用してO(n)時間・空間。単一値はPollard Rho |
| `subset_transform.hpp` | `subset_zeta/mobius`, `superset_zeta/mobius`, `or_convolution`, `and_convolution` | 長さ2^BにO(B 2^B)。変換は追加空間O(1)、畳み込みはO(2^B) |
| `xor_convolution.hpp` | `walsh_hadamard_transform(a,inverse)`, `xor_convolution` | 同じ計算量。逆変換には長さが可逆な係数環。偶数法modintは長さ1以外拒否 |
| `mod_sqrt.hpp` | `mod_sqrt_u64(a,p)` | 素数法限定、最小非負根またはnullopt。非剰余探索Z候補を含めO(Z log p + log²p) |

Pollard Rhoは1軌道を2^20更新で打ち切り、既定64回の再試行を使い切ると例外を返す。
seedで再現・変更が可能。0の分解と非素数法の平方根はinvalid_argument。
φ・μの0番は一括篩用の番兵0であり、単一値APIでは0を許さない。
係数が通常の整数の場合、変換・積の中間値もその型に収める。

ローカル検証は `.verify-helper/tests/test_round4.py` とphase別fixtureを使う。
各phaseでC++20+ASan/UBSan、GNU++23、単独ヘッダ、複数翻訳単位、Doxygen例、スニペット、bundleを実行する。
公式問題は対応する `verify/*/round4_*.test.cpp`。公式データの取得・実行と外部サイトへの提出は区別する。

## Phase B：グラフ・木

| ヘッダ | 主なAPI | 計算量・規約 |
| --- | --- | --- |
| `bipartite_graph.hpp` | `bipartite_coloring(n,edges)` | O(V+E)。成功時2色、失敗時は始点再掲なしの奇閉路と辺ID |
| `bipartite_matching.hpp` | `BipartiteMatching`, `add_edge`, `solve`, `matching`, `minimum_vertex_cover` | 初期化O(V)+探索O(E√V)、保持O(V+E)。被覆復元O(V+E) |
| `centroid_decomposition.hpp` | `CentroidDecomposition(tree)`, `parent`, `level`, `ancestors` | O(N log N)時間・保持空間。祖先ごとに元の木での距離 |
| `biconnected_components.hpp` | `BiconnectedComponents`, `add_edge`, `build`, `components`, `component_edges` | 既存Lowlinkのord/low/関節点を利用。O(V+E) |
| `block_cut_tree.hpp` | `BlockCutTree(bcc)`, `tree`, `vertex_node`, `articulation_vertex` | O(V+E)。ブロックノードの後に関節点ノード |
| `tree_isomorphism.hpp` | `TreeIsomorphism.rooted_id/unrooted_id` | ソート+共有辞書による厳密判定。N頂点・M既存型にO(N log(N+M+1)) |

全探索・愚直比較に加え、HKの10万段増加路、重心分解の20万頂点の鎖を反復処理で検証する。
二重頂点連結成分の自己ループは、所属頂点の最初の通常ブロックへ割り当てる。
ループだけの頂点にも1個のブロックを作る。この規約でBlock-Cut森の不要な葉を増やさない。
関節点は複数ブロックに現れるが、辺IDはそれぞれちょうど1ブロックに属する。
木同型のIDは同じencoder内でのみ比較する。整数IDそのものは他encoderへの持ち出し用正規形ではない。
