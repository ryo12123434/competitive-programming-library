#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

/**
 * @brief 座標圧縮 Li Chao Tree。直線 ax+b の追加と、登録済み整数座標での最小値取得を行う。
 * @note a,b,x は long long の全範囲に対応。乗算前に符号付き __int128 に拡張し、評価値も同型で返す。
 * |a*x+b| <= 2^126+2^63 なので符号付き128bitに収まる。INF は使わず、空状態は std::nullopt。
 * @note クエリ座標は構築前に列挙する。未知の座標・最大値クエリには対応しない。区間限定追加はadd_segmentを使う。
 * @par Complexity
 * 入力座標数 K、重複除去後 M に対し構築 O(K log(K+1)) 時間・O(K+M) ピーク空間、保持空間 O(M)。
 * add_line / query は O(log(M+1)) 時間・追加空間 O(1)。区間追加は O(log^2(M+1))。空座標の add_line は O(1)。
 * @code
 * #include "library/data_structure/li_chao_tree.hpp"
 * #include <cassert>
 * int main() {
 *     LiChaoTree tree(std::vector<long long>{-2, 0, 3, 3});
 *     assert(!tree.query(0));
 *     tree.add_line(2, 1);
 *     tree.add_line(-1, 4);
 *     auto answer = tree.query(3);
 *     assert(answer && *answer == 1); // long long へ暗黙に狭めない
 * }
 * @endcode
 */
class LiChaoTree {
public:
    /// @brief ax+b の評価値の型。標準 ostream の直接出力には非対応。
    using Value = __int128;

    /**
     * @brief クエリ座標をコピーし、ソート・重複除去して空の木を構築する。
     * @param xs 今後 query(x) するすべての座標。順不同・重複・空配列も可。
     * @pre 座標数は int に収まり、必要な配列を確保できること。
     * @note 元の配列を変更しない。空配列の場合 add_line は何もしない。
     * @par Complexity
     * K 個の入力に対して O(K log(K+1)) 時間・O(K+M) ピーク空間、保持空間 O(M)。
     */
    explicit LiChaoTree(std::vector<long long> xs) {
        assert(xs.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        std::sort(xs.begin(), xs.end());
        xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
        xs_.assign(xs.begin(), xs.end());
        assert(xs_.size() <= tree_.max_size() / 4);
        tree_.resize(xs_.size() * 4);
    }

    /// @brief 登録した異なるクエリ座標の個数を O(1) で返す。直線数ではない。
    int size() const { return static_cast<int>(xs_.size()); }

    /**
     * @brief 全登録座標で有効な直線 ax+b を追加する。同傾き・同一直線も可。
     * @param a 傾き。long long の全範囲。
     * @param b 切片。long long の全範囲。
     * @par Complexity
     * O(log(M+1)) 時間・追加空間 O(1)。M==0 は O(1)。
     */
    void add_line(long long a, long long b) {
        if (!xs_.empty())
            insert_line(Line{a, b}, 0, size(), 1);
    }

    /**
     * @brief xが半開区間[left,right)内の登録座標だけで有効な直線ax+bを追加する。
     * @pre left<=right。範囲端は登録不要。空区間は何もしない。
     * @par Complexity O(log^2(M+1))時間・O(log(M+1))呼出しスタック。
     */
    void add_segment(long long left, long long right, long long a, long long b) {
        assert(left <= right);
        int l = static_cast<int>(std::lower_bound(xs_.begin(), xs_.end(), left) - xs_.begin());
        int r = static_cast<int>(std::lower_bound(xs_.begin(), xs_.end(), right) - xs_.begin());
        add_segment_indices(l, r, a, b);
    }
    /// @brief ソート・重複除去後の座標添字[l,r)で有効な直線。LLONG_MAXの座標も含められる。
    /// @pre 0<=l<=r<=size()。計算量はadd_segmentと同じ。
    void add_segment_indices(int l, int r, long long a, long long b) {
        assert(0 <= l && l <= r && r <= size());
        if (l == r)
            return;
        auto update = [&](auto &&self, std::size_t node, int low, int high) -> void {
            if (r <= low || high <= l)
                return;
            if (l <= low && high <= r) {
                insert_line(Line{a, b}, low, high, node);
                return;
            }
            int mid = low + (high - low) / 2;
            self(self, node * 2, low, mid);
            self(self, node * 2 + 1, mid, high);
        };
        update(update, 1, 0, size());
    }

    /**
     * @brief 登録済み座標 x での最小値を取得する。
     * @param x コンストラクタへ渡した座標のいずれか。
     * @pre x は登録済みであること。違反は assert で検出（NDEBUG 時は検査無効）。
     * @return 最小値を含む optional<Value>。直線未追加なら std::nullopt。値 0 も有効。
     * @note long long に変換する場合は呼び出し側で範囲を確認する。
     * @par Complexity
     * O(log(M+1)) 時間・追加空間 O(1)。
     */
    std::optional<Value> query(long long x) const {
        const auto it = std::lower_bound(xs_.begin(), xs_.end(), x);
        assert(it != xs_.end() && *it == x);
        const int index = static_cast<int>(it - xs_.begin());
        int l = 0, r = size();
        std::size_t node = 1;
        std::optional<Value> answer;
        while (l < r) {
            if (tree_[node]) {
                const Value value = tree_[node]->eval(x);
                if (!answer || value < *answer) answer = value;
            }
            if (r - l == 1) break;
            const int mid = l + (r - l) / 2;
            if (index < mid) { r = mid; node *= 2; }
            else { l = mid; node = node * 2 + 1; }
        }
        return answer;
    }

private:
    struct Line {
        long long a, b;
        Value eval(long long x) const { return static_cast<Value>(a) * x + b; }
    };
    void insert_line(Line line, int l, int r, std::size_t node) {
        while (true) {
            if (!tree_[node]) {
                tree_[node] = line;
                return;
            }
            const int mid = l + (r - l) / 2; // 座標値の加減算をせず添字の中点を使う
            const bool better_left = line.eval(xs_[l]) < tree_[node]->eval(xs_[l]);
            const bool better_mid = line.eval(xs_[mid]) < tree_[node]->eval(xs_[mid]);
            if (better_mid)
                std::swap(line, *tree_[node]);
            if (r - l == 1 || line.a == tree_[node]->a)
                return;
            if (better_left != better_mid) {
                r = mid;
                node *= 2;
            } else {
                l = mid;
                node = node * 2 + 1;
            }
        }
    }

    std::vector<long long> xs_;
    std::vector<std::optional<Line>> tree_;
};
