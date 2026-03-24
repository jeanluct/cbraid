# Issue #3 Deep-Dive Plan (USS Segfault / Delta-Power Edge Cases)

## Goal

Resolve the crash reported in `jeanluct/cbraid#3` while preserving mathematical correctness of USS/centralizer-related outputs (not just preventing a segfault).

## What issue #3 says (cbraid)

- Reproducer from issue comments (owner):
  - Option `u` (Ultra Summit Set), `n = 6`
  - Word: `2 1 3 2 1 4 3 2 5 4 3 2 1 1 4`
  - Run as `./braiding < bug.in`
- Valgrind trace in comments points to:
  - `CBraid::Factor::At` / `Flip`
  - called from `Braiding::Returns(...)` near `braiding.cpp:1000`
- Diagnostic note in comments:
  - During failing path, braid prints as `(1|0)` (interpreted as Delta-power with canonical length 0)
  - `FactorList` is empty, but code dereferences `*B1.FactorList.begin()`
- Historical note from Bert (quoted in issue):
  - Similar crash behavior on related examples
  - Older tool variant did not crash but produced suspicious USS output (`D` in multiple orbits), suggesting semantic sensitivity around this case.

## Related evidence from SageMath issue `sagemath/sage#35529`

- Symptom: `b.centralizer()` may segfault / memory-corrupt depending on platform/build.
- Multiple comments identify upstream `libbraiding` as source.
- Miguel Marco links likely crash site in `braiding.cpp` and connects it explicitly to cbraid issue #3.
- Key hypothesis repeated there:
  - Algorithm assumes a non-empty factor list.
  - Some intermediate braids are powers of Delta "in disguise" (empty factor list with nonzero `LeftDelta`).
- Important correctness signal from comments:
  - A patch that stopped crashing (linked there) was reported to return an **incorrect centralizer** (too small; misses known generator like `s0*s2`).
  - Therefore, crash-fix-only patches are insufficient unless validated for algebraic correctness.

## Working interpretation

The segfault is a memory-safety manifestation of a broader representation invariant mismatch:

- Code in USS/transport/returns paths sometimes treats "first simple factor" as always existing.
- For braids represented as pure Delta powers, canonical length is zero and `FactorList` is empty.
- In those states, "first factor" must be interpreted via Delta context (or algorithm branch) rather than list dereference.

## Risks called out by tracker history

- A minimal null/empty guard can remove crashes but alter group-theoretic meaning.
- Centralizer output is a strong downstream correctness test; regressions may be subtle.
- Platform/compiler variability can hide UB, so deterministic local non-crash is not enough.

## Implementation plan

### Phase 1: Reproduce and baseline

1. Build with debug symbols and no optimization (`DEBUG=1`) in both `lib` and `programs`.
2. Reproduce with `./braiding < programs/bug.in`.
3. Confirm crash stack and line numbers under valgrind.
4. Save baseline output (`foo.txt`) for later semantic comparison.

### Phase 2: Audit unsafe dereferences in the relevant pipeline

Target functions first (based on both issue threads):

- `Returns(...)`
- `Transport(...)`
- callers in USS/minUSS and centralizer flow where first-factor assumptions appear

For each site:

1. Identify assumptions about non-empty `FactorList`.
2. Classify expected mathematical branch when braid has CL=0 / is Delta-power.
3. Replace direct `begin()` dereference with representation-aware helper logic.

### Phase 3: Introduce one canonical helper for "head factor"

Add an internal helper in `lib/braiding.cpp` (file-local):

- Input: braid `B`
- Behavior:
  - if `B.FactorList` non-empty: return first factor
  - else if `B` is Delta/identity form: return factor consistent with existing code conventions for this algorithmic context
- Document intended semantics in code comments very briefly (why CL=0 is special).

Rationale:

- Prevents repeated ad-hoc conditionals.
- Makes subsequent audit consistent and reviewable.

### Phase 4: Correctness-focused validation (not only crash-free)

Use two classes of checks:

1. **Crash/UB checks**
   - `./braiding < programs/bug.in` no crash
   - valgrind shows no invalid read/write in affected path
2. **Algebraic sanity checks**
   - For reproducer and at least one related braid from comments, verify centralizer output includes the braid itself (identity requirement from comments).
   - Compare results against expected known elements where available (e.g., comments mentioning missing `s0*s2` in Sage case).
   - Confirm outputs are stable across repeated runs.

#### Explicit non-crash/wrong-answer test from SageMath thread

Use the concrete example from `sagemath/sage#35529` as an oracle check for correctness, not just stability:

CLI input that can be fed directly to `braiding`:

```
z
5
3 3 4 3 3 2 1 4 3 2
sage35529.out
```

(Saved in this repo as `programs/sage35529.in`, run with `./braiding < sage35529.in` from `programs/`.)

```python
BG = BraidGroup(5)
b = BG([3, 3, 4, 3, 3, 2, 1, 4, 3, 2])
x = BG([1, 3])          # this is s0*s2

# sanity: x is known to commute with b
assert x*b == b*x

C = b.centralizer()      # calls libbraiding-backed centralizer

# centralizer output must consist of commuting elements
assert all(c*b == b*c for c in C)

# stronger check: known commuting element must lie in subgroup generated by C
H = BG.subgroup(C)
assert x in H

# and b itself must lie in its own centralizer subgroup
assert b in H
```

Pass criteria for this test:

- no segfault/crash,
- all returned generators commute with `b`, and
- subgroup generated by returned generators contains both `x = s0*s2` and `b`.

For the pure `braiding` CLI run, treat this as a two-stage check:

1. `./braiding < sage35529.in` must complete and produce `sage35529.out`.
2. The generators printed in `sage35529.out` must then be validated externally (e.g., Sage script above) to ensure they generate a centralizer containing `s0*s2` and `b`.

This is the specific guard against the "doesn't crash but returns too-small centralizer" failure mode documented in both issue threads.

### Phase 5: Regression guardrails

Add lightweight reproducible inputs under `programs/` and document expected high-level properties:

- USS reproducer from issue #3
- at least one 5-strand related example from issue discussion

If adding formal tests is too invasive for current build system, keep scripted/manual check commands in notes for CI follow-up.

### Phase 6: Finalization

1. Re-run full local build.
2. Re-run existing test program(s) to catch unrelated breakage.
3. Summarize in commit/PR:
   - root cause (Delta-power empty factor list)
   - why the fix is representation-aware
   - what correctness checks were used beyond "no segfault"

## Acceptance criteria

- Reproducer from `jeanluct/cbraid#3` no longer crashes.
- No invalid memory access in valgrind for that run.
- Centralizer/USS outputs for reproducer and related sample satisfy stated algebraic sanity checks.
- No obvious regressions in existing sample/test binaries.

## Notes for this branch

- Current branch name `iss033-ultra-summit-set-segfaults` is legacy/misnumbered relative to GitHub issue `#3`.
- Renaming can be done later; correctness work should proceed independently.
