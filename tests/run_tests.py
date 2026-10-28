"""Compare solver results with exhaustive enumeration on small graph fixtures."""

import itertools
from pathlib import Path
import re
import subprocess
import sys


def expected_results(data):
    values = list(map(int, data.split()))
    n, m, k = values[:3]
    if not 1 <= n <= 8 or len(values) != 3 + 2 * m:
        raise ValueError("Fixtures must contain valid graphs with at most 8 vertices")
    neighbours = {u: set() for u in range(1, n + 1)}
    for u, v in zip(values[3::2], values[4::2]):
        neighbours[u].add(v)
        neighbours[v].add(u)

    valid = []
    for size in range(k + 1, n + 1):
        for combination in itertools.combinations(neighbours, size):
            nodes = frozenset(combination)
            if any(len(neighbours[u] & nodes) < k for u in nodes):
                continue
            if any(
                v not in neighbours[u] and not (neighbours[u] & neighbours[v] & nodes)
                for u, v in itertools.combinations(nodes, 2)
            ):
                continue
            valid.append(nodes)
    return k, {nodes for nodes in valid if not any(nodes < other for other in valid)}


def actual_results(output, k):
    results = set()
    prefix = f"Maximal {k}-core with 2-diameter is:"
    for line in output.splitlines():
        if not line.startswith(prefix):
            continue
        ids = line[len(prefix):].split()
        if not ids or any(re.fullmatch(r"[1-9][0-9]*", value) is None for value in ids):
            raise AssertionError(f"Invalid result line: {line!r}")
        nodes = frozenset(map(int, ids))
        if len(nodes) != len(ids):
            raise AssertionError(f"Repeated vertex in result: {line!r}")
        results.add(nodes)
    return results


def describe(results):
    return sorted(sorted(nodes) for nodes in results)


def main():
    if len(sys.argv) != 3:
        print("Usage: run_tests.py MAIN_BINARY GREEDY_BINARY", file=sys.stderr)
        return 2

    fixtures = sorted((Path(__file__).parent / "cases").glob("*.in"))
    if not fixtures:
        raise RuntimeError("No graph fixtures found")
    failures = 0
    for binary in map(Path, sys.argv[1:]):
        for fixture in fixtures:
            try:
                data = fixture.read_text()
                k, expected = expected_results(data)
                run = subprocess.run(
                    [str(binary.resolve())],
                    input=data,
                    text=True,
                    capture_output=True,
                    timeout=5,
                    check=True,
                )
                actual = actual_results(run.stdout, k)
                if actual != expected:
                    raise AssertionError(
                        f"expected {describe(expected)}, got {describe(actual)}"
                    )
                print(f"PASS {binary.name}: {fixture.stem}")
            except (AssertionError, ValueError, OSError, subprocess.SubprocessError) as error:
                print(f"FAIL {binary.name}: {fixture.stem}: {error}", file=sys.stderr)
                failures += 1

    print(f"{len(fixtures) * 2 - failures}/{len(fixtures) * 2} cases passed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
