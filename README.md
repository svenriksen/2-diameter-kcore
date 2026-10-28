# 2-Diameter k-Core

Experimental C++ implementations for finding maximal induced subgraphs with minimum degree at least `k` and diameter at most two. Every vertex must have at least `k` neighbours within the subgraph, and every pair of vertices must be at most two hops apart.

- `main.cpp`: searches by removing vertices from the graph's maximal k-core.
- `main-greedy.cpp`: searches by adding vertices in degree order.

## Build and run

Requires GNU Make, `g++` with C++17 support, and a POSIX shell. Tests also need Python 3.

```sh
make cpp   # Compile main.cpp, read inp.txt, write out.txt, remove the executable
make test  # Build both implementations and run regression cases
make clean # Remove generated executables and output
```

Builds use `-Wall -Werror -std=c++17`. Tests compare results against exhaustive enumeration on eight small graphs, covering cliques, a four-cycle, a star, k-core peeling, separate components, and an empty k-core. GitHub Actions runs the same checks on pull requests and pushes to `main`.

## Input

The first line contains `n m k` (vertices, edges, minimum degree), followed by `m` lines containing an undirected edge `u v`. Use vertex IDs `1` through `n`, with `1 <= n < 1000` and `1 <= k < n`. Inputs must have no self-loops or duplicate edges.

Example `inp.txt` (included):

```text
3 3 2
1 2
2 3
1 3
```

Both programs report the triangle `{1, 2, 3}` as a maximal 2-core with diameter at most two. Results go to standard output; progress messages go to standard error.

These are experimental search implementations: pruning can miss valid subgraphs, including disconnected-component cases in `main.cpp` and some vertex-expansion cases in `main-greedy.cpp`.

## License

[GNU GPL v3](LICENSE).
