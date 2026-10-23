---
slice: S2.9
stateFile: docs/STATE.md
donotCount: 8
seededFrom: ff1a5f4
---

# Next slice — S2.9: name the condition-variable object and fix the lost wakeup

## The next action — verbatim

S2.9 — Name the guest object that owns the condition variable at guest heap+0xC284E8A8 and the waiter/signaller pair in the present stop, then fix the lost wakeup. S2.8 measured the stop with `TOTK_PRESENT_WATCHDOG` and the thread dump: six bounded boots stopped at frames 440, 677, 676, 545, 565 and 494 (14.9–22.8 s; the prior record is 354–600), `submissions == last presented frame` in every run, and the created guest thread `0x1002` was either spinning in `SignalProcessWideKey` (0x1D) on the key or waiting in `WaitProcessWideKeyAtomic` (0x1C) on the same key; the render thread `0x1001` stopped submitting. Identify the object and the missing signal from the dump and the call chains before changing behaviour; do not fix from a guess. Measure first and record only what the run shows; no gameplay claim.

S2.9 closes when the object at guest heap+0xC284E8A8 and the waiter/signaller pair are named from a recorded dump, the lost wakeup is fixed, and a bounded headless boot presents past frame 677 without the condition-variable stall; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The stop state is one condition variable, not the present queue.** Every run has `submissions == last presented frame` at the stop; no frame was submitted without presenting, and the next frame was never submitted. The stop is before submission, not in Metal, a fence, or a present event.
- **The condition variable is at guest heap+0xC284E8A8** (lock word at heap+0xC284E8A4, tag/thread `0x1002`). Per-run addresses differ only by the randomized heap base: A `0x7c7284e8a8`, B `0x73f284e8a8`, C/E `0x70f284e8a8`, D `0x7bf284e8a8`, F `0x70f284e8a8`.
- **The wait/spin split.** In A/B/C/E/F the created guest thread `0x1002` (prio 44, core 0) spins in `SignalProcessWideKey` (0x1D) on that key — top counts 11,149,511 / 11,642,620 / 11,084,815 / 4,635,340 / 11,148,046, key word 0. In D it waits in `WaitProcessWideKeyAtomic` (0x1C) on the same key (`wait=2`, `addr=heap+0xC284E8A4`, `key=heap+0xC284E8A8`); core 0 owner 0. The render thread `0x1001` (the NVN caller, `hletrace` tags its calls `h=0x1001`) is in the guest lock path in the spin runs (A/F: `wait=0`, last SVC `0x1A ArbitrateLock`, A `heap+0xC284E7A4`, F `heap+0xC284E824`) and waits on a different condition in D (`wait=2`, key `0x16ae6c8f8`, last host call `nn::oe::TryPopNotificationMessage`). No fix.
- **The stop frame and time vary; neither is pinned.** S2.8: 440, 677, 676, 545, 565, 494 (14.9–22.8 s). Prior: S2.6 354 and 600; S2.7 399 and 354. Range **354–677**, 12.0–22.8 s. Identical configurations differ (A 440/14.9 s vs D 545/18.4 s vs F 494/16.6 s). Report the range and the runs (Do-Not #8).
- **`framestep` is a no-op in NFS.** `runtime_frame_step_pin` is `void runtime_frame_step_pin(double ms) { (void)ms; }` (`games/nfs/runtime/game_patches.c:13`); boots B/C are not a frame-rate separation test and no `framestep` result may be cited as one.
- **Diagnostics available.** `TOTK_PRESENT_WATCHDOG=<ms>` (unset/0 off; `1` = 1500 ms) prints one `[nvn] present stop: last presented frame …` line at the first present gap after frame 2 and calls the thread dump. The dump prints each thread's last SVC `x0..x3`, a top-3 SVC histogram, and the relevant word: for `0x1C` it reads the key `x1`, for `0x1A`/`0x1B`/`0x1D` the address in `x0`, and for a condition wait (`wkind==2`) the key.
- **Dump limitation.** D's log shows the `0x1C` lock word (`x0`, 0), not the key (`x1`); the `0x1C` path writes the key before releasing the lock. F used the corrected dump and spun, so a waiting key word has not yet been observed. If a new run shows a wait, record the key word.
- **Call chains to name the object.** `0x1002`: `sdk+0xd88504` (spin) / `sdk+0xd884dc` (wait) <- `main+0x75679c` or `main+0x756e64` <- `main+0x756900`/`main+0x7593e0` <- `main+0x745260` (thread entry) <- `sdk+0x55b800` <- `sdk+0x33e7a8` <- `sdk+0x342f74` <- `dm_thread_enter+0x20`. `0x1001` runs through `main+0x349f90`, `main+0x3522bc`, `main+0x366004`.
- **Last new host call under `hletrace 2`.** A `nvnCommandBufferDrawElementsBaseVertex` 13.146 s; B/C/D/E `nvnBufferFinalize` 18.247–18.277 s. A/F stop before that finalize, so it is not causal.
- **Build and run.** Build from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs`; run from the work tree `./build/nfs-native rewritten/nfs100 boot` with `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1`, `TMPDIR` on `/Volumes/X9BS/externalTmp/MacNativeGames`, bounded with `timeout -k 5 N`. The work tree is re-derivable, not evidence.
- **Gate and code.** S2.8 code commit `ff1a5f4`; `python -m unittest discover -s tests` passed 255 tests, 1 skipped. No picture was taken in S2.8; the S2.6 frame-441 capture is the only visual record and no gameplay is established.

## Withdrawn — proven wrong, do not use

- "Presenting stops at frame 600." S2.6's frame-600 was one run; the observed range is 354–677 (Do-Not #8).
- "The stop tracks wall time" and "the stop tracks frame count." Both excluded: identical configurations stopped at different frames and times.
- "B/C separate frame rate by `framestep` 16.7 vs 50 ms." Withdrawn; `runtime_frame_step_pin` is a no-op in NFS.
- "The condition key word is 0 in every run." The 0 is the spin runs' key word; D's waiting key was not read (the old dump read the lock at `x0`). The corrected dump reads it.
- "The main thread `0x1001` is the spinning thread." Withdrawn; `0x1002` is a created guest thread, and `0x1001` is the render/NVN thread.
- Numbers **2, 3, 4, 5** — unchanged from earlier seeds; do not re-measure. The prior record numbers (8 shaders, frame 441, p50 33.33 ms; GPU p50 0.86–1.45 ms; 255 tests; 721 strings) are quoted as history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Name the object | Dump the memory around `heap+0xC284E8A8` (vtable/pointers) and resolve the dump call chains; start in `games/nfs/runtime/threads.c`. |
| Fix the wakeup | `games/nfs/runtime/threads.c` for the HLE `0x1A/0x1B/0x1C/0x1D` condition/lock protocol; `games/nfs/runtime/game_patches.c` only for a guest patch if the race is in the game; `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` only if the present/frame loop is involved. |
| Do not | Change `runtime_frame_step_pin` as part of the fix, and do not fix before the waiter/signaller pair is named from a dump. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S2.8 — Locate the NFS present stop".
- `docs/STATE.md` — the S2.8 write-up and next action.
- `docs/donot.md` #4, #7, #8.
- `.babysteps/scratch/S2.8-NFS-present-stop/bootA.log`–`bootF.log` (the dumps) and `gate.log`.

## Do-Not entries that apply here (3 of 8)

- #4 — **Do not blanket-rename a forked tree's comments to the new game's name.** The NFS fork's comments carry predecessor measurements.
- #7 — **Do not call a log bilingual because its source comments carry an English sentence.** Check the emitted string.
- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes.
- A bounded headless boot with `TOTK_PRESENT_WATCHDOG` and the dump; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

## When this slice closes

Write the next seed from what S2.9 leaves behind, re-verify the Do-Not count (8 or higher), and commit the state, the feasibility record and the seed.
