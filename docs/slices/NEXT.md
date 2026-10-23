---
slice: S2.10
stateFile: docs/STATE.md
donotCount: 9
seededFrom: d8a90e4
---

# Next slice — S2.10: capture what the boot presents past the old stop, and the next stop

## The next action — verbatim

S2.10 — With the condition-variable stop fixed, capture what the boot presents past the old stop range and record what the next stop is, if any. S2.9 named the object and the pair (worker `0x1002` spinning/signal-waiting on the third `nn::os::EventType` at `heap+0xC284E888`, cv at `+0xC284E8A8`; main `0x1001` woken onto an event critical section and starved for core 0) and fixed the core handoff with an equal-priority slice; runs then presented `timing to frame 2400` with zero present stops in 90 s, against the prior record of stops at 354–677. Take bounded headless captures past frame 677 (the `shot <path>` line in `TOTK_INPUT_FILE`, as in S2.6) and record what the picture shows and whether a new stop appears; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

S2.10 closes when a capture past frame 677 exists and what it shows is recorded, and either no new stop appears in the bounded run or the next stop is recorded with its own message and the command that produced it; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The stop was a core-handoff failure, not a lost signal.** Boot1/boot2: main `0x1001` (mask `0x1` — core 0 only) was woken from `ArbitrateLock` (word `0x40001001`, owner `0x1001` with waiters) and waited in `core_take` for the whole 2.6 s gap (`core waiter 0x1001 prio 44 mask 0x1`) while worker `0x1002` held core 0 for the same 2.6 s and spun `0x1D` 11,095,006 times (boot1; 11,108,312 in boot2) with no `SleepThread` and no block. `0x1003`/`0x1004` then queued behind the starved main thread's critical section. The wakeup arrived; the core handoff did not (Do-Not #9).
- **The object is named.** Three 0x80-byte `nn::os::EventType` objects at `heap+0xC284E788`, `+0xC284E808`, `+0xC284E888` (cs at +0x1c, cv at +0x20 — the layout `SignalEvent` uses at `sdk+0x338e30`). The seed's key `heap+0xC284E8A8` is the third event's cv; its cs at `heap+0xC284E8A4` read `0x1002` (thread `0x1002` inside it). `TimedWaitEvent` (`sdk+0x33906c`, from `main+0x756e60`) waits; `SignalEvent` (`sdk+0x338e50`, via the refcounted closure runner `main+0x756720`/`0x7568e0` from `main+0x756798`) signals. The trace shows `signal key=...E8A8 by=0x1002 woke=0` (no waiter) while `0x1005` waits and times out on `...E858a8` every 37 ms.
- **The fix.** `core_take` marks `yield_eq` on equal-priority co-core owners; `runtime_core_checkpoint` (SVC entry, `services.c:307`) hands the core over once the holder has run `core_slice_ns` = 4 ms; higher-priority preemption is unchanged. Do not change `core_slice_ns` as part of a capture slice.
- **Verified state.** `boot3.log`: 90 s, zero present stops, `timing to frame 2400`, p50 33.33 ms; `boot4.log`: 30 s, frame 600, zero stops. The prior stop range 354–677 is superseded; report ranges, not one run's frame (Do-Not #8).
- **Diagnostics available.** `TOTK_PRESENT_WATCHDOG=<ms>` (unset/0 off; `1` = 1500 ms) prints one `[nvn] present stop: last presented frame …` line at the first present gap and calls the thread dump; the dump now also prints the condition ring (`[cond-trace]`, last 24 waits/signals/timeouts with age) and a 0x100-byte window per waiting lock (`[cond-mem]`, `<-lock`/`<-key` markers, module pointers resolved). All three fire at a stop, never in normal play.
- **Capture.** The input file's `shot <path>` line writes the current presented frame as PNG (S2.6, headless). The frame-441 capture (S2.6, `.babysteps/scratch/S2.6-NFS-first-frame/f15.png`) remains the only visual record, and no gameplay is established.
- **Build and run.** Build from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs`; run from the work tree `./build/nfs-native rewritten/nfs100 boot` with `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1`, `TMPDIR` on `/Volumes/X9BS/externalTmp/MacNativeGames`, bounded with `timeout -k 5 N`. The work tree is re-derivable, not evidence.
- **Gate and code.** S2.9 commits `0c4e0b9` (condition ring + memory window) and `d8a90e4` (rotation); `python -m unittest discover -s tests` passed 255 tests, 1 skipped.

## Withdrawn — proven wrong, do not use

- "The present stop is a lost wakeup/condition race." Withdrawn: the wakeup arrived (owner word `0x40001001`) and the woken thread could not get its core (Do-Not #9).
- "The condition key word is 0 in every run." Withdrawn: S2.9's windows read the waiters' keys as `0x1` (a waiter is registered); the `0` belonged to the spinning signaller's key and to `0x1002`'s unwaited event.
- "The main thread `0x1001` is the spinning thread." Withdrawn; `0x1002` is the created guest thread.
- "Presenting stops at frame 600 / 354 / 677" as a pinned value. The stop is not pinned to a frame; report the range and the runs (Do-Not #8).
- Numbers **2, 3, 4, 5** — unchanged from earlier seeds; do not re-measure. Prior record numbers (8 shaders, frame 441, p50 33.33 ms; GPU p50 0.86–1.45 ms; 255 tests; 721 strings) are quoted as history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Capture | `TOTK_INPUT_FILE` `shot <path>` lines (mechanism in `games/nfs/runtime/input.c`); PNGs into the slice's scratch dir. |
| Next stop | Only if a new stop appears: the file its message names; `games/nfs/runtime/threads.c` only if the stop is a synchronization stop, and name its waiter/signaller pair from a dump before changing anything. |
| Do not | Do not change `core_slice_ns` or `runtime_core_checkpoint` in a capture slice; do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S2.9 — Name the condition object and fix the lost wakeup".
- `docs/donot.md` #8, #9.
- `.babysteps/scratch/S2.9-cond-object/boot3.log` (the clean 90 s run), `boot1.log`/`boot2.log` (the stop and its dump).

## Do-Not entries that apply here (2 of 9)

- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.
- #9 — **Do not model a virtual core without rotating equal-priority threads.** A woken sibling that can only run on that core is the same as no wakeup.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes.
- A bounded headless boot with `TOTK_PRESENT_WATCHDOG` and a capture past frame 677; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

## When this slice closes

Write the next seed from what S2.10 leaves behind, re-verify the Do-Not count (9 or higher), and commit the state, the feasibility record and the seed.
