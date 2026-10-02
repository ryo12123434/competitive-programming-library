# Snippet Migration Task

`migration/snippets.cpp` contains reference implementations copied from the existing VS Code snippets.
Use them as source material, but follow the repository's current `AGENTS.md` and existing library conventions.

## Goals

Migrate the following eight snippets into reusable library headers:

- `Dijkstra` -> `library/graph/dijkstra.hpp`
- `Bellman-Ford Algorithm` -> `library/graph/bellman_ford.hpp`
- `Topological Sort (Kahn's Algorithm)` -> `library/graph/topological_sort.hpp`
- `Trie Tree` -> `library/string/trie.hpp`
- `Rolling Hash (2^61-1)` -> `library/string/rolling_hash.hpp`
- `最長増加部分列 (LIS)` -> `library/algorithm/lis.hpp`
- `Count Inversions (BIT / ACL)` -> `library/algorithm/inversion_number.hpp`
- `2D Imos Method (Half-open)` -> `library/data_structure/imos_2d.hpp`

Creating `library/algorithm/` is allowed for the two general algorithms above.

Also enhance these two existing libraries instead of creating duplicates:

### `library/data_structure/union_find.hpp`

Preserve the existing public API and add the useful functionality from the snippet:

- maintain the number of edges in each connected component
- when `unite(x, y)` is called for vertices already in the same component, count that call as one additional edge and continue returning `false`
- when two components are merged, the merged edge count is `edges(rx) + edges(ry) + 1`
- add `getedges(int x)` to return the edge count of the component containing `x`
- keep `root`, `issame`, `unite`, and `getsize` compatible with the current header

Update Doxygen comments and complexity documentation accordingly.

### `library/data_structure/interval_set.hpp`

Preserve the current sentinel-free representation and existing `insert` / `erase` behavior. Do not replace it wholesale with the older snippet implementation. Add the useful capabilities represented by the snippet:

- track total covered length as `covered_length`
- `intersects(l, r)` for half-open interval `[l, r)`
- `contains(x)`
- `mex(x = 0)`

Keep intervals half-open and continue merging adjacent intervals as the current implementation does. Ensure `covered_length` stays correct through all merge, split, insert, erase, overlap, adjacency, and empty-interval cases. Do not add debug/printing APIs unless they are needed by the library design.

## Implementation rules

- C++23 / AtCoder GNU++23.
- ACL may be used.
- Follow `AGENTS.md`, especially Doxygen style, 0-indexing, half-open ranges, and non-breaking existing APIs.
- Remove dependencies on external globals such as `INF`; define necessary constants inside the library or derive them safely from the type.
- Keep the original snippets' intended behavior, but fix obvious header-specific issues such as ODR problems (`RollingHash` static data), missing includes, and external namespace assumptions.
- Do not add speculative features that are not present in the snippets or required for a clean reusable API.
- Prefer `std::` qualification and minimal includes, consistent with the newer headers in this repository.

## Verification

- Add verification programs under the matching `verify/` category whenever a suitable online judge problem exists.
- Preserve and rerun existing verification for `UnionFind` and any other affected library.
- For enhanced behavior that lacks a suitable online judge task, add focused local/compile-time test coverage if practical and state clearly what remains not externally verified.
- Compile with GNU++23 and run `oj-verify all`.
- Do not claim completion if verification fails.

## Scope exclusions

Do not migrate or modify these snippets as part of this task:

- DFS Lambda Template (Backtracking)
- FPS Library (ACL)
- Binomial Coefficient (ACL)

The FPS and combination functionality already exists in the repository, and the DFS lambda is intentionally kept as a snippet template.

## Cleanup

After all migrations and verification succeed, delete `migration/snippets.cpp` and this task file only if the user explicitly asks for cleanup. Do not commit or push unless explicitly requested by the user.
