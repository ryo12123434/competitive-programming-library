#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

/**
 * @brief 静的整数列の順位・k番目・値域頻度を扱う Wavelet Matrix。
 * @tparam T 64bit以下の符号付き／符号なし整数型。bool 不可。既定 long long。
 * @note 符号ビット反転で整数順序を保ち、最小符号化値を引いた差の bit_width を B とする。
 * LLONG_MIN～LLONG_MAX、uint64_t 全域にも対応。全要素同一・空列では B=0。
 * @pre N<=INT_MAX。元の列は構築後に変更しても結果に影響しない。
 * @par Complexity
 * 構築 O(N(B+1)) 時間・O(N) ワードの作業空間。各クエリ O(B+1)。
 * 保持空間はビット本体 NB ビットと、64bitブロックごとの32bit rank 目録・O(B)ワード。
 * おおむね N*B*1.5 ビット。元の配列を保持せず、access で復元する。
 * @code
 * #include "library/data_structure/wavelet_matrix.hpp"
 * #include <cassert>
 * int main() {
 *     WaveletMatrix<> matrix(std::vector<long long>{-3,7,-3,2});
 *     assert(matrix.access(1)==7 && matrix.rank(-3,3)==2);
 *     assert(matrix.kth_smallest(0,4,1)==-3 && matrix.kth_largest(0,4,0)==7);
 *     assert(matrix.range_freq(0,4,-3,3)==3);
 *     assert(matrix.prev_value(0,4,2)==-3 && matrix.next_value(0,4,3)==7);
 * }
 * @endcode
 */
template<class T = long long>
class WaveletMatrix {
    static_assert(std::is_integral_v<T> && !std::is_same_v<T,bool> && sizeof(T)<=8);
    using U = std::make_unsigned_t<T>;
    static constexpr int digits = std::numeric_limits<U>::digits;
    struct BitVector {
        std::vector<std::uint64_t> words;
        std::vector<int> prefix;
        explicit BitVector(int n) : words((static_cast<std::size_t>(n)+63)/64), prefix(words.size()+1) {}
        void set(int i) { words[i/64] |= std::uint64_t{1} << (i%64); }
        bool get(int i) const { return (words[i/64] >> (i%64)) & 1; }
        void build() { for(std::size_t i=0;i<words.size();++i) prefix[i+1]=prefix[i]+std::popcount(words[i]); }
        int rank1(int r) const {
            const int offset = r%64;
            return prefix[r/64] + (offset ? std::popcount(words[r/64] & ((std::uint64_t{1}<<offset)-1)) : 0);
        }
    };
    int n_ = 0, bits_ = 0;
    U base_ = 0, maximum_ = 0;
    std::vector<BitVector> levels_;
    std::vector<int> zeros_;
    static U encode(T x) {
        U code = static_cast<U>(x);
        if constexpr (std::is_signed_v<T>) code ^= U{1} << (digits-1);
        return code;
    }
    T decode(U key) const {
        U code = key + base_;
        if constexpr (std::is_signed_v<T>) { code ^= U{1} << (digits-1); return std::bit_cast<T>(code); }
        else return code;
    }
    void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0<=l && l<=r && r<=n_); }
public:
    /// @brief values をコピーせず参照して前計算する。空列も可。保持データは独立する。
    explicit WaveletMatrix(const std::vector<T>& values) {
        assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
        n_ = static_cast<int>(values.size());
        if (!n_) return;
        base_ = maximum_ = encode(values[0]);
        for (T x : values) { base_=std::min(base_,encode(x)); maximum_=std::max(maximum_,encode(x)); }
        bits_ = std::bit_width(U(maximum_-base_));
        if (!bits_) return;
        std::vector<U> current(n_), next(n_);
        for (int i=0;i<n_;++i) current[i]=encode(values[i])-base_;
        levels_.reserve(bits_); zeros_.reserve(bits_);
        for (int bit=bits_-1;bit>=0;--bit) {
            levels_.emplace_back(n_); auto& bv=levels_.back(); int zero=0;
            for (int i=0;i<n_;++i) { if((current[i]>>bit)&U{1})bv.set(i);else ++zero; }
            bv.build(); zeros_.push_back(zero);
            int left=0,right=zero;
            for(U key:current) { if((key>>bit)&U{1})next[right++]=key;else next[left++]=key; }
            current.swap(next);
        }
    }
    /// @brief 元配列の要素数。O(1)。
    int size() const { return n_; }
    /// @brief 実際に確保したビット段数。O(1)。
    int bit_width() const { return bits_; }
    /// @brief 元配列の i 番目を返す。@pre i は [0,size())。
    T access(int i) const {
        assert(0<=i && i<n_);
        U key=0;
        for(int level=0;level<bits_;++level) {
            const auto& bv=levels_[level];const int ones=bv.rank1(i);
            if(bv.get(i)) { key |= U{1}<<(bits_-level-1); i=zeros_[level]+ones; }
            else i-=ones;
        }
        return decode(key);
    }
    /// @brief [0,r) 中の x の個数。@pre 0<=r<=size()。
    int rank(T x,int r) const { return count(0,r,x); }
    /// @brief [l,r) 中の x の個数。@pre 0<=l<=r<=size()。
    int count(int l,int r,T x) const {
        check_range(l,r);
        const U code=encode(x);
        if(l==r || code<base_ || code>maximum_) return 0;
        const U key=code-base_;
        for(int level=0;level<bits_;++level) {
            const int a=levels_[level].rank1(l),b=levels_[level].rank1(r);
            if((key>>(bits_-level-1))&U{1}) { l=zeros_[level]+a;r=zeros_[level]+b; }
            else { l-=a;r-=b; }
        }
        return r-l;
    }
    /// @brief [l,r) の小さい方から0-indexedで k 番目。範囲外・空なら nullopt。
    std::optional<T> kth_smallest(int l,int r,int k) const {
        check_range(l,r);
        if(k<0 || k>=r-l) return std::nullopt;
        U key=0;
        for(int level=0;level<bits_;++level) {
            const int a=levels_[level].rank1(l),b=levels_[level].rank1(r),zero=(r-l)-(b-a);
            if(k<zero) { l-=a;r-=b; }
            else { k-=zero;key|=U{1}<<(bits_-level-1);l=zeros_[level]+a;r=zeros_[level]+b; }
        }
        return decode(key);
    }
    /// @brief [l,r) の大きい方から0-indexedで k 番目。範囲外なら nullopt。
    std::optional<T> kth_largest(int l,int r,int k) const {
        check_range(l,r);
        if(k<0 || k>=r-l) return std::nullopt;
        return kth_smallest(l,r,r-l-1-k);
    }
    /// @brief [l,r) 中の x 未満の個数。@pre 0<=l<=r<=size()。
    int count_less(int l,int r,T x) const {
        check_range(l,r);
        const U code=encode(x);
        if(l==r || code<=base_) return 0;
        if(code>maximum_) return r-l;
        const U key=code-base_;int result=0;
        for(int level=0;level<bits_;++level) {
            const int a=levels_[level].rank1(l),b=levels_[level].rank1(r);
            if((key>>(bits_-level-1))&U{1}) { result+=(r-l)-(b-a);l=zeros_[level]+a;r=zeros_[level]+b; }
            else { l-=a;r-=b; }
        }
        return result;
    }
    /// @brief [l,r) のうち値が [lower,upper) の個数。@pre lower<=upper。上端を型の最大値より大きくは指定できない。
    int range_freq(int l,int r,T lower,T upper) const {
        assert(lower<=upper);return count_less(l,r,upper)-count_less(l,r,lower);
    }
    /// @brief [l,r) に存在する x 未満の最大値。なければ nullopt。
    std::optional<T> prev_value(int l,int r,T x) const { return kth_smallest(l,r,count_less(l,r,x)-1); }
    /// @brief [l,r) に存在する x 以上の最小値。なければ nullopt。
    std::optional<T> next_value(int l,int r,T x) const { return kth_smallest(l,r,count_less(l,r,x)); }
};
