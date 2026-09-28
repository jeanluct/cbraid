# Handoff: cbraid issue #3 fix introduces a hang in `AreConjugate`

> **Resolved** on branch `iss003-conjugacy-hang`.  `FirstFactorInLCF`
> returned Δ whenever `LeftDelta > 0`, even when `FactorList` was not
> empty, out of step with `Cycling()`, which always cycles the first
> factor.  The fix checks `CL(B) > 0` first.  Smallest hanging input:
> `AreConjugate(B, B)` with B = σ2σ4σ3⁻¹·Δ² on 5 strings (now a case
> in `harness.cpp`).  All verification criteria below pass.

This note hands an investigation over to a new Claude Code session.  It
was written by the session that found the problem while updating braidlab
(`~/Projects/articles/braidlab`), which bundles cbraid.  Everything below
was measured with the programs in this folder.

## Summary

- cbraid issue #3 ("ultra summit set segfaults",
  <https://github.com/jeanluct/cbraid/issues/3>, still **open**) was
  fixed on `master` by PR #9 (`f70a5f3`, "Fix Delta-power factor handling
  in USS and centralizer paths"), plus follow-ups up to `6484749`.
- **The fix works for the centralizer.**  The issue's reproducer (the
  centralizer of the 5-string braid `3 3 4 3 3 2 1 4 3 2`) crashes the
  old code and succeeds with the new.
- **But the new code hangs in `AreConjugate`** on random inputs that the
  old code answers instantly and correctly.  With the deterministic fuzz
  inputs `fuzz 50 1`, the old code takes 0.6 s for all 50 cases.  The new
  code does not finish them in 120 s and sits at 100% CPU: an infinite
  loop, or at least a catastrophic slowdown, introduced by the fix.
- **The task:** find why, fix it on cbraid `master` (keeping the issue-#3
  crash fixed), and verify both.

## Why it matters (braidlab)

- braidlab's `extern/cbraid` is a git subtree at `6f06e68` (before the
  fix).  It is tracked by `cbraid-branch` in the braidlab repo and updated
  as described in braidlab's `extern/README.md`.
- braidlab reaches `AreConjugate` from its public API:
  `braid.conjtest` → `cfbraid.conjtest` → `conjtest_helper.cpp`, which
  runs `WordToBraid` → `MakeLCF` → `Braiding::AreConjugate`.  The
  `fuzz.cpp` and `harness.cpp` programs in this folder mimic that path
  exactly.
- With the **old** code, braidlab's path looked safe: 18 hand-picked
  cases and 9,000 random conjugate pairs (seeds 1, 2 and 3 × 3,000), many
  with Δ-power factors.  There were no wrong answers, and the checked
  standard library found no invalid iterator dereference.  braidlab does
  not use the centralizer, where the crash was.
- So updating braidlab to current cbraid would swap a crash it doesn't
  seem to hit for a hang it does hit.  A hang inside a MEX file freezes
  MATLAB, and Ctrl-C does not help.
- **The cbraid update is on hold for braidlab 3.4.1** until this is fixed.
  Nothing in braidlab has been changed for it.

## What changed in the fix

The old code took the first factor of a braid's left normal form as
`*B.FactorList.begin()`.  For a braid that is a power of Δ, or trivial,
`FactorList` is empty, so that dereference is undefined behavior: the
issue-#3 crash.  The fix adds

```cpp
ArtinFactor FirstFactorInLCF(ArtinBraid B)
{
  sint16 n=B.Index();
  if (B.LeftDelta>0)  return ArtinFactor(n,1);
  if (CL(B)==0)       return ArtinFactor(n,0);
  return *B.FactorList.begin();
}
```

and uses it in `SendToUSS`, `Transport`, `Returns`, `Pullback`, `TreePath`
and `AreConjugate` in `lib/braiding.cpp`.

Note one point to check: `FirstFactorInLCF` returns Δ whenever
`LeftDelta > 0`, even if `FactorList` is **not** empty.  The old code
used `*FactorList.begin()` in that case.  If that is the behavior change
behind the hang, then `Transport`, `Pullback` or the cycling in `Returns`
could stop converging.  (Two sites, `Returns` and `Pullback`, already had
explicit Delta and identity branches in the old code, which the fix
replaced with the helper.)  This is a hypothesis to test, not a
conclusion.

To see the whole change: `git diff 6f06e68 master -- lib/braiding.cpp`.

## Files in this folder

- `harness.cpp`: 18 fixed `AreConjugate` cases (the reproducer braid,
  half and full twists on 3–6 strings, conjugates) with known answers.
  `harness centralizer` runs the issue-#3 centralizer reproducer as a
  control.
- `fuzz.cpp`: `fuzz N SEED` makes N random pairs `(b, g b g^-1)` (3–6
  strings, short random words, `b` times Δ^k for k from 0 to 3) and
  checks that `AreConjugate` says conjugate.  It uses `rand()` with the
  given seed, so the inputs are deterministic and identical for old and
  new code.
- `build.sh`: builds `build/{harness,fuzz}-{old,new}`.  "Old" is commit
  `6f06e68`, extracted with `git archive`; "new" is the **working tree**,
  so edits to `lib/braiding.cpp` are tested by rerunning `build.sh`.  The
  flags are `-D_GLIBCXX_DEBUG` (aborts on dereferencing a past-the-end
  iterator), ASan, UBSan, `-O1 -g`.
- `build/` holds the build outputs; everything here is untracked.

## Reproduce

```bash
cd ~/C/cbraid
devel/iss003-ultra-summit-set-segfaults/build.sh
B=devel/iss003-ultra-summit-set-segfaults/build
$B/harness-old centralizer   # aborts: attempt to dereference a past-the-end iterator
$B/harness-new centralizer   # centralizer reproducer: 2 generators
$B/harness-new               # 18 cases, "done, 0 wrong"
$B/fuzz-old 50 1             # "fuzz done: 50 cases, 0 wrong", instantly
timeout 20 $B/fuzz-new 50 1  # times out (exit 124)
```

## Suggested next steps

1. **Find the first hanging input.**  Make `fuzz.cpp` print each case
   (n, `b`, `g`, k) before running it, or give each case an `alarm()`.
   Record the first case that takes more than a few seconds with the new
   code, and confirm the old code answers it at once, correctly.
2. **Minimize it:** shorten `b` and `g` while it still hangs.
3. **Locate the loop.**  Run the new build under `gdb`, interrupt it
   after a few seconds, and take a backtrace.  Then compare that function
   between `6f06e68` and `master`.  The likely suspects are the loops in
   `Returns()` (`find(ret.begin(), ret.end(), F1)` never matching), the
   `while(B1!=B)` cycling loops, and `SendToUSS`.
4. **Root cause and fix on `master`.**  The fix must keep the issue-#3
   centralizer reproducer working.  A regression test for the hang would
   be good (cbraid's `programs/` has reproducible-input files, and the
   branch `iss003-ultra-summit-set-segfaults` has the earlier ones).
5. **Verify:**
   - `harness-new centralizer` succeeds;
   - `harness-new` gives 0 wrong;
   - `fuzz-new 3000 S` for S = 1, 2, 3 finishes with 0 wrong, no
     checked-STL abort, and no sanitizer report;
   - timing is comparable to `fuzz-old`.
6. **Report and ask** before committing to `master`, pushing, or
   commenting on issue #3.  Once cbraid is fixed, updating braidlab is a
   separate step, done from the braidlab repo following its
   `extern/README.md`.

## Conventions

- Don't push, or comment on the GitHub issue, without the maintainer's
  approval.
- Commit messages: a short summary line, then an explanation of why.
- Markdown prose wrapped at about 78 columns.
