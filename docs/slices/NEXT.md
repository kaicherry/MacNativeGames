---
slice: S2.13
stateFile: docs/STATE.md
donotCount: 11
seededFrom: 5994c6d
---

# Next slice — S2.13: fix the all-idle present stop's missing wakeup

## The next action — verbatim

S2.13 — Fix the all-idle present stop. S2.12 reproduced it in 1 of 8 bounded 65 s boots (r4: `[totk-gfx] [nvn] present stop: last presented frame 1218, 1218 submissions, no present for 2.6 s (43.4 s since start)`; the seven others reached frames 1759–1856 with zero stops) and named the pair: the main thread `0x1001` parks untimed (`x3=0xffffffffffffffff`) on the third `nn::os::EventType`'s condition (`addr=0x16b3f48a4 key=0x16b3f48a8`, tag `0x1001`, last host call `nn::oe::TryPopNotificationMessage`) while the frame workers `0x1003`/`0x1004` — which in every healthy run's dumps signal that key and wake `0x1001` (`signal key=… by=0x1003|0x1004 woke=0x1001`) — cycle on their own condition keys with ~33 ms timeouts (x3 ≈ 0x1f78a0b ns) and emit no signal at all across r4's three dumps (43.4 s, t=45 s, t=55 s). Establish what the workers stop doing and why the signal that wakes `0x1001` never comes, from the SDK worker loop (`main+0x756e64` calling `TimedWaitEvent` at `sdk+0x33906c`) and the runtime's condition emulation (`games/nfs/runtime/threads.c`, the `0x1C`/`0x1D` cases) read against the record; instrument before changing behaviour and fix only from the named cause — do not weaken the S2.9 equal-priority rotation (Do-Not #9), and do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim.

S2.13 closes when the cause of the missing wakeup is named from a recorded observation (a dump, a trace, or SDK code read against the record), a fix is made, and bounded headless boots with `TOTK_PRESENT_WATCHDOG=2500` present through the stop's window (39.7–43.4 s) and past it with zero stops across N runs, each with its bound and frame count; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The stop reproduces.** 1 of 8 bounded 65 s runs (r4, 43.4 s, frame 1218); 2 of 14 across the post-fix runs (39.7 s / frame 1105 in S2.11 run C; 43.4 s / frame 1218 in r4); the seven healthy S2.12 runs reached frames 1759–1856 at their 65 s bound.
- **The pair.** The main thread `0x1001` waits untimed on the third `nn::os::EventType`'s condition; the frame workers `0x1003`/`0x1004` (sometimes `0x1002`) signal that key and wake it in every healthy dump; in r4 no signal event appears in any of the three rings (~15 s) while the four frame-loop threads cycle ~33 ms timeouts on their own keys.
- **r4's stopped state persists and never resumes.** No `[shot]`; the second hletrace window printed `0 calls to 0 functions`; three dumps the same shape; every core `owner 0`; load `T 0%`.
- **The instrument set that works.** The S2.12 control schedule and env (`TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1 TOTK_PRESENT_WATCHDOG=2500 TOTK_INPUT_LOG=1 TOTK_INPUT_FILE=<file>` from the work tree); `threads` dumps at t=45/55 s catch the healthy state, the watchdog's own dump catches the stop; the end `shot` names the presented frame; `hletrace 1`…`hletrace 0` brackets a window exactly.
- **Healthy baseline.** At t=45 s healthy runs show the main<->worker signal exchange in the ring; `0x1002` can hold core 0 (core waiter) and `0x1004` can wait on `0x1002`'s lock — mid-flight, not all-parked.
- **Not the S2.9 shape.** No spin, all cores idle; the S2.9 rotation is untouched.

## Withdrawn — proven wrong, do not use

- "The guest does not react to input." Withdrawn by S2.11: it does.
- "The present stop is a lost wakeup/condition race." Withdrawn for the S2.8/S2.9 stop (the wakeup arrived; the core handoff did not) — that stays withdrawn. With the pair now named from a dump, the missing-signal hypothesis for the new all-idle stop may be tested against the record.
- "The condition key word is 0 in every run." Withdrawn: a waiter is registered; the `0` belonged to the spinning signaller's key and to `0x1002`'s unwaited event.
- "The main thread `0x1001` is the spinning thread." Withdrawn; `0x1002` was.
- "Presenting stops at frame 600 / 354 / 677 / 1105 / 1218" as a pinned value. Withdrawn: report the range and the runs (Do-Not #8).
- Numbers **2, 3, 4, 5** — unchanged; do not re-measure. Prior record numbers (8 shaders, frame 441, p50 33.33 ms, GPU p50 0.86–1.45 ms, 255 tests, 721 strings, worst per-window max 996.92 / 510.55 ms, S2.11 run P worst 203.88 ms, S2.12 end frames 1759/1781/1801/1851/1856/1851/1802) are history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Name the cause | `games/nfs/runtime/threads.c` (`0x1C`/`0x1D` condition emulation: `block_inner`, `pick`, `wake`, `lock_handoff`, `ctrace_push`) read against r4's dump, and the SDK worker loop `main+0x756e64` (`TimedWaitEvent` `sdk+0x33906c`) as needed. |
| Instrument | Only if the record cannot name the cause; no behaviour change in the instrumenting commit. |
| Fix + verify | The smallest change in `games/nfs/runtime/threads.c`, verified by bounded boots with `TOTK_PRESENT_WATCHDOG=2500` presenting past 43.4 s with zero stops. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S2.12 — Diagnose the all-idle present stop".
- `docs/donot.md` #8, #9, #10, #11.
- `.babysteps/scratch/S2.12-all-idle-stop/r4.log` (the reproduced stop: three dumps, the timeout-only rings); `r1.log` (healthy contrast: the `signal … woke=0x1001` lines).

## Do-Not entries that apply here (4 of 11)

- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.
- #9 — **Do not model a virtual core without rotating equal-priority threads.** Every core read `owner 0` here, so this is not the mechanism — but it must not be weakened while looking.
- #10 — **Do not rest a probe's attribution on a separate control run.** A control that dies before the comparison window is not a control.
- #11 — **Do not close a slice by appending its record while the STATE.md top sections still name the finished slice.** Advance `Where the work is`, `Next action` + gate + closes-when + Do-Not count, `Completed`, `In progress` and `Queued` in the same commit as the record.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 today).
- Bounded headless boots with `TOTK_PRESENT_WATCHDOG=2500` that present through the stop's window and past it with zero stops; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.13 leaves behind, re-verify the Do-Not count (11 or higher), and commit the state, the feasibility record and the seed.
