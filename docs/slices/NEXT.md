---
slice: S2.7
stateFile: docs/STATE.md
donotCount: 7
seededFrom: 814a514
---

# Next slice — S2.7: make the NFS host diagnostics emit English

## The next action — verbatim

S2.7 — Make the NFS host diagnostics emit English. The boot's `[nfs]` runtime lines are already English, but the graphics layer prints Chinese: `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` holds 250 `MLOG` strings with Chinese text, 44 more sit in `fprintf`/`printf` sites across `games/nfs`, and the 61 `TOTK_UI("中文","English")` pairs select Chinese from the Mac's language (`totk_ui_lang.h`, `TOTK_UI_LANG` unset). Translate the emitted strings to English without changing format specifiers, arguments, or values; make the NFS fork default the host's own text to English (leave the game's own language on `TOTK_LANG`); rebuild and show a bounded headless boot whose host diagnostics are English. Do not translate comments, and do not rename predecessor measurements (Do-Not #4). Measure first and record only what the run shows.

## What is settled — do not re-derive

- **Submission works.** With `TOTK_GPU_ALLOW_SUBMIT=1`, frame 1 carries one `QueueSubmitCommands` and one `QueuePresentTexture`; GPU time is p50 0.86 / p95 1.21 ms over 292 frames (boot1) and p50 1.45 / p95 1.74 over 300 (boot2), against 0.00 ms with submission disabled. No Metal validation failure, draw error, crash, or kernel panic was recorded in four bounded runs.
- **The picture path works, and the first non-black capture is frame 441.** The frame-2 and frame-144 (5 s) shots are pure black (mean 0, max 0); the frame-441 (15 s) shot is non-black (mean 0.000697, max 0.996; a small white bar lower right, `f15.png`). No further visual-correctness claim.
- **A new port stop is recorded, not this slice's work.** Presenting stops at frame 600 (~20 s): the 30/50/65 s shots produce no file, a 40 s dump shows guest thread `0x1002` on core 0 for 16.7 s with `wait=0` and `nsvc=71,196,080`, and the process stays alive to the timer. The cause is not established. Do not re-measure or fix it here.
- **The Chinese comes from the graphics layer, not the runtime.** `[nfs]` runtime lines are English. `nvn_mtl.m` holds 250 `MLOG` Chinese strings; 44 `fprintf`/`printf` sites elsewhere hold Chinese; `TOTK_UI` is bilingual by design and picks the Mac's language (`zh` here) unless `TOTK_UI_LANG` is set.
- **The work tree is re-derivable, not evidence.** `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` (7.2 GB); build from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs`. Temp goes to `/Volumes/X9BS/externalTmp/MacNativeGames` (`TMPDIR`), never the internal volume.
- **Budget:** `.babysteps/config.json` plans 180,000 work-unit tokens; the live host window binds — verify the session model each slice and size to the usable window after the compaction reserve.
- Earlier (S2.1–S2.6): window paths, NVN names, device properties, HID, account, `friend:u`, friend-invitation event, shader translator, audio — as recorded in `docs/STATE.md`.

## Do-Not list that applies

- Do-Not #4 (do not blanket-rename a forked tree's comments; keep predecessor names unless re-observed) and #7 (do not call a log bilingual because its comments carry English; check the emitted string).
- Do not claim visual correctness beyond the captured frame; do not claim gameplay.

## Code to touch

| Work | Touch |
| --- | --- |
| Chinese log strings | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` — 250 `MLOG` strings; `games/nfs` — 44 `fprintf`/`printf` sites with Chinese. Translate the emitted string only; preserve every format specifier and argument. |
| UI language default | `games/nfs/third_party/d2r_gfx/gfx/nvn/include/totk_ui_lang.h` — `totk_ui_zh()`; the NFS fork should default the host's own text to English (`TOTK_UI_LANG` may still override). The game's language stays on `TOTK_LANG` (`runtime/services.c`). |
| Verification | Build from `games/nfs` with `make runtime`; run a bounded headless boot from the work tree. The boot's stderr is the evidence; no game-frame claim is needed for this slice. |

## Gate commands

- `python -m unittest discover -s tests` passes (with `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`); a bounded boot whose host diagnostics contain no Chinese outside the game's own output; every claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from earlier seeds; do not re-measure them as if new. The S2.5 boot numbers (8 shaders, frame 900, p50 33.33 ms) and the S2.6 numbers (GPU p50 0.86–1.45 ms, frame 441, frame 600, 255 tests) are the prior record: quote them, do not re-derive them as new.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.6 — Submit the encoded frames and observe the first picture" (submission, picture, stall, gate) and "S2.5 — Resolve the `AudioOut buffer bounds` stop" only where the descriptor contract is needed.
- `docs/donot.md` #7 — the bilingual-log claim and why it was wrong.
