---
slice: S2.25
stateFile: docs/STATE.md
donotCount: 23
seededFrom: a602855 (the operator's playable run)
plannedTokens: 150000
---
# Next slice — S2.25: read the operator's captured play log, then default the depth-stencil path

## The next action — verbatim

S2.25 — Read the operator's captured play log and name the next stop. He runs the windowed boot himself
with the log captured to `.babysteps/scratch/S2.25-play/play.log`
(`TOTK_GPU_ALLOW_SUBMIT=1 TOTK_GPU_MIDCLEAR=1 TOTK_INPUT_LOG=1 ./build/nfs-native rewritten/nfs100 boot 2>&1
| tee …`), so a play session's diagnostics are on disk for the first time. Read that log for the next stop,
the input reads that show the frame was controllable, the save write, and any `FATAL`/`FAIL`. Then default
the mid-pass depth-stencil path on — `gpu_path_on("TOTK_GPU_MIDCLEAR", …)` (`nvn_mtl.m:4053`) aborts
unless the switch is `1`, and the probe (6144 checks) plus the operator's ≥5-minute session without a panic
have met the isolation's condition; keep the switch as an opt-out. Do not touch `games/totk` or
`games/by3`.

S2.25 closes when the operator's `play.log` is read and the next stop (or the absence of one) is named from
it, and a repo-built boot runs the mid-pass depth-stencil clear without `TOTK_GPU_MIDCLEAR`. A run whose
log is not kept does not close it.

## What the next session does first

1. Read `.babysteps/scratch/S2.25-play/play.log`. If it is missing or empty, the operator has not run the
   capture yet — say so and stop; do not substitute a log from another run and do not re-derive the next
   stop from the disk trace.
2. In it, find: the last `frame` line and the stop (if any) — `FATAL`, `FAIL`, `signal=`, `abort`,
   `not implemented`; the `[nfs-pctl]` lines; the `[totk-input]`/`user input` reads that show the pad was
   live; the `fs mount save` and any save write; and whether any `FATAL mid-pass / masked / scissored
   depth-stencil clear` appears (it should not, the switch was on).
3. Name the next stop from the log, not from the operator's recollection, and record it.
4. Then default the path: `gpu_path_on("TOTK_GPU_MIDCLEAR", …)` (`nvn_mtl.m:4053`) aborts unless the
   switch is `1`. Default the depth-stencil call site (:9083) on, keep `TOTK_GPU_MIDCLEAR` as an opt-out,
   and keep the comment honest about why the isolation existed. Build from the repo (Do-Not #19).

## What is settled — do not re-derive

- **The boot reaches a controllable frame.** With the switch on, the operator attempted to play the first
  Career Mode race. No crash report exists for that run; it lasted ≥5 minutes and wrote 1090 pipeline
  records and a 553 KB save-sized file.
- **The depth-stencil clear's probe passes**: 6144 checks — scissored clears, partial stencil masks,
  preserved colours and untouched pixels across passes.
- **`TOTK_GPU_MIDCLEAR` is the only thing between the repo build and that frame.**
- **The fs host writes saves without a `savedata` service** — but 44% of `MUD64.8` is `0xcd` fill, so its
  content is not established (Do-Not #6).
- **The runtime writes only stdout/stderr.** Nothing tees; a session is on disk only if the shell
  captured it. `log=/dev/ttys002` was the operator's terminal, and that log is gone.
- **The gate** passed 255 tests, 1 skipped, exit 0; it does not cover runtime C.

## Withdrawn — proven wrong, do not use

- **"The race-course reveal cannot run until the clear is implemented."** It runs with the switch on; the
  path already existed, gated.
- **"`savedata` is the next blocker."** Withdrawn: the game wrote a save through the fs host with no
  `savedata` request.
- **"The operator's 19:38 run is a log."** It is not: `log=/dev/ttys002`, nothing captured.

## Code to touch

| Work | Touch |
| --- | --- |
| The log | `.babysteps/scratch/S2.25-play/play.log` — the operator's session; read it, do not overwrite it. |
| The gate | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m:4053` (`gpu_path_on`) and its depth-stencil call site (:9083). |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim a completed race; do not read a rate or frame count (Do-Not #14); do not treat the `0xcd` save as data (Do-Not #6). |

## Read these first, and only these

- `.operator/specs/nfs-port.md` § "Claim boundaries".
- `docs/STATE.md` § "S2.24" and "Next action".
- `docs/donot.md` #6, #8, #11, #14, #17, #19, #22, #23.

## Do-Not entries that apply here (8 of 23)

- #6 — a fill pattern is not data; `0xcd` in the save is uninitialised.
- #8 — do not name a stall by the frame one run happened to stop at.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU.
- #17 — do not name main's own call by the SDK function an unwind frame reaches.
- #19 — do not treat a work tree as the repo; re-sync and force the rebuild.
- #22 — do not read a scripted schedule's failure to reproduce a live session as a fact about the buttons.
- #23 — do not answer a command that returns an object with a result-only reply.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (255
  tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 150k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Write the next seed from what S2.25 leaves behind (the stop its log names, or S3.1d if the save content is
the next gap), re-verify the Do-Not count (23 or higher), and commit the state and the seed.
