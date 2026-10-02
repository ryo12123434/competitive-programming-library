# AGENTS.md

- Target AtCoder GNU++23. ACL is allowed; do not depend on unavailable external libraries.
- Put reusable headers under the appropriate `library/` category, normally one library per header. Add a category only when needed.
- Prefer 0-indexing, half-open ranges `[l, r)`, and existing API/naming conventions. Keep implementations simple for contest use.
- Document public APIs with concise Doxygen comments; include preconditions, pitfalls, overflow, and complexity when relevant.
- Write comments and Doxygen documentation in Japanese. Keep code, API names, and identifiers in English.
- Put verification code under the matching `verify/` category with the problem URL when a suitable judge problem exists. Compile/test affected code.
- Do not make unrelated changes or break existing APIs without approval. Do not commit or push unless explicitly requested.
