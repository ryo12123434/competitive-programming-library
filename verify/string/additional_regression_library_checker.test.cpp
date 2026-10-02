#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// RLE の復元とバイト列、RollingHash の積を 128 bit の直接剰余と比較する。
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include "../../library/string/run_length_encoding.hpp"
#include "../../library/string/rolling_hash.hpp"

int main() {
    std::mt19937_64 rng(1713);
    assert(RLE("").empty());
    for (int trial = 0; trial < 1000; ++trial) {
        std::string s(rng() % 100, '\0');
        for (char& c : s) c = static_cast<char>(rng() % 256);
        auto encoded = RLE(s);
        std::string restored;
        for (int i = 0; i < static_cast<int>(encoded.size()); ++i) {
            auto [c, count] = encoded[i];
            assert(count > 0 && (i == 0 || encoded[i - 1].first != c));
            restored.append(count, c);
        }
        assert(restored == s);
        auto a = rng() % RollingHash::mod, b = rng() % RollingHash::mod;
        assert(RollingHash::mul(a, b) == static_cast<std::uint64_t>(static_cast<__uint128_t>(a) * b % RollingHash::mod));
    }
    assert(RLE(std::string(10000, '\0')) == (std::vector<std::pair<char, int>>{{'\0', 10000}}));
    assert(RollingHash::mul(RollingHash::mod - 1, RollingHash::mod - 1) == 1);
    long long a, b;
    if (std::cin >> a >> b) std::cout << a + b << '\n';
}
