#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

namespace cpl::debug {
/**
 * @brief steady_clock によるストップウォッチ。LOCAL 無効時は時計を読まず 0 を返す。
 * @note LOCAL の定義は全翻訳単位で統一すること。
 * @code
 * #include "library/debug/timer.hpp"
 * int main() {
 *     cpl::debug::Timer timer;
 *     timer.reset();
 *     [[maybe_unused]] double ms = timer.elapsed_ms();
 *     CPL_SCOPED_TIMER("solve");
 * }
 * @endcode
 */
class Timer {
#ifdef LOCAL
    std::chrono::steady_clock::time_point start_;
#endif
public:
    /// @brief 計測を開始する。O(1)。
    Timer() { reset(); }
    /// @brief 計測の開始時刻を現在に更新する。O(1)。
    void reset() {
#ifdef LOCAL
        start_ = std::chrono::steady_clock::now();
#endif
    }
    /// @brief 経過時間をミリ秒で返す。LOCAL 無効時は 0。O(1)。
    double elapsed_ms() const {
#ifdef LOCAL
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_).count();
#else
        return 0.0;
#endif
    }
};

/**
 * @brief 破棄時にスコープの経過時間を cerr に出す。通常は CPL_SCOPED_TIMER を使う。
 * @note ラベルはコピーして保持する。コピー・ムーブは不可。
 * @code
 * #include "library/debug/timer.hpp"
 * int main() {
 *     cpl::debug::ScopeTimer timer("initialization");
 * }
 * @endcode
 */
class ScopeTimer {
#ifdef LOCAL
    std::string label_;
    const char* file_;
    int line_;
    Timer timer_;
#endif
public:
    /// @brief スコープの計測を開始する。label は表示名、file / line は表示元情報。
    explicit ScopeTimer([[maybe_unused]] std::string_view label, [[maybe_unused]] const char* file = "",
                        [[maybe_unused]] int line = 0)
#ifdef LOCAL
        : label_(label), file_(file), line_(line)
#endif
    {}
    /// @brief LOCAL が有効なら計測結果を出力する。
    ~ScopeTimer() {
#ifdef LOCAL
        if (file_ && *file_) std::cerr << file_ << ':' << line_ << " | ";
        std::cerr << label_ << ": " << timer_.elapsed_ms() << " ms\n";
#endif
    }
    ScopeTimer(const ScopeTimer&) = delete;
    ScopeTimer& operator=(const ScopeTimer&) = delete;
};
} // namespace cpl::debug

#define CPL_DEBUG_DETAIL_JOIN_INNER(a, b) a##b
#define CPL_DEBUG_DETAIL_JOIN(a, b) CPL_DEBUG_DETAIL_JOIN_INNER(a, b)
#ifdef LOCAL
#define CPL_SCOPED_TIMER(label) ::cpl::debug::ScopeTimer CPL_DEBUG_DETAIL_JOIN(cpl_scope_timer_, __COUNTER__)((label), __FILE__, __LINE__)
#else
#define CPL_SCOPED_TIMER(...) ((void)0)
#endif
