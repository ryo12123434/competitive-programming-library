# HLD：木の経路・部分木を区間にする

`#include "library/graph/hld.hpp"`、グローバル名前空間の `HLD` を使います。
無向辺を両方向に格納した非空の木から `HLD hld(tree, root = 0)` で構築します。
既存の LCA・EulerTour とは独立して利用でき、セグ木の種類や演算は固定しません。

| 用途 | API | 時間 |
|---|---|---|
| 親・深さ・部分木サイズ | `parent(v)` / `depth(v)` / `subtree_size(v)` | O(1) |
| heavy child・heavy path の先頭 | `heavy_child(v)` / `head(v)` | O(1) |
| 頂点から一次元化位置へ | `tin(v)` | O(1) |
| 部分木の右端 | `tout(v)` | O(1) |
| 位置から頂点へ・全訪問順序の参照 | `vertex(index)` / `order()` | O(1) |
| 最小共通祖先 | `lca(u,v)` | O(log N) |
| 部分木の半開区間 | `subtree_range(v, edge = false)` | O(1) |
| 経路順に並んだ区間列 | `path_segments(u,v, edge = false)` | O(log N) |

構築は O(N) 時間・空間、再帰なし。頂点・位置とも 0-indexed です。
最大部分木の子を heavy child とし、同サイズなら隣接リストで先の子を選びます。
heavy child を優先した DFS 順なので、EulerTour と同じ `tin` になるとは限りません。
経路は O(log N) 個の区間に分解され、ACL Segtree と組み合わせた経路クエリは O(log² N) です。
区間更新には同じ区間列を ACL Lazy Segtree へ渡せます。

頂点の値は `seg.set(hld.tin(v), value)` で格納します。和の完全な例はヘッダの Doxygen にあります。
辺の値は**根から見て深い端点**の位置に格納し、経路・部分木の取得に `edge = true` を指定します。
辺の経路では LCA の位置を除き、部分木では v と親を結ぶ辺を除きます。
`u == v` の頂点経路は v だけ、辺経路は空です。根の親・葉の heavy child は `-1` です。

## 非可換演算：一次関数の合成

`PathSegment{l,r,reversed}` は必ず `u → v` 順に返ります。
`reversed == false` なら位置 `l,...,r-1`、true なら `r-1,...,l` の順で読みます。
セグ木は正順・逆順の集約を両方保持してください。区間の集約値から逆順の集約を
復元できるとは限りません。行列積や逆関数のない写像でも、この二方向の保持が必要です。

以下は ACL Segtree で、通過した順に一次関数を適用する完全な例です。
`combine(f,g)` は f の後に g を適用します。合成は結合的、単位元は `{1,0}` です。

```cpp
#include "library/graph/hld.hpp"
#include <atcoder/modint>
#include <atcoder/segtree>
#include <cassert>
using Mint = atcoder::modint998244353;
struct Affine { Mint a, b; };
Affine combine(Affine f, Affine g) { return {g.a * f.a, g.a * f.b + g.b}; }
struct Node { Affine forward, backward; };
Node op(Node l, Node r) {
    return {combine(l.forward, r.forward), combine(r.backward, l.backward)};
}
Node e() { return {{1, 0}, {1, 0}}; }
int main() {
    HLD hld(std::vector<std::vector<int>>{{1}, {0}});
    atcoder::segtree<Node, op, e> seg(hld.size());
    seg.set(hld.tin(0), {{2, 1}, {2, 1}});
    seg.set(hld.tin(1), {{3, 4}, {3, 4}});
    auto evaluate = [&](int u, int v, Mint x) {
        Affine result{1, 0};
        for (auto part : hld.path_segments(u, v)) {
            const Node node = seg.prod(part.l, part.r);
            result = combine(result, part.reversed ? node.backward : node.forward);
        }
        return result.a * x + result.b;
    };
    assert(evaluate(0, 1, 1) == 13);
    assert(evaluate(1, 0, 1) == 15);
}
```

通常の整数和などは、セグ木内の演算の中間結果も型に収めてください。
HLD 自体は値の演算を行わず、位置と向きだけを提供します。
