#pragma once

#include <vector>
#include <utility>

struct LinearSieve {
    std::vector<int> lpf;
    std::vector<int> primes;

    // 1..N の最小素因数と素数一覧を O(N) で前計算する。
    // メモリ: O(N)
    explicit LinearSieve(int n) : lpf(n + 1, 0) {
        for (int i = 2; i <= n; ++i) {
            if (lpf[i] == 0) {
                lpf[i] = i;
                primes.push_back(i);
            }

            for (int p : primes) {
                if (p > lpf[i] || 1LL * i * p > n) break;
                lpf[i * p] = p;
            }
        }
    }

    // x を素因数分解し、(素因数, 指数) の列を返す。
    // 前提: 1 <= x < lpf.size()
    std::vector<std::pair<int, int>> factorize(int x) const {
        std::vector<std::pair<int, int>> res;

        while (x > 1) {
            int p = lpf[x];
            int cnt = 0;

            while (x % p == 0) {
                x /= p;
                ++cnt;
            }

            res.emplace_back(p, cnt);
        }

        return res;
    }
};
