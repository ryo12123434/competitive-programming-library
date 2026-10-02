# Snippet Migration

Use `migration/snippets.cpp` as reference and follow `AGENTS.md`.

## Add

- Dijkstra -> `library/graph/dijkstra.hpp`
- Bellman-Ford -> `library/graph/bellman_ford.hpp`
- Topological Sort -> `library/graph/topological_sort.hpp`
- Trie -> `library/string/trie.hpp`
- Rolling Hash -> `library/string/rolling_hash.hpp`
- LIS -> `library/algorithm/lis.hpp`
- Count Inversions -> `library/algorithm/inversion_number.hpp`
- 2D Imos -> `library/data_structure/imos_2d.hpp`

Creating `library/algorithm/` is allowed.

## Enhance existing headers

### UnionFind
Keep the existing API. Add per-component edge counts and `getedges(x)`.
Each `unite(x, y)` represents one added edge: if already connected, increment that component's edge count and return `false`; otherwise merge with `edges(rx) + edges(ry) + 1`.

### IntervalSet
Keep the current sentinel-free implementation and existing `insert`/`erase` behavior.
Add `covered_length`, `intersects(l, r)`, `contains(x)`, and `mex(x = 0)`. Keep covered length correct across merges, splits, adjacency, overlaps, and empty ranges.

## Notes

Remove external-global dependencies such as `INF`. Fix header-specific issues such as missing includes and RollingHash ODR problems, but do not add unrelated features.

Add suitable verification programs and run `oj-verify all`. Leave the files under `migration/` untouched.
