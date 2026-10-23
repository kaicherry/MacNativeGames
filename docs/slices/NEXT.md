---
slice: S2.14
stateFile: docs/STATE.md
donotCount: 12
seededFrom: 1248923
---

# Next slice — S2.14: account the frame-completion semaphores' credits

## The next action — verbatim

S2.14 — Account the frame-completion semaphores' credits. S2.13 caught the stop's transition for the first time (r17, 59.7 s, frame 1706; 1 of 20 bounded 65 s boots) and replaced the lost-wakeup hypothesis with a measured one: main `0x1001` kicked all three frame workers (`signal key=0x78f284e828 by=0x1001 woke=0x1003`, `signal key=0x78f284e7a8 by=0x1001 woke=0x1004`, `signal key=0x78f284e8a8 by=0x1001 woke=0x1002`), parked untimed (`x3=0xffffffffffffffff`) on `addr=0x16bc68b14 key=0x16bc68b18`, and nothing touched that key again — the ring's remaining 562 entries (2.58 s) are only the four workers' `TimedWaitEvent` timeout+wait cycles. Each worker was woken by main's kick and went straight back to its own `TimedWaitEvent`, producing no completion at all. The object is an `nn::os::Semaphore` (base `0x16bc68af8`: `[+0x10]=1`, `[+0x14]=count=0`, `[+0x18]=max=0x7fffffff`, cs word 0, cv word 1) reached through `nn::os::AcquireSemaphore` (`sdk+0x33b8e0`; stack `sdk+0x33b918 <- sdk+0x55c0fc <- main+0x743c10`), whose count is read and decremented under the same critical section that `ReleaseSemaphore` (`sdk+0x33ba80`) increments it in — so a release cannot be lost, and the workers are awake and polling rather than blocked (`main+0x756e64` discards `TimedWaitEvent`'s result). Over the 3.9 s ring only `0x1001` ever waits on a completion key (240 waits on `0x16bc68b18`) while only the workers signal it (240 signals: `0x1002` x70, `0x1003` x89, `0x1004` x81), and `0x1001` also signals it 173 times with `woke=0`; a worker must therefore be taking those credits through `TryAcquireSemaphore` (`sdk+0x33b940`), which issues no SVC and so leaves no trace. Record that trace — the count at every `ReleaseSemaphore`/`TryAcquireSemaphore`/`AcquireSemaphore` on the completion semaphores, and in the stall dump which thread last decremented each one — and name the consumer that took a credit and returned none, from the widened ring's transition (`.babysteps/scratch/S2.13-missing-wakeup/r17.log`, third dump, ring spanning `-3938.0` to `0 ms`; worker loop `main+0x756e64`; `games/nfs/runtime/threads.c`); instrument before changing behaviour and fix only from the named cause — do not weaken the S2.9 equal-priority rotation (Do-Not #9), and do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim.

S2.14 closes when the credit consumer is named from a recorded observation (a trace, or SDK/game code read against the record), a fix is made, and bounded headless boots with `TOTK_PRESENT_WATCHDOG=2500` present through the stop's window (39.7–59.7 s across the post-fix runs) and past it with zero stops across N runs, each with its bound and frame count; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The transition, captured for the first time.** r17's third dump (the watchdog at 59.7 s) has a ring spanning `-3938.0 ms` to `0 ms`; its last events on the completion key are `-2579.7 ms signal key=0x78f284e828 by=0x1001 woke=0x1003`, `… key=0x78f284e7a8 by=0x1001 woke=0x1004`, `… key=0x78f284e8a8 by=0x1001 woke=0x1002` and `-2579.7 ms wait key=0x16bc68b18 by=0x1001`. Nothing touches `0x16bc68b18` again; the remaining 562 entries (2.55 s) are the four workers' `timeout`+`wait` cycles (~33 ms each) on their own keys.
- **Counts over the 3.9 s ring.** `0x1001` 1256 signal / 364 wait; `0x1002` 160/467/70; `0x1003` 228/493/69; `0x1004` 210/493/70; `0x1005` 108 wait / 108 timeout. Per completion key: `0x16bc68b18` 240 waits by `0x1001` against 240 worker signals (`0x1002` x70, `0x1003` x89, `0x1004` x81) plus 173 `woke=0` signals by `0x1001`; `0x16bc68b28` 83/83; `0x16bc68d58` 41/41. Only `0x1001` waits on a completion key; the workers only signal it.
- **The object is a semaphore, not a stateless cv.** Base `0x16bc68af8` reads `[+0x10]=1`, `[+0x12]=0`, `[+0x14]=count=0`, `[+0x18]=max=0x7fffffff`, cs `0`, cv word `1`; reached via `nn::os::AcquireSemaphore` (`sdk+0x33b8e0`; stack `sdk+0xd884dc <- sdk+0x33b918 <- sdk+0x55c0fc <- main+0x743c10`). S2.12's r4 decodes the same (`0x16b3f4888`).
- **A release cannot be lost.** `AcquireSemaphore` is `cs.Enter(); while (count == 0) cv.Wait(cs); count--; cs.Leave()`; `ReleaseSemaphore` (`sdk+0x33ba80`) is `cs.Enter(); count++; cv.Signal(); cs.Leave()`. The count is the state, touched under one critical section, so the missing event is a release that was never issued.
- **The workers are awake and polling.** Woken by main's kick (`woke=0x1003`/`0x1004`/`0x1002`), each went straight back to its own `TimedWaitEvent` with no completion; `main+0x756e64` overwrites `TimedWaitEvent`'s result in `w0`, so the body re-runs every ~33 ms (`x3 ≈ 0x1f78a0b` ns) regardless of signal.
- **The SDK protocol, read against the record.** `Signal()` (`sdk+0x343f20`) is `orr w1, wzr, #0x1; b <svc #0x1D>` — unconditional, no user-mode key read; `Wait`/`TimedWait` (`sdk+0x343f40`/`0x343fe0`) issue `svc #0x1C`; `Enter`/`Leave` (`sdk+0x344520`/`0x344610`) are user-mode `ldaxr/stlxr` reaching `svc #0x1A`/`0x1B` only on the `0x40000000` waiters bit. The `0x1C` case comment's stated reason (the SDK reads the condition key in user mode) does not describe this title's SDK.
- **Not the S2.9 shape.** All three cores `owner 0`; no `core waiter` line (`g_core_wait` empty); every waiting thread `wkind=2` with cs word `0`, cv word `1`; no lock waiters. The S2.9 rotation is untouched (Do-Not #9).

## Withdrawn — proven wrong, do not use

- **"No signal event appears in r4's rings" as evidence about the missing wakeup.** Withdrawn: the ring was 0.6 s while the transition sits 2.6 s before the dump (Do-Not #12).
- **"The present stop is a lost wakeup/condition race" for this stop.** Withdrawn: the object is a count-based semaphore, so a release cannot be lost.
- **"The workers stop being woken" / "the workers are parked".** Withdrawn: main's kick woke all three (`woke=0x1003`/`0x1004`/`0x1002`); the workers are awake and polling.
- Numbers **2, 3, 4, 5** — unchanged; number **1** is **12** now. Prior record numbers (8 shaders, frame 441, p50 33.33 ms, GPU p50 0.86–1.45 ms, 255 tests, 721 strings, S2.12 end frames 1759/1781/1801/1851/1856/1851/1802) are history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Account the credits | `games/nfs/runtime/threads.c` and the stall dump — the count at every `ReleaseSemaphore`/`TryAcquireSemaphore`/`AcquireSemaphore` on the completion semaphores (`0x16bc68b18`, `0x16bc68b28`, `0x16bc68d58`), and in the dump which thread last decremented each one. |
| Instrument | Only to record the credit consumer; no behaviour change in the instrumenting commit. |
| Fix + verify | The smallest change at the named cause, verified by bounded boots with `TOTK_PRESENT_WATCHDOG=2500` presenting past 59.7 s with zero stops. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `.babysteps/scratch/S2.13-missing-wakeup/r17.log` — the stopped run: the widened ring's transition at the third dump (the last events on `0x16bc68b18` at `-2579.7 ms`), the thread dump, and the `cond-mem` block for `0x16bc68af8`.
- `.babysteps/scratch/S2.13-missing-wakeup/sdk-sem.dis` (`AcquireSemaphore`/`ReleaseSemaphore`), `sdk-icv.dis` (`Signal`/`Wait`/`TimedWait`), `sdk-ics.dis` (`Enter`/`Leave`).
- `docs/donot.md` #8, #9, #10, #11, #12.

## Do-Not entries that apply here (5 of 12)

- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.
- #9 — **Do not model a virtual core without rotating equal-priority threads.** Every core read `owner 0` here, so this is not the mechanism — but it must not be weakened while looking.
- #10 — **Do not rest a probe's attribution on a separate control run.** A control that dies before the comparison window is not a control.
- #11 — **Do not close a slice by appending its record while the STATE.md top sections still name the finished slice.** Advance `Where the work is`, `Next action` + gate + closes-when + Do-Not count, `Completed`, `In progress` and `Queued` in the same commit as the record.
- #12 — **Do not size a stall's trace window from the dump.** Size it from the stop's own lateness — the dump lands 2.6 s after the stop begins.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 today).
- Bounded headless boots with `TOTK_PRESENT_WATCHDOG=2500` that present through the stop's window and past it with zero stops; every claim tied to a recorded observation; no gameplay claim.

## When this slice closes

Write the next seed from what S2.14 leaves behind, re-verify the Do-Not count (12 or higher), and commit the state, the feasibility record and the seed.
