---
slice: S2.12
stateFile: docs/STATE.md
donotCount: 10
seededFrom: 3d7cda1
---

# Next slice — S2.12: diagnose the new all-idle present stop

## The next action — verbatim

S2.12 — Diagnose the all-idle present stop. S2.11's control run C stopped presenting at 39.7 s (`[totk-gfx] [nvn] present stop: last presented frame 1105, 1105 submissions, no present for 2.6 s (39.7 s since start)`), before its first capture, and never resumed (the last frame number anywhere in its log is 1105). At the dump every core read `owner 0` and every guest thread sat `wait=2` on a condition whose tag is its own handle — main thread `tid=2382712 h=0x1001 prio=44 nsvc=65228 addr=heap+0xC284E8A4 key=heap+0xC284E8A8 tag=0x1001 word@key=0x1`, last host call `nn::oe::TryPopNotificationMessage(unsigned int*)`; `h=0x1002` nsvc=221103; `h=0x1003` nsvc=73212 — and no thread was spinning, so this is not the S2.9 starvation shape. S2.11's press run P did not stop (zero stops, frame 5100 in 180 s) and S2.10's two runs saw none, so the base rate is unknown and reproducing it is the first job. Reproduce the all-idle stop in bounded headless boots (`TOTK_PRESENT_WATCHDOG=2500`, `hletrace 1` with `hletrace 0`-bracketed windows, `TOTK_INPUT_FILE`), record its own message and the runs that did and did not stop, and name the waiter/signaller pair from a dump before changing anything; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

S2.12 closes when the all-idle stop is either reproduced with its own recorded message and its waiter/signaller pair named from a dump, or not reproduced in N bounded runs and that is recorded with the run count, their bounds and their frame counts, together with what the parked threads' condition tags resolve to; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The guest reads the pad and reacts to input.** ⊕ (Mac key 36 = Return → `button()` bit 1024 = Plus) advanced run P from the `PRESS ⊕` title screen (`F1487`, t=52 s) to a save-data access notice reading "THIS ICON INDICATES THE GAME IS ACCESSING SAVE DATA ON THE SYSTEM MEMORY…" with a `Press Ⓐ to continue` prompt (`F1788`, t=62 s), where it stayed to 180 s with the icon flashing (bright `F2629`, dim `F4276`) — live and animating, not frozen. First observation of the guest reacting to input. Attribution limit: run C died at 39.7 s so no control reached the comparison window; the claim rests on the same run's own before-capture, the attract loop's known screens (S2.10's 7 captures: EA logo, logo screen, racing footage only) and the notice gating on a different button.
- **The pad is polled continuously.** Windowed `hletrace`: `nn::hid::GetNpadState` 5,143 (0–45 s) / 6,004 (45–95 s) / 6,600 (95–150 s) ≈ 120/s; `GetNpadStyleSet` 48,868 / 57,038 / 62,700 ≈ 1,140/s. `TOTK_INPUT_LOG=1` records `[totk-input] game reads full-key state for controller 0 (supported styles 0x7, ids allowed 1)` — player 1's full-key pad.
- **The windowed `hletrace` bracket is the instrument.** `runtime_hle_trace_set` (`games/nfs/runtime/hle_stack.c:80`): `1` baselines `since[]` and turns counting on, `0` turns it off and prints `counts[i] - since[i]` ranked, and `counts` advances only while tracing is on — so `1 … 0` is exactly that window. `hletrace 2` logs only the first call of each function, so it cannot answer a rate question.
- **The all-idle stop (run C).** Message verbatim above. Every core `owner 0`; every thread `wait=2` on a condition tagged with its own handle; no thread spinning; presentation never resumed. Base rate unknown: S2.10's two runs (130 s, 185 s) and S2.9's boot3/boot4 saw none; 1 of S2.11's 2 runs stopped. Report the range and the runs (Do-Not #8).
- **Mechanisms.** `shot <path>` captures the next presented frame (~2 frames after the line is read; log line `[dm] [nvn-mtl] [shot] F<n> <path>`). `key <code> <ms>` (ms 1–5000), `down <code>`, `up <code>` → `runtime_input_key_synthetic`, and `key` sets the press edge in `pending`, so the next pad read cannot miss it. `TOTK_INPUT_SCRIPT="sec:keycode:ms,…"` presses at wall times without a file, and turns `TOTK_AUTOLOAD` off. Frontend codes: `J` = 38 is A, `Return` = 36 is Plus. **`close` does not end a headless boot** — both S2.11 runs ended on their timer (exit 124).
- **Build and run.** `make -n runtime shader-tools` from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs` reports nothing to do when the work-tree binary is current. Run from that work tree via `./build/nfs-native rewritten/nfs100 boot` with `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1`, `TMPDIR` on `/Volumes/X9BS/externalTmp/MacNativeGames`, bounded with `timeout -k 5 N`. The work tree is re-derivable, not evidence.
- **Gate and code.** S2.11 changed no code; `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0. S2.9's fix is `0c4e0b9`/`d8a90e4`; S2.10's close is `3d7cda1`; the S2.11 close commit carries this seed.

## Withdrawn — proven wrong, do not use

- "The guest does not react to input." Withdrawn by S2.11: it does.
- "The present stop is a lost wakeup/condition race." Withdrawn for the S2.8/S2.9 stop (the wakeup arrived; the core handoff did not). Do not reapply it to the new all-idle stop before a dump names the pair.
- "The condition key word is 0 in every run." Withdrawn: a waiter is registered; the `0` belonged to the spinning signaller's key and to `0x1002`'s unwaited event.
- "The main thread `0x1001` is the spinning thread." Withdrawn; `0x1002` was.
- "Presenting stops at frame 600 / 354 / 677" as a pinned value. Withdrawn: report the range and the runs (Do-Not #8).
- Numbers **2, 3, 4, 5** — unchanged; do not re-measure. Prior record numbers (8 shaders, frame 441, p50 33.33 ms, GPU p50 0.86–1.45 ms, 255 tests, 721 strings, worst per-window max 996.92 / 510.55 ms, S2.11 run P worst 203.88 ms) are history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Reproduce the stop | No code needed: bounded headless boots with `TOTK_PRESENT_WATCHDOG=2500`, `hletrace 1`/`hletrace 0` brackets, and `threads` / `shot` lines in `TOTK_INPUT_FILE`. |
| Name the pair | `games/nfs/runtime/threads.c` for the dump only, and only once a dump names the waiter/signaller pair; the condition ring and the memory window already exist (S2.9, `0c4e0b9`). |
| Do not | Do not change `core_slice_ns` or `runtime_core_checkpoint`; do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S2.11 — Press ⊕ at the title screen and record what changes".
- `docs/donot.md` #8, #9, #10.
- `.babysteps/scratch/S2.11-press-plus/bootC.log` (the all-idle stop: the dump, the `[cond-trace]` ring, the `[cond-mem]` window); `bootP.log` for contrast.

## Do-Not entries that apply here (3 of 10)

- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.
- #9 — **Do not model a virtual core without rotating equal-priority threads.** Every core read `owner 0` here, so this is not the mechanism — but it must not be weakened while looking.
- #10 — **Do not rest a probe's attribution on a separate control run.** A control that dies before the comparison window is not a control.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes.
- Bounded headless boots with `TOTK_PRESENT_WATCHDOG` that reproduce the all-idle stop or record that they did not, each with its own bound and frame count; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

## When this slice closes

Write the next seed from what S2.12 leaves behind, re-verify the Do-Not count (10 or higher), and commit the state, the feasibility record and the seed.
