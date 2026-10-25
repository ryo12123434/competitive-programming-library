#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"

#include <cassert>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "../../library/data_structure/li_chao_tree.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, q;
    std::cin >> n >> q;
    std::vector<std::pair<long long, long long>> lines(n);
    for (auto& [a, b] : lines) std::cin >> a >> b;
    struct Query { int type; long long a, b; };
    std::vector<Query> queries(q);
    std::vector<long long> xs;
    for (auto& query : queries) {
        std::cin >> query.type >> query.a;
        query.b = 0;
        if (query.type == 0) std::cin >> query.b;
        else xs.push_back(query.a);
    }
    LiChaoTree tree(xs);
    for (auto [a, b] : lines) tree.add_line(a, b);
    for (auto query : queries) {
        if (query.type == 0) tree.add_line(query.a, query.b);
        else {
            const auto answer = tree.query(query.a);
            // この問題の制約では答えが long long に収まる。明示的に検査してから変換する。
            assert(answer && std::numeric_limits<long long>::min() <= *answer &&
                   *answer <= std::numeric_limits<long long>::max());
            std::cout << static_cast<long long>(*answer) << '\n';
        }
    }
}
