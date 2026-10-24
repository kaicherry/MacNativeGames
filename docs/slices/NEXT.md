---
slice: S2.27
stateFile: docs/STATE.md
donotCount: 24
seededFrom: the S2.26 close (dev-tree shader libraries load precompiled)
plannedTokens: 150000
---
# Next slice — S2.27: report the per-submit wait total and measure the hitches

## The next action — verbatim

S2.27 — Report the per-submit wait total, then measure whether the prebuilt libraries removed the waits.
S2.26 removed on-demand compilation (`601 loaded precompiled, 0 compiled on demand`) but the wait total that
would name a hitch never printed: the summary at `nvn_mtl.m:7732` fires only on every 100th prediction scan
(`g_pp_scans % 100`), and a run completes fewer than 100 scans. Lower the cadence (or report the total another
way) so `parallel precompile to submit … taken by draws (N waits totalling X ms)` appears, then run one bounded
windowed boot with `build/shader-libs` present and one with it absent (same schedule, GPU otherwise free) and
compare `N waits` and `X ms`. Do not claim a hitch cost before the total exists, and discard a batch taken while
another local process held the GPU (Do-Not #14). Do not touch `games/totk` or `games/by3`.

S2.27 closes when the per-submit wait summary prints in a boot and one windowed boot with `build/shader-libs`
and one without report their `N waits` / `X ms` side by side, or when the measurement shows the waits are zero
with and without the libraries and that is recorded instead.

## What the next session does first

1. Read the scan function around `nvn_mtl.m:7690`–`:7741` (`pso_predict_rest`) and the counters it reports
   (`g_pp_scans`, `g_pp_taken`, `g_pp_waits`, `g_pp_wait_ns`, declared at `:6272`). `nvn_mtl.m` is over the
   index's size cap — read it directly (Do-Not #16).
2. Make the total appear: lower the `g_pp_scans % 100` cadence (or print it when the run stops). Keep it
   logging-only — this line must not change what is submitted.
3. Measure both ways on the same binary: `make shader-libs` present, then move/rename `build/shader-libs` away,
   and run a bounded windowed boot each (same schedule, GPU otherwise free). Compare `N waits` / `X ms` with the
   `loaded precompiled` / `compiled on demand` counts from the same two logs.
4. Build from the repo (Do-Not #19) and capture both logs under `.babysteps/scratch/S2.27-waits/`.

## What is settled — do not re-derive

- **The dev-tree preload route is `build/shader-libs`**: `shader_load` (`nvn_mtl.m:4985`) reads the variant from
  `build/shader-cache/rt-<chain>` and prefers `build/shader-libs/<SHA-256 of the MSL>.metallib`
  (`:4977`/`:4992`) over compiling it. `make shader-libs` (`games/nfs/scripts/build_shader_libs.py`) writes
  them; the `Makefile` `boot` target depends on it.
- **Measured on a repo-built windowed boot**: `601 loaded precompiled, 0 compiled on demand`, warm-up
  `772 shaders / 502 render pipelines`, no `shader compiled on demand: #N` line, no load failure. Against
  S2.25's `0 loaded precompiled, 601 compiled on demand`.
- **The libraries dedupe by source bytes**: 772 `rt-` sources → 349 unique libraries; a changed compiler or flag
  set wipes `build/shader-libs` (`compiler.txt` stamp).
- **The `b-<chain>` bundle is not the route** (Do-Not #24): it is only consulted when the key is absent from
  the `rt-` cache, and route B's shader map has 0 entries here. Do not build one.
- **The wait total has never printed** in any run, here or in S2.25's 11,400-frame log.
- **The gate** passed 261 tests, 1 skipped, exit 0.

## Withdrawn — proven wrong, do not use

- **"No preload in a dev tree."** Now false for every key in the `rt-` cache: they load precompiled. Only a key
  in neither `rt-` nor `build/shader-libs` is still compiled on demand.
- **"The `b-<chain>` bundle is the fix."** It would have been inert (Do-Not #24). The seed that named it was
  wrong.
- **"The race stop is the intro video decoder."** No `movie`/video service opens.
- **"The save is a working save."** It is 44% `0xcd` fill (Do-Not #6).

## Code to touch

| Work | Touch |
| --- | --- |
| The wait total | `nvn_mtl.m:7732` (the `g_pp_scans % 100` summary) and the counters at `:6272`. |
| Evidence | `.babysteps/scratch/S2.27-waits/`: two logs (with and without `build/shader-libs`), each carrying `loaded precompiled` / `compiled on demand` and `N waits` / `X ms`. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim a hitch cost before the total exists; do not read a rate or frame count (Do-Not #14); do not rebuild the `b-` bundle (#24). |

## Read these first, and only these

- `.operator/specs/nfs-shaders.md` — the mechanism and the measurement gap.
- `.operator/guides/running-the-nfs-boot.md` — the repo build and the windowed boot.
- `docs/STATE.md` § "S2.26" and "Next action".
- `docs/donot.md` #6, #11, #14, #16, #19, #24.

## Do-Not entries that apply here (6 of 24)

- #6 — a fill pattern is not data (the save).
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a rate or frame count taken while another local process held the GPU; discard such a batch.
- #16 — do not read a zero-hit `search_text` as absence; `nvn_mtl.m` is over the index's size cap.
- #19 — do not treat a work tree as the repo; re-sync and force the rebuild.
- #24 — do not build the `b-<chain>` bundle; the `rt-` cache is read first and the bundle is never consulted.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261
  tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 150k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Write the next seed from what S2.27 leaves behind, re-verify the Do-Not count (24 or higher), and commit the
state and the seed. The queue after this is S3.1c (`savedata`); S5 is provisional.
