#define PROBLEM "https://atcoder.jp/contests/abc293/tasks/abc293_e"
#include "../../library/math/linear_recurrence.hpp"
#include <atcoder/modint>
#include <iostream>
int main() {
    std::uint64_t a, n;
    int m;
    std::cin >> a >> n >> m;
    using M = atcoder::dynamic_modint<451>;
    M::set_mod(m);
    M x = a;
    std::cout << linear_recurrence(std::vector<M>{0, 1}, std::vector<M>{x + 1, -x}, n).val() << "\n";
}
