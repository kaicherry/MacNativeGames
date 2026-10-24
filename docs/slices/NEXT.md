---
slice: S2.25
stateFile: docs/STATE.md
donotCount: 23
seededFrom: dc6cc23 (the S2.24 probe record)
plannedTokens: 150000
---
# Next slice — S2.25: lift the depth-stencil gate and capture a logged play session

## The next action — verbatim

S2.25 — Lift the `TOTK_GPU_MIDCLEAR` gate and capture a logged play session. The probe passed (`checked`
6144) and the operator's enabled run lasted at least five minutes in the first Career Mode race with no
crash report, so the isolation's condition — a standalone probe plus a full-game run without a panic — is
met. Make the mid-pass depth-stencil clear run without the switch, rebuild from the repo, and run one
bounded windowed session with its output captured to a file (the operator's run wrote to `/dev/ttys002`,
so nothing was kept). Name the next stop from that log; if the operator's session ended at a stop, that
stop is the unit. Do not touch `games/totk` or `games/by3`.

S2.25 closes when a repo-built boot runs the mid-pass depth-stencil clear without `TOTK_GPU_MIDCLEAR` and
its log is on disk — the reveal and the first Career Mode race run, and the next stop (or the absence of
one) is named from that log. A run whose log is not captured does not close it.

## What the next session does first

1. Ask the operator how his session ended — that answer may name the next stop and change this unit's
   shape. Do not re-derive it from the disk: the disk shows only a ≥5-minute session, 1090 pipeline
   records and a save write.
2. Make the path default. `gpu_path_on("TOTK_GPU_MIDCLEAR", …)` (`nvn_mtl.m:4053`) aborts unless the
   switch is `1`; the probe passed and the full game ran without a panic, so default it on and keep the
   switch as an opt-out rather than deleting the gate. Keep the comment honest about why it existed.
3. Build from the repo (Do-Not #19) and run **one** bounded windowed session with its output captured:
   `... boot 2>&1 | tee <scratch>/play.log`. No `TOTK_GPU_MIDCLEAR`.
4. Read the log for the next stop, the save write, and any `FATAL`/`FAIL`. Name the next stop from the
   log, not from the operator's recollection.

## What is settled — do not re-derive

- **The boot reaches a controllable frame.** With the switch on, the operator attempted to play the first
  Career Mode race. No crash report exists for that run; it lasted ≥5 minutes.
- **The depth-stencil clear's probe passes**: 6144 checks — scissored clears, partial stencil masks,
  preserved colours and untouched pixels across passes.
- **`TOTK_GPU_MIDCLEAR` is the only thing between the repo build and that frame.**
- **The fs host writes saves without a `savedata` service**: a 553 KB file lands in the mounted save
  directory — but 44% of it is `0xcd` fill, so its content is not established (Do-Not #6).
- **The runtime writes only stdout/stderr.** Nothing tees; a session is on disk only if the shell
  captured it.
- **The gate** passed 255 tests, 1 skipped, exit 0; it does not cover runtime C.

## Withdrawn — proven wrong, do not use

- **"The race-course reveal cannot run until the clear is implemented."** It runs with the switch on; the
  path already existed, gated.
- **"`savedata` is the next blocker."** Withdrawn: the game wrote a save through the fs host with no
  `savedata` request.
- **"The operator's run is a log."** It is not: `log=/dev/ttys002`, nothing captured.

## Code to touch

| Work | Touch |
| --- | --- |
| The gate | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m:4053` (`gpu_path_on`) and its depth-stencil call site (:9083). Default that path on; keep `TOTK_GPU_MIDCLEAR` as an opt-out. |
| Evidence | `.babysteps/scratch/S2.25-play/`: the tee'd windowed log, the save listing, the probe log reference. |
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
