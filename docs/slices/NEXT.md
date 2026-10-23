---
slice: S2.8
stateFile: docs/STATE.md
donotCount: 8
seededFrom: 13d2b2b
---

# Next slice — S2.8: locate the NFS present stall

## The next action — verbatim

S2.8 — Locate why NFS presenting stops between frame 354 and 600. S2.6 recorded a stop at frame 600; S2.7's two English-log boots stopped at 399 (13.6 s) and 354 (12.0 s), so the stop frame varies. With `TOTK_GPU_ALLOW_SUBMIT=1`, run bounded headless boots with `TOTK_DUMP_AFTER` and `hletrace`, capture the guest-thread state at the stop, and identify the last submitted and presented frame, the fence, event, or shared word the main thread is spinning on or waiting for, and whether the stop tracks wall time, frame count, or a specific game event. Record the first stop with its own message; do not fix it in the same slice unless the measurement names the cause. Measure first and record only what the run shows; no gameplay claim.

## What is settled — do not re-derive

- **Submission works.** With `TOTK_GPU_ALLOW_SUBMIT=1`, frame 1 carries one `QueueSubmitCommands` and one `QueuePresentTexture`; GPU time is p50 0.86 / p95 1.21 ms over 292 frames (boot1) and p50 1.45 / p95 1.74 over 300 (boot2), against 0.00 ms with submission disabled. No Metal validation failure, draw error, crash, or kernel panic was recorded.
- **The picture path works; the first non-black capture is frame 441.** The frame-2 and frame-144 (5 s) shots are pure black (mean 0, max 0); the frame-441 (15 s) shot is non-black (mean 0.000697, max 0.996; a small white bar lower right, `f15.png`). No further visual-correctness claim.
- **The stall frame varies.** S2.6: boot1 stopped at 354, boot2 at 600. S2.7: 399 (13.6 s) and 354 (12.0 s). The process stays alive to the timer. A 40 s dump shows guest thread `0x1002` on core 0 for 16.7 s with `wait=0` and `nsvc=71,196,080`, the other guest threads waiting; the 55 s `hletrace` run records 460 `QueueSubmitCommands` and 460 `QueuePresentTexture` calls. The cause is not established. Do not re-derive the stall; reproduce it with a dump.
- **Host diagnostics are English.** 721 literals across 22 files translated; `totk_ui_lang.h` defaults to English (`TOTK_UI_LANG=zh` overrides); two bounded boots emitted 191-line ASCII-only stderr; gate 255 tests, 1 skipped. Code commit `13d2b2b`.
- **The work tree is re-derivable, not evidence.** `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` (7.2 GB); build from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs`; temp goes to `/Volumes/X9BS/externalTmp/MacNativeGames` (`TMPDIR`), never the internal volume.
- **Budget:** `.babysteps/config.json` plans 180,000 work-unit tokens; the live host window binds — verify the session model each slice and size to the usable window after the compaction reserve.
- Earlier (S2.1–S2.7): window paths, NVN names, device properties, HID, account, `friend:u`, friend-invitation event, shader translator, audio, English logs — as recorded in `docs/STATE.md`.

## Do-Not list that applies

- #4 (do not blanket-rename predecessor comments), #7 (do not call a log bilingual from its comments), #8 (do not name a stall by one run's stop frame).
- Do not claim visual correctness beyond the captured frame; do not claim gameplay. Do not "fix" the stall before the dump names the cause.

## Code to touch

| Work | Touch |
| --- | --- |
| Stop capture | `TOTK_DUMP_AFTER` (guest-thread dump), `hletrace 1/0` (host-call counts), `TOTK_INPUT_FILE` if a marker is needed; no source change unless the existing dump cannot name the wait. |
| If a probe is needed | `games/nfs/runtime/threads.c` / `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` diagnostic counters only; record the observed wait word/fence before touching logic. |

## Gate commands

- `python -m unittest discover -s tests` passes (with `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`); a bounded headless boot with the dump; every claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from earlier seeds; do not re-measure. The S2.5/S2.6/S2.7 numbers (8 shaders, frame 900, p50 33.33 ms; GPU p50 0.86–1.45 ms, frame 441, 354–600, 255 tests; 721 strings) are the prior record: quote them, do not re-derive them as new.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.6 — Submit the encoded frames and observe the first picture" (the stall and dump) and "S2.7 — Make the host diagnostics emit English" (what changed).
- `docs/donot.md` #7/#8.
- `docs/STATE.md` — the S2.6 thread-dump detail.
