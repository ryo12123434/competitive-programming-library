#include <boost/multiprecision/cpp_int.hpp>
#include "library/math/int128_io.hpp"
#include "library/math/integer_division.hpp"
#include "library/math/integer_sqrt.hpp"
#include "library/math/safe_arithmetic.hpp"
#include "library/math/integer_digits.hpp"
#include "library/debug/debug.hpp"
#include <bit>
#include <cassert>
#include <climits>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using boost::multiprecision::cpp_int;
using I = __int128_t;
using U = __uint128_t;
constexpr U UMAX = ~U(0);
constexpr I IMAX = static_cast<I>(UMAX >> 1);
constexpr I IMIN = -IMAX - 1;

template <class T> struct UnsignedRef {
    using type = std::make_unsigned_t<T>;
};
template <> struct UnsignedRef<I> {
    using type = U;
};
template <> struct UnsignedRef<U> {
    using type = U;
};
template <class T> using UR = typename UnsignedRef<T>::type;
template <class T> cpp_int big(T value) {
    if constexpr (sizeof(T) <= 8) {
        if constexpr (std::numeric_limits<T>::is_signed)
            return cpp_int(static_cast<long long>(value));
        else
            return cpp_int(static_cast<unsigned long long>(value));
    } else {
        U bits = static_cast<U>(value);
        cpp_int result = static_cast<std::uint64_t>(bits >> 64);
        result <<= 64;
        result += static_cast<std::uint64_t>(bits);
        if constexpr (std::numeric_limits<T>::is_signed)
            if (value < 0)
                result -= cpp_int(1) << 128;
        return result;
    }
}
template <class T> T nonnegative(const cpp_int &value) {
    U bits = static_cast<std::uint64_t>((value & ((cpp_int(1) << 64) - 1)).convert_to<std::uint64_t>());
    if constexpr (sizeof(T) > 8)
        bits |= U((value >> 64).convert_to<std::uint64_t>()) << 64;
    return static_cast<T>(bits);
}
template <class T> T random_integer(std::mt19937_64 &rng) {
    UR<T> bits;
    if constexpr (sizeof(T) > 8)
        bits = (U(rng()) << 64) | rng();
    else
        bits = static_cast<UR<T>>(rng());
    return std::bit_cast<T>(bits);
}
template <class T> void expect(const std::optional<T> &result, const cpp_int &expected) {
    if (expected < big(std::numeric_limits<T>::lowest()) || expected > big(std::numeric_limits<T>::max()))
        assert(!result);
    else
        assert(result && big(*result) == expected);
}

template <class A, class B>
concept CanFloor = requires(A a, B b) { floor_div(a, b); };
template <class T>
concept CanWide = requires(T a) { wide_mul(a, a); };
static_assert(!CanFloor<bool, bool> && !CanFloor<long long, unsigned long long>);
static_assert(!CanWide<I> && !CanWide<U> && CanWide<std::uint64_t>);
static_assert(floor_div(-7, 3) == -3 && ceil_div(7, -3) == -2);
static_assert(!floor_div(IMIN, I(-1)) && !ceil_div(IMIN, I(0)));
static_assert(checked_add(5, 7) == 12 && !checked_add(IMAX, I(1)) && !checked_mul(UMAX, U(2)));
static_assert(isqrt(~std::uint64_t(0)) == 4294967295ULL && isqrt(UMAX) == U(18446744073709551615ULL));
static_assert(digit_count(IMIN) == 39 && digit_sum(-120) == 3);

void io() {
    std::vector<cpp_int> values{0,
                                1,
                                -1,
                                cpp_int(LLONG_MIN),
                                cpp_int(LLONG_MAX),
                                (cpp_int(1) << 64) - 1,
                                cpp_int(1) << 64,
                                (cpp_int(1) << 127) - 1,
                                -(cpp_int(1) << 127),
                                (cpp_int(1) << 128) - 1};
    for (const auto &value : values) {
        auto text = value.str();
        auto si = parse_int128(text);
        auto ui = parse_uint128(text);
        if (value >= big(IMIN) && value <= big(IMAX)) {
            assert(si && big(*si) == value && to_string_int128(*si) == text);
        } else
            assert(!si);
        if (value >= 0 && value <= big(UMAX)) {
            assert(ui && big(*ui) == value && to_string_uint128(*ui) == text);
        } else
            assert(!ui);
    }
    assert(to_string_int128(IMIN) == "-170141183460469231731687303715884105728");
    assert(to_string_int128(IMAX) == "170141183460469231731687303715884105727");
    assert(to_string_uint128(UMAX) == "340282366920938463463374607431768211455");
    std::vector<std::string> bad{"",     " ",   " 1",  "1 ",    "\t1",  "1\n",
                                 "+",    "-",   "+-1", "-+1",   "++1",  "--1",
                                 "0x10", "1.5", "1e5", "1_000", "１２", std::string("1\0", 2)};
    for (const auto &text : bad)
        assert(!parse_int128(text) && !parse_uint128(text));
    assert(!parse_int128("170141183460469231731687303715884105728"));
    assert(!parse_int128("-170141183460469231731687303715884105729"));
    assert(!parse_uint128("340282366920938463463374607431768211456"));
    assert(parse_int128("-000") == 0 && parse_int128("+000001") == 1 && parse_uint128("+000001") == 1 &&
           !parse_uint128("-0"));
    assert(parse_int128(std::string(5000, '0')) == 0);
    std::mt19937_64 rng(4821);
    for (int i = 0; i < 12000; ++i) {
        I x = random_integer<I>(rng);
        U y = random_integer<U>(rng);
        assert(to_string_int128(x) == big(x).str());
        assert(to_string_uint128(y) == big(y).str());
        assert(parse_int128(big(x).str()) == x && parse_uint128(big(y).str()) == y);
        std::ostringstream out;
        print_int128(out, x) << ' ';
        print_uint128(out, y);
        std::istringstream in(out.str());
        I a = 0;
        U b = 0;
        read_int128(in, a);
        read_uint128(in, b);
        assert(a == x && b == y && !in.fail());
    }
    std::ostringstream out;
    out << std::hex << std::showpos;
    auto flags = out.flags();
    print_int128(out, -42) << ' ' << 17;
    assert(out.str() == "-42 11" && out.flags() == flags);
    out.str("");
    out << std::setfill('.') << std::setw(7);
    print_int128(out, -42);
    assert(out.str() == "....-42");
    std::istringstream malformed("170141183460469231731687303715884105728 9");
    I x = 7;
    read_int128(malformed, x);
    assert(malformed.fail() && x == 7);
    malformed.clear();
    read_int128(malformed, x);
    assert(!malformed.fail() && x == 9);
    std::istringstream eof("0");
    read_int128(eof, x);
    assert(x == 0 && !eof.fail() && eof.eof());
    read_int128(eof, x);
    assert(eof.fail() && x == 0);
    std::istringstream no_skip(" 1");
    no_skip >> std::noskipws;
    read_int128(no_skip, x);
    assert(no_skip.fail() && x == 0);
    std::istringstream throws("bad");
    throws.exceptions(std::ios_base::failbit);
    bool caught = false;
    try {
        read_int128(throws, x);
    } catch (const std::ios_base::failure &) {
        caught = true;
    }
    assert(caught && x == 0);
    U y = 7;
    std::istringstream negative("-0");
    read_uint128(negative, y);
    assert(negative.fail() && y == 7);
}

template <class T> void arithmetic_pair(T a, T b) {
    cpp_int A = big(a), B = big(b);
    expect(checked_add(a, b), cpp_int(A + B));
    expect(checked_mul(a, b), cpp_int(A * B));
    if (B == 0) {
        assert(!floor_div(a, b) && !ceil_div(a, b));
        return;
    }
    cpp_int q = A / B, r = A - q * B, lo = q, hi = q;
    if (r != 0) {
        if ((A < 0) != (B < 0))
            --lo;
        else
            ++hi;
    }
    auto floor = floor_div(a, b), ceil = ceil_div(a, b);
    expect(floor, lo);
    expect(ceil, hi);
    if (floor) {
        cpp_int f = big(*floor);
        if (B > 0)
            assert(f * B <= A && A < (f + 1) * B);
        else
            assert((f + 1) * B < A && A <= f * B);
    }
    if constexpr (sizeof(T) <= 8)
        assert(big(wide_mul(a, b)) == A * B);
}
template <class T> void type_arithmetic(int random_count) {
    std::vector<T> edges{T(0),
                         T(1),
                         T(2),
                         std::numeric_limits<T>::max(),
                         T(std::numeric_limits<T>::max() - 1),
                         std::numeric_limits<T>::lowest()};
    if constexpr (std::numeric_limits<T>::is_signed) {
        edges.push_back(T(-1));
        edges.push_back(T(-2));
        edges.push_back(T(std::numeric_limits<T>::lowest() + 1));
    }
    for (T a : edges)
        for (T b : edges)
            arithmetic_pair(a, b);
    std::mt19937_64 rng(547 + sizeof(T));
    for (int i = 0; i < random_count; ++i) {
        T a = random_integer<T>(rng), b = random_integer<T>(rng);
        arithmetic_pair(a, b);
        if constexpr (sizeof(T) <= 8) {
            T c = random_integer<T>(rng), d = random_integer<T>(rng);
            cpp_int l = big(a) * big(b), r = big(c) * big(d);
            int expected = l < r ? -1 : l > r ? 1 : 0;
            assert(compare_products(a, b, c, d) == expected);
        }
    }
}
void arithmetic() {
    for (int a = -128; a < 128; ++a)
        for (int b = -128; b < 128; ++b)
            arithmetic_pair(static_cast<std::int8_t>(a), static_cast<std::int8_t>(b));
    for (int a = 0; a < 256; ++a)
        for (int b = 0; b < 256; ++b)
            arithmetic_pair(static_cast<std::uint8_t>(a), static_cast<std::uint8_t>(b));
    type_arithmetic<std::int16_t>(2000);
    type_arithmetic<std::uint16_t>(2000);
    type_arithmetic<std::int32_t>(5000);
    type_arithmetic<std::uint32_t>(5000);
    type_arithmetic<std::int64_t>(10000);
    type_arithmetic<std::uint64_t>(10000);
    type_arithmetic<I>(15000);
    type_arithmetic<U>(15000);
}

template <class T> void sqrt_value(T value) {
    cpp_int expected = boost::multiprecision::sqrt(big(value));
    auto actual = isqrt(value);
    assert(big(actual) == expected);
    cpp_int root = big(actual);
    assert(root * root <= big(value) && (root + 1) * (root + 1) > big(value));
}
void roots_digits() {
    for (unsigned i = 0; i <= 65535; ++i)
        sqrt_value(static_cast<std::uint16_t>(i));
    std::mt19937_64 rng(17252);
    for (int i = 0; i < 12000; ++i) {
        auto u = random_integer<U>(rng);
        auto v = random_integer<std::uint64_t>(rng);
        sqrt_value(u);
        sqrt_value(v);
        for (I x : {random_integer<I>(rng), IMIN, IMAX}) {
            auto text = big(x).str();
            auto digits = text.front() == '-' ? text.substr(1) : text;
            int sum = 0;
            for (char c : digits)
                sum += c - '0';
            assert(digit_count(x) == static_cast<int>(digits.size()) && digit_sum(x) == sum);
        }
        auto text = big(u).str();
        int sum = 0;
        for (char c : text)
            sum += c - '0';
        assert(digit_count(u) == static_cast<int>(text.size()) && digit_sum(u) == sum);
    }
    for (cpp_int root :
         std::vector<cpp_int>{0, 1, 2, 3, (cpp_int(1) << 16) - 1, cpp_int(1) << 16, (cpp_int(1) << 32) - 1,
                              cpp_int(1) << 32, cpp_int(1) << 63, (cpp_int(1) << 64) - 1})
        for (int delta : {-1, 0, 1}) {
            cpp_int x = root * root + delta;
            if (x >= 0 && x <= big(UMAX))
                sqrt_value(nonnegative<U>(x));
            if (x >= 0 && x <= big(std::numeric_limits<std::uint64_t>::max()))
                sqrt_value(nonnegative<std::uint64_t>(x));
            if (x >= 0 && x <= big(IMAX))
                sqrt_value(nonnegative<I>(x));
        }
    assert(digit_count(0) == 1 && digit_sum(0) == 0 && digit_count(LLONG_MIN) == 19 &&
           digit_count(UMAX) == 39);
}
void debug_display() {
    using cpl::debug::repr;
    cpp_int huge = cpp_int(1) << 500;
    std::string lo = big(IMIN).str(), hi = big(UMAX).str();
    assert(repr(IMIN) == lo && repr(UMAX) == hi && repr(huge) == huge.str());
    assert(repr(std::vector<I>{IMIN, 0, IMAX}) == "[" + lo + ", 0, " + big(IMAX).str() + "]");
    assert(repr(std::pair<I, I>{IMIN, IMAX}) == "(" + lo + ", " + big(IMAX).str() + ")");
    std::queue<std::pair<I, U>> q;
    q.push({IMIN, UMAX});
    assert(repr(q) == "queue[(" + lo + ", " + hi + ")]");
    std::priority_queue<I> heap;
    heap.push(IMIN);
    heap.push(IMAX);
    assert(repr(heap) == "priority_queue[" + big(IMAX).str() + ", " + lo + "]");
    std::vector<std::map<I, std::vector<cpp_int>>> nested{{{IMIN, {huge, -huge}}}};
    assert(repr(nested) == "[{(" + lo + ", [" + huge.str() + ", " + cpp_int(-huge).str() + "])}]");
    assert(repr(std::vector<I>{IMIN, IMAX}, {1, 8}) == "[" + lo + ", ...]");
    assert(repr(nested, {20, 1}) == "[...]");
    const std::vector<std::vector<I>> table{{IMIN, IMAX}};
    assert(cpl::debug::render_table(table).find(lo) != std::string::npos);
    std::ostringstream captured;
    auto *previous = std::cerr.rdbuf(captured.rdbuf());
    int evaluations = 0;
    debug(IMIN, UMAX, huge, ++evaluations);
    debug_table(table);
    std::cerr.rdbuf(previous);
#ifdef LOCAL
    assert(evaluations == 1 && captured.str().find(lo) != std::string::npos &&
           captured.str().find(hi) != std::string::npos &&
           captured.str().find(huge.str()) != std::string::npos);
#else
    assert(evaluations == 0 && captured.str().empty());
    debug(undefined_128());
    CPL_DEBUG(undefined_128());
#endif
}
int main(int argc, char **argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (mode == "all" || mode == "io")
        io();
    if (mode == "all" || mode == "arithmetic")
        arithmetic();
    if (mode == "all" || mode == "roots_digits")
        roots_digits();
    if (mode == "all" || mode == "debug")
        debug_display();
}
