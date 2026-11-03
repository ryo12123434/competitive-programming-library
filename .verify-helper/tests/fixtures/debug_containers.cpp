#include "library/debug/debug.hpp"

#include <array>
#include <cassert>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using cpl::debug::repr;

template <class Container>
void check_empty(const Container& value, const std::string& expected) {
    assert(repr(value) == expected);
    assert(repr(value, {0, 8}) == expected);
    assert(repr(value, {20, 0}) == "...");
}

void check_ranges() {
    assert(repr(std::vector<int>{1, 2, 3}) == "[1, 2, 3]");
    assert(repr(std::array<int, 3>{1, 2, 3}) == "[1, 2, 3]");
    assert(repr(std::deque<int>{1, 2, 3}) == "[1, 2, 3]");
    assert(repr(std::list<int>{1, 2, 3}) == "[1, 2, 3]");
    assert(repr(std::set<int>{3, 1, 2}) == "{1, 2, 3}");
    assert(repr(std::multiset<int>{3, 1, 1}) == "{1, 1, 3}");
    assert(repr(std::unordered_set<int>{2}) == "{2}");
    assert(repr(std::unordered_multiset<int>{2, 2}) == "{2, 2}");
    assert(repr(std::map<int, int>{{2, 20}, {1, 10}}) == "{(1, 10), (2, 20)}");
    assert(repr(std::multimap<int, int>{{1, 10}, {1, 20}}) == "{(1, 10), (1, 20)}");
    assert(repr(std::unordered_map<int, int>{{1, 10}}) == "{(1, 10)}");
    assert(repr(std::unordered_multimap<int, int>{{1, 10}, {1, 10}}) == "{(1, 10), (1, 10)}");
    assert(repr(std::pair{1, 2}) == "(1, 2)");
    assert(repr(std::tuple{1, true, 'x', std::string("str")}) == "(1, true, 'x', \"str\")");
    check_empty(std::vector<int>{}, "[]");
    check_empty(std::array<int, 0>{}, "[]");
    check_empty(std::deque<int>{}, "[]");
    check_empty(std::list<int>{}, "[]");
    check_empty(std::set<int>{}, "{}");
    check_empty(std::multiset<int>{}, "{}");
    check_empty(std::unordered_set<int>{}, "{}");
    check_empty(std::unordered_multiset<int>{}, "{}");
    check_empty(std::map<int, int>{}, "{}");
    check_empty(std::multimap<int, int>{}, "{}");
    check_empty(std::unordered_map<int, int>{}, "{}");
    check_empty(std::unordered_multimap<int, int>{}, "{}");
    check_empty(std::tuple<>{}, "()");
}

struct Compare {
    bool min_first;
    bool operator()(int a, int b) const { return min_first ? a > b : a < b; }
};

void check_adapters() {
    std::queue<int> queue;
    std::stack<int> stack;
    std::priority_queue<int> heap;
    check_empty(queue, "queue[]");
    check_empty(stack, "stack[]");
    check_empty(heap, "priority_queue[]");
    for (int x : {1, 3, 2}) { queue.push(x); stack.push(x); heap.push(x); }
    assert(repr(queue) == "queue[1, 3, 2]");
    assert(repr(stack) == "stack[2, 3, 1]");
    assert(repr(heap) == "priority_queue[3, 2, 1]");
    assert(queue.size() == 3 && queue.front() == 1 && queue.back() == 2);
    assert(stack.size() == 3 && stack.top() == 2);
    assert(heap.size() == 3 && heap.top() == 3);
    for (int x : {1, 3, 2}) { assert(queue.front() == x); queue.pop(); }
    for (int x : {2, 3, 1}) { assert(stack.top() == x); stack.pop(); }
    for (int x : {3, 2, 1}) { assert(heap.top() == x); heap.pop(); }
    assert(queue.empty() && stack.empty() && heap.empty());

    std::queue<int, std::list<int>> list_queue({4, 1, 4});
    std::stack<int, std::vector<int>> vector_stack({4, 1, 4});
    std::stack<int, std::list<int>> list_stack({4, 1, 2});
    std::priority_queue<int, std::deque<int>, std::greater<int>> min_heap;
    std::priority_queue<int, std::vector<int>, Compare> stateful_heap(Compare{true});
    for (int x : {5, 1, 3, 3}) { min_heap.push(x); stateful_heap.push(x); }
    assert(repr(list_queue) == "queue[4, 1, 4]");
    assert(repr(vector_stack) == "stack[4, 1, 4]");
    assert(repr(list_stack) == "stack[2, 1, 4]");
    assert(repr(min_heap) == "priority_queue[1, 3, 3, 5]");
    assert(repr(stateful_heap) == "priority_queue[1, 3, 3, 5]");
    assert(min_heap.size() == 4 && stateful_heap.size() == 4);
    assert(min_heap.top() == 1 && stateful_heap.top() == 1);
}

void check_nesting() {
    std::vector<std::set<int>> a{{1, 2}, {3, 4}};
    std::vector<std::vector<std::set<int>>> b{a, {}};
    std::map<int, std::vector<int>> c{{1, {2, 3}}};
    std::priority_queue<int> heap;
    for (int x : {1, 5, 3}) heap.push(x);
    std::vector<std::priority_queue<int>> d{heap, {}};
    std::queue<std::pair<int, int>> e;
    e.push({1, 2}); e.push({3, 4});
    std::set<std::pair<int, int>> f{{3, 4}, {1, 2}};
    std::stack<int> stack;
    stack.push(1); stack.push(2);
    std::vector<std::stack<int>> g{stack, {}};
    assert(repr(a) == "[{1, 2}, {3, 4}]");
    assert(repr(b) == "[[{1, 2}, {3, 4}], []]");
    assert(repr(c) == "{(1, [2, 3])}");
    assert(repr(d) == "[priority_queue[5, 3, 1], priority_queue[]]");
    assert(repr(e) == "queue[(1, 2), (3, 4)]");
    assert(repr(f) == "{(1, 2), (3, 4)}");
    assert(repr(g) == "[stack[2, 1], stack[]]");
    std::queue<std::vector<std::set<int>>> nested;
    nested.push(a);
    assert(repr(nested) == "queue[[{1, 2}, {3, 4}]]");
    std::stack<decltype(nested)> nested_stack;
    nested_stack.push(nested);
    assert(repr(std::tuple{nested_stack, std::pair{true, a}}) ==
           "(stack[queue[[{1, 2}, {3, 4}]]], (true, [{1, 2}, {3, 4}]))");

    const std::vector<std::vector<std::vector<std::set<int>>>> table{{a}};
    assert(cpl::debug::render_table(table).find("[{1, 2}, {3, 4}]") != std::string::npos);
    const std::vector<std::vector<std::priority_queue<int>>> heaps{{heap}};
    assert(cpl::debug::render_table(heaps).find("priority_queue[5, 3, 1]") != std::string::npos);

    const auto previous_options = cpl::debug::print_options();
    cpl::debug::print_options() = {1, 8};
    assert(cpl::debug::render_table(heaps).find("priority_queue[5, ...]") != std::string::npos);
    cpl::debug::print_options() = previous_options;

    std::ostringstream captured;
    auto* previous = std::cerr.rdbuf(captured.rdbuf());
    int evaluations = 0;
    debug(a, b, c, d, e, f, g, ++evaluations);
    CPL_DEBUG(nested_stack, ++evaluations);
    debug_table(heaps);
    CPL_DEBUG_TABLE(table);
    std::cerr.rdbuf(previous);
#ifdef LOCAL
    assert(evaluations == 2);
    const auto output = captured.str();
    assert(output.find(std::string(__FILE__) + ":") != std::string::npos);
    assert(output.find("a, b, c, d, e, f, g, ++evaluations") != std::string::npos);
    assert(output.find(repr(b)) != std::string::npos);
    assert(output.find(repr(d)) != std::string::npos);
    assert(output.find(repr(e)) != std::string::npos);
    assert(output.find(repr(g)) != std::string::npos);
    assert(output.find(repr(nested_stack)) != std::string::npos);
    assert(output.find("r/c") != std::string::npos);
#else
    assert(evaluations == 0 && captured.str().empty());
    debug(undefined_container());
    CPL_DEBUG(undefined_container());
    debug_table(undefined_table());
    CPL_DEBUG_TABLE(undefined_table());
#endif
    assert(heap.size() == 3 && heap.top() == 5);
    assert(e.size() == 2 && e.front() == std::pair(1, 2));
    assert(stack.size() == 2 && stack.top() == 2);
}

template <class Adapter>
void check_adapter_limits(Adapter value, const std::string& name) {
    for (int x : {1, 1, 1}) value.push(x);
    assert(repr(value, {0, 8}) == name + "[...]");
    assert(repr(value, {1, 8}) == name + "[1, ...]");
    assert(repr(value, {2, 8}) == name + "[1, 1, ...]");
    assert(repr(value, {3, 8}) == name + "[1, 1, 1]");
    assert(repr(value, {4, 8}) == name + "[1, 1, 1]");
    assert(repr(value, {20, 0}) == "...");
    assert(repr(value, {20, 1}) == name + "[1, 1, 1]");
    assert(repr(std::vector{value}, {20, 1}) == "[...]");
    assert(repr(std::vector{value}, {20, 2}) == "[" + name + "[1, 1, 1]]");
    assert(value.size() == 3);
}

void check_limits() {
    check_adapter_limits(std::queue<int>{}, "queue");
    check_adapter_limits(std::stack<int>{}, "stack");
    check_adapter_limits(std::priority_queue<int>{}, "priority_queue");
    std::queue<std::vector<std::set<int>>> nested;
    nested.push({{1, 2}, {3}});
    assert(repr(nested, {20, 1}) == "queue[...]");
    assert(repr(nested, {20, 2}) == "queue[[..., ...]]");
    assert(repr(nested, {20, 3}) == "queue[[{1, 2}, {3}]]");
    assert(repr(nested, {1, 8}) == "queue[[{1, ...}, ...]]");
    assert(repr(std::map<int, std::vector<int>>{{1, {2}}}, {20, 1}) == "{...}");
    assert(repr(std::set<int>{1, 2}, {1, 8}) == "{1, ...}");
    assert(repr(std::tuple{1, 2}, {0, 8}) == "(...)");
}

int main() {
    check_ranges();
    check_adapters();
    check_nesting();
    check_limits();
}
