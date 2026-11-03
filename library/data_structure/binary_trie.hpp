#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

/**
 * @brief 符号なし整数の多重集合。重複数を保持し、削除した枝のノードを再利用する。
 * @tparam UInt uint32_t / uint64_t 等の64bit以下の符号なし整数（bool 不可）。
 * @tparam Bits 使用ビット数（1～UInt のビット数）。insert・xor_all・XORクエリの入力はこの幅に収める。
 * @note 符号付き負数の大小順には対応しない。明示的キャストした場合も符号なし値として扱う。
 * @pre 個数は size_t に収まり、同時に必要なノード数は UINT32_MAX 未満であること。
 * @par Complexity
 * 各操作 O(Bits)、xor_all / size / empty は O(1)。insert / erase は配列拡張を含め償却 O(Bits)。
 * 空間 O(過去の最大同時異値数 * Bits)。挿入・削除の累積回数には比例しない。
 * @code
 * #include "library/data_structure/binary_trie.hpp"
 * #include <cassert>
 * int main() {
 *     BinaryTrie<> trie;
 *     trie.insert(3); trie.insert(3); trie.insert(5);
 *     assert(trie.count(3) == 2 && trie.min_xor(2) == 1);
 *     assert(trie.erase(3) && trie.count(3) == 1 && !trie.erase(9));
 *     trie.xor_all(1);
 *     assert(trie.count(2) == 1 && trie.kth(0) == 2);
 * }
 * @endcode
 */
template<class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class BinaryTrie {
    static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> &&
                  !std::is_same_v<UInt, bool> && sizeof(UInt) <= 8);
    static_assert(1 <= Bits && Bits <= std::numeric_limits<UInt>::digits);
    using Index = std::uint32_t;
    static constexpr Index nil = std::numeric_limits<Index>::max();
    struct Node { std::array<Index, 2> child{nil, nil}; std::size_t count = 0; };
    std::vector<Node> nodes_{Node{}};
    std::vector<Index> free_;
    UInt tag_ = 0;
    static bool fits(UInt x) {
        if constexpr (Bits == std::numeric_limits<UInt>::digits) return true;
        else return (x >> Bits) == 0;
    }
    Index allocate() {
        if (!free_.empty()) { auto id = free_.back(); free_.pop_back(); nodes_[id] = Node{}; return id; }
        assert(nodes_.size() < nil);
        nodes_.push_back(Node{});
        return static_cast<Index>(nodes_.size()-1);
    }
    std::size_t node_count(Index id) const { return id == nil ? 0 : nodes_[id].count; }
    std::optional<UInt> xor_arg(UInt x, bool maximum) const {
        assert(fits(x));
        if (empty()) return std::nullopt;
        Index node = 0; UInt raw = 0, target = x ^ tag_;
        for (int bit = Bits-1; bit >= 0; --bit) {
            int side = int((target >> bit) & UInt{1}) ^ int(maximum);
            if (!node_count(nodes_[node].child[side])) side ^= 1;
            raw |= UInt(side) << bit;
            node = nodes_[node].child[side];
        }
        return raw ^ tag_;
    }
public:
    /// @brief 総要素数（重複を含む）。O(1)。
    std::size_t size() const { return nodes_[0].count; }
    /// @brief 空かを O(1) で返す。
    bool empty() const { return size() == 0; }
    /// @brief x を1個追加する。@pre x は Bits に収まり、総個数が size_t に収まる。
    void insert(UInt x) {
        assert(fits(x) && size() < std::numeric_limits<std::size_t>::max());
        x ^= tag_;
        Index node = 0; ++nodes_[node].count;
        for (int bit = Bits-1; bit >= 0; --bit) {
            const int side = int((x >> bit) & UInt{1});
            auto next = nodes_[node].child[side];
            if (next == nil) { next = allocate(); nodes_[node].child[side] = next; }
            node = next; ++nodes_[node].count;
        }
    }
    /// @brief x の個数。幅に収まらない値は0。
    std::size_t count(UInt x) const {
        if (!fits(x)) return 0;
        x ^= tag_; Index node = 0;
        for (int bit = Bits-1; bit >= 0; --bit) {
            node = nodes_[node].child[(x >> bit) & UInt{1}];
            if (node == nil) return 0;
        }
        return nodes_[node].count;
    }
    /// @brief x を1個削除。存在しない場合は変更せず false。
    bool erase(UInt x) {
        if (count(x) == 0) return false;
        x ^= tag_;
        std::array<Index, Bits+1> path{};
        for (int bit = Bits-1, depth = 0; bit >= 0; --bit, ++depth)
            path[depth+1] = nodes_[path[depth]].child[(x >> bit) & UInt{1}];
        for (Index id : path) --nodes_[id].count;
        for (int depth = Bits; depth > 0 && nodes_[path[depth]].count == 0; --depth) {
            nodes_[path[depth-1]].child[(x >> (Bits-depth)) & UInt{1}] = nil;
            free_.push_back(path[depth]);
        }
        return true;
    }
    /// @brief x との XOR を最小にする要素。空なら nullopt。
    std::optional<UInt> argmin_xor(UInt x) const { return xor_arg(x, false); }
    /// @brief x との XOR を最大にする要素。空なら nullopt。
    std::optional<UInt> argmax_xor(UInt x) const { return xor_arg(x, true); }
    /// @brief 最小 XOR 値（要素そのものではない）。空なら nullopt。
    std::optional<UInt> min_xor(UInt x) const { auto y = argmin_xor(x); return y ? std::optional<UInt>(*y ^ x) : std::nullopt; }
    /// @brief 最大 XOR 値。空なら nullopt。
    std::optional<UInt> max_xor(UInt x) const { auto y = argmax_xor(x); return y ? std::optional<UInt>(*y ^ x) : std::nullopt; }
    /// @brief 全要素を x と XOR する。@pre x は Bits に収まる。O(1)。
    void xor_all(UInt x) { assert(fits(x)); tag_ ^= x; }
    /// @brief 符号なし昇順で0-indexedの k 番目。範囲外なら nullopt。
    std::optional<UInt> kth(std::size_t k) const {
        if (k >= size()) return std::nullopt;
        Index node = 0; UInt result = 0;
        for (int bit = Bits-1; bit >= 0; --bit) {
            const int zero = int((tag_ >> bit) & UInt{1});
            const auto count = node_count(nodes_[node].child[zero]);
            if (k < count) node = nodes_[node].child[zero];
            else { k -= count; node = nodes_[node].child[zero ^ 1]; result |= UInt{1} << bit; }
        }
        return result;
    }
    /// @brief x 未満の要素数。幅を超えた x なら全要素数。
    std::size_t count_less(UInt x) const {
        if (!fits(x)) return size();
        Index node = 0; std::size_t result = 0;
        for (int bit = Bits-1; bit >= 0 && node != nil; --bit) {
            const int zero = int((tag_ >> bit) & UInt{1});
            const int side = int((x >> bit) & UInt{1});
            if (side) result += node_count(nodes_[node].child[zero]);
            node = nodes_[node].child[zero ^ side];
        }
        return result;
    }
};
