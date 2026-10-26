#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <utility>
#include <vector>

/**
 * @brief union by size・経路圧縮なしの、巻き戻し可能な Union-Find。
 * @note 成功・失敗を問わず merge 1回が undo 1回に対応する。辺数は管理しない。
 * snapshot はこの個体の寿命内だけ有効。個体の同一性を保つためコピー・ムーブは不可。
 * @par Complexity
 * 初期化 O(N)、leader / same / size / merge は O(log(N+1))、undo / snapshot / is_valid は O(1)。
 * rollback は取り消す merge 数に比例。保持空間 O(N+H)、H は最大履歴数。
 * @code
 * #include "library/data_structure/rollback_union_find.hpp"
 * #include <cassert>
 * int main() {
 *     RollbackUnionFind uf(3);
 *     auto initial = uf.snapshot();
 *     assert(uf.merge(0,1) && !uf.merge(0,1));
 *     assert(uf.undo() && uf.same(0,1)); // 失敗した merge だけ取り消す
 *     assert(uf.rollback(initial) && !uf.same(0,1) && uf.components()==3);
 * }
 * @endcode
 */
class RollbackUnionFind {
public:
    /**
     * @brief 不透明な履歴トークン。auto token=uf.snapshot() で取得する。既定構築は無効。
     * @note undo / rollback で捨てた履歴のトークンは無効。同じ深さに再び達しても復活しない。
     * 残っている接頭履歴のトークンと初期状態のトークンは引き続き有効。生成例はクラスを参照。
     */
    class Snapshot {
        friend class RollbackUnionFind;
        const RollbackUnionFind* owner_ = nullptr;
        std::size_t depth_ = 0;
        std::uint64_t id_ = 0;
        Snapshot(const RollbackUnionFind* owner, std::size_t depth, std::uint64_t id)
            : owner_(owner), depth_(depth), id_(id) {}
    public:
        Snapshot() = default;
    };

    /// @brief n 個の独立成分で初期化。@param n 頂点数。@pre n>=0。頂点は [0,n)。
    explicit RollbackUnionFind(int n) : components_(n) {
        assert(n >= 0);
        parent_.assign(n, -1);
    }
    RollbackUnionFind(const RollbackUnionFind&) = delete;
    RollbackUnionFind& operator=(const RollbackUnionFind&) = delete;
    RollbackUnionFind(RollbackUnionFind&&) = delete;
    RollbackUnionFind& operator=(RollbackUnionFind&&) = delete;

    /// @brief x の代表元。経路圧縮しない。@pre x は [0,n)。O(log(N+1))。
    int leader(int x) const {
        assert(0 <= x && x < static_cast<int>(parent_.size()));
        while (parent_[x] >= 0) x = parent_[x];
        return x;
    }
    /// @brief x,y が同一成分か。@pre x,y は [0,n)。O(log(N+1))。
    bool same(int x, int y) const { return leader(x) == leader(y); }
    /// @brief x の成分の頂点数。@pre x は [0,n)。O(log(N+1))。
    int size(int x) const { return -parent_[leader(x)]; }
    /// @brief 連結成分数。O(1)。
    int components() const { return components_; }

    /**
     * @brief x,y を併合し、別成分だったら true。同一頂点・同一成分でも1履歴を追加する。
     * @pre x,y は [0,n)。この個体への merge 総回数が UINT64_MAX 未満。
     * @par Complexity
     * O(log(N+1))。履歴は deque で保存し、既存の履歴要素を移動しない。
     */
    bool merge(int x, int y) {
        x = leader(x); y = leader(y);
        assert(next_id_ < std::numeric_limits<std::uint64_t>::max());
        const auto id = ++next_id_;
        if (x == y) { history_.push_back({-1,-1,0,0,id}); return false; }
        if (parent_[x] > parent_[y]) std::swap(x,y);
        history_.push_back({x,y,parent_[x],parent_[y],id});
        parent_[x] += parent_[y]; parent_[y] = x; --components_;
        return true;
    }
    /// @brief 直前の merge を取り消す。履歴が空なら変更せず false。O(1)。
    bool undo() {
        if (history_.empty()) return false;
        const auto change = history_.back(); history_.pop_back();
        if (change.a != -1) {
            parent_[change.a] = change.old_a; parent_[change.b] = change.old_b; ++components_;
        }
        return true;
    }
    /// @brief 現在の履歴を記録する。状態全体のコピーはしない。O(1)。
    Snapshot snapshot() const { return Snapshot(this,history_.size(),history_.empty()?0:history_.back().id); }
    /// @brief token がこの個体の現在の履歴の接頭状態か。外国・破棄済みトークンは false。O(1)。
    bool is_valid(Snapshot token) const {
        return token.owner_ == this && token.depth_ <= history_.size() &&
               (token.depth_ == 0 || history_[token.depth_-1].id == token.id_);
    }
    /// @brief token まで戻す。無効なら一切変更せず false。O(取り消す履歴数+1)。
    bool rollback(Snapshot token) {
        if (!is_valid(token)) return false;
        while (history_.size() > token.depth_) undo();
        return true;
    }

private:
    struct Change { int a,b,old_a,old_b; std::uint64_t id; };
    std::vector<int> parent_;
    std::deque<Change> history_;
    std::uint64_t next_id_ = 0;
    int components_;
};
