#define PROBLEM "https://judge.yosupo.jp/problem/aplusb"

// A+B は verify-helper から回帰テストを起動するための入出力だけに使う。
// Functional Graph 等の正しさは、下の全探索・独立参照実装との比較で検証する。
#define CPL_TASK_A_NO_MAIN
#include "../../.verify-helper/tests/fixtures/task_a_regression.cpp"
#include <iostream>

int main() {
    run_task_a_regressions();
    long long a, b;
    std::cin >> a >> b;
    std::cout << a + b << '\n';
}
