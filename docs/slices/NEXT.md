---
slice: S2.26
stateFile: docs/STATE.md
donotCount: 23
seededFrom: the S2.25 close (gate lifted, play log read)
plannedTokens: 150000
---
# Next slice — S2.26: give a dev-tree run a precompiled shader bundle

## The next action — verbatim

S2.26 — Give a dev-tree run a precompiled shader bundle so variants stop compiling on demand. The
operator's captured log (`.babysteps/scratch/S2.25-play/play.log`) shows `precompiled metallib bundle
…/build/shader-cache/b-totk-native-v4-unverified is missing; warming from this machine's runtime
translation cache` and then `shader library: 0 loaded precompiled, 601 compiled on demand` on a run that
already held a 3,937-entry `rt-` cache — so the preload path is absent, not late, and the parallel
precompile (32 pipelines per submit, `TOTK_WARM_THREADS` 8, 2 in world) is the only cushion. Produce that
bundle from the runtime cache (see `docs/first-launch-shaders.md` § packaging precompile and
`tests/test_variant_libs.py` for the variant-lib path), or, if no builder exists for a dev tree, preload
the `rt-` cache before the first draw. The per-submit precompile summary (`… taken by draws (N waits
totalling X ms)`) never printed, so instrument the wait before claiming a hitch cost. Do not touch
`games/totk` or `games/by3`.

S2.26 closes when a repo-built boot logs a nonzero `loaded precompiled` count and a `compiled on demand`
count below S2.25's 601, with both numbers recorded — or when the measurement shows a dev tree cannot
carry a bundle and the preload-at-boot route is taken instead, with the same before/after numbers.

## What the next session does first

1. Read `docs/first-launch-shaders.md` § 打包时预编 (packaging precompile) and § 后台预热并行编译, and find
   the builder that emits the `b-<chain>` bundle from the `rt-<chain>` cache; `tests/test_variant_libs.py`
   names the variant-lib path. `nvn_mtl.m` is over the index's size cap — read it directly (Do-Not #16).
2. Decide which route is real: build the `b-` bundle for a dev tree (preferred — it is what a packaged app
   carries), or preload the `rt-` cache before the first draw. Do not do both.
3. Measure the before: `shader library: 0 loaded precompiled, 601 compiled on demand` on a repo-built run,
   and instrument the per-submit precompile summary (`… taken by draws (N waits totalling X ms)`) so the
   hitch cost exists as a number before the change.
4. Build from the repo (Do-Not #19), run bounded, and compare `loaded precompiled` / `compiled on demand`.

## What is settled — do not re-derive

- **The game is playable to the operator's quit**: 1409 log lines, no fault, last frame 11400, ended
  `window closed`. No race was completed.
- **The mid-pass clears run by default**, and a repo-built boot reaches the race without `TOTK_GPU_MIDCLEAR`.
- **The preload path is absent in a dev tree, not late**: the `b-` bundle is missing, so every variant is
  compiled on demand (`0 loaded precompiled, 601 compiled on demand`).
- **The mitigations already on**: parallel precompile (32/submit), `TOTK_WARM_THREADS` 8 (2 in world),
  light warm-up. None of them preloads.
- **The wait total never printed** — the summary line is absent from a 1409-line log.
- **The gate** passed 255 tests, 1 skipped, exit 0.

## Withdrawn — proven wrong, do not use

- **"Shader loading is not async."** The parallel precompile does hand pipelines to a background queue;
  what is missing is the precompiled bundle, so first use still compiles. Say "no preload in a dev tree".
- **"The race stop is the intro video decoder."** Still wrong: no `movie`/video service opens.
- **"The save is a working save."** It is 44% `0xcd` fill (Do-Not #6).

## Code to touch

| Work | Touch |
| --- | --- |
| The bundle | the builder that emits `build/shader-cache/b-<chain>` from `rt-<chain>`; see `docs/first-launch-shaders.md` § 打包时预编 and `tests/test_variant_libs.py`. |
| The loader | `nvn_mtl.m:4627` (bundle-missing message), `:4977`/`:4992` (precompiled vs on-demand counters), `totk_shader_seed.h`/`totk_shader_rt.m` (import path). |
| Evidence | `.babysteps/scratch/S2.26-shaders/`: before/after `loaded precompiled` counts and the wait total. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim a hitch cost before the wait total exists; do not read a rate or frame count (Do-Not #14). |

## Read these first, and only these

- `.operator/specs/nfs-shaders.md` — the mechanism as it stands.
- `docs/STATE.md` § "S2.25" and "Next action".
- `docs/donot.md` #6, #8, #11, #14, #16, #19, #22.

## Do-Not entries that apply here (7 of 23)

- #6 — a fill pattern is not data.
- #8 — do not name a stall by the frame one run happened to stop at.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU.
- #16 — do not read a zero-hit `search_text` as absence; `nvn_mtl.m` is over the index's size cap.
- #19 — do not treat a work tree as the repo; re-sync and force the rebuild.
- #22 — do not read a scripted schedule's failure to reproduce a live session as a fact about the buttons.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (255
  tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 150k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Write the next seed from what S2.26 leaves behind, re-verify the Do-Not count (23 or higher), and commit
the state and the seed.
