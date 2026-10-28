# 2-Diameter k-Core

Experimental C++ implementations for finding maximal induced subgraphs with minimum degree at least `k` and diameter at most two. Every vertex must have at least `k` neighbours within the subgraph, and every pair of vertices must be at most two hops apart.

- `main.cpp`: searches by removing vertices from the graph's maximal k-core.
- `main-greedy.cpp`: searches by adding vertices in degree order.

## Build and run

Requires a C++17 compiler. Build each implementation separately:

```sh
g++ -std=c++17 -O2 main.cpp -o kcore
g++ -std=c++17 -O2 main-greedy.cpp -o kcore-greedy
./kcore < graph.txt
# Or: ./kcore-greedy < graph.txt
```

## Input

The first line contains `n m k` (vertices, edges, minimum degree), followed by `m` lines containing an undirected edge `u v`. Use vertex IDs `1` through `n`, with `1 <= n < 1000` and `1 <= k < n`. Inputs must have no self-loops or duplicate edges.

Example `graph.txt`:

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
