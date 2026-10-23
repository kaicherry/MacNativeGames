---
slice: S2.16
stateFile: docs/STATE.md
donotCount: 15
seededFrom: 3e0a4ab
---

# Next slice — S2.16: Read the producer — does the frame worker's loop body reach a completion when it runs?

## The next action — verbatim

S2.16 — Read the producer: does the frame worker's loop body reach a completion when it runs? S2.15 closed the credit
account: the trap on the SDK's three semaphore count stores (`udf #0` at `sdk+0x33b924`, `sdk+0x33b96c`,
`sdk+0x33ba0c`; the SIGILL handler performs the store from the faulting thread's registers and records the take) names
`0x1001` as the only consumer of every frame-completion credit in 12 clean 65 s boots — `takes` equals `rel` on all four
completion-shaped keys (48/48 rows; 248 of 249 ledger rows read `takes − rel` within the live count), the three frame
workers' `take` is 0 there in all 12 runs, and `takes − acq` on each key equals the consumer's own `rel` (r1 `…cc8`
acq 7069 / takes 11920 / own rel 4851: 11920 − 7069 = 4851). So the surplus S2.14 could not attribute is `0x1001`'s
own fast-path re-acquire of its own post: no credit is stolen and none is lost. The trap is not blind to a cross-thread
take — on the shared semaphore `0x…e9b0` `0x1008` takes 560 while posting 327, and the 233 difference is exactly
`0x1001`'s 233 releases there — so its zero for the workers is a measurement. What is left is why a woken worker runs
and produces no completion: read the worker loop's body (`main+0x756d70`, dequeuing from `[x23+0x10]` and
`[x23+0x50]`; `main+0x756e60`, the timed wait on the worker's own event `[x22+0x48]` with a timeout of
`[x23+0x1528]` ms) down to the completion `ReleaseSemaphore` (`sdk+0x33ba80`) if the body reaches one, and name the
branch that skips it. Instrument before changing behaviour; do not weaken the S2.9 equal-priority rotation (Do-Not #9);
do not read `rewait`/`unattributed` as the fault (Do-Not #13); do not touch `games/totk` or `games/by3`.

S2.16 closes when the worker loop's path to the completion `ReleaseSemaphore` — or the branch that skips it — is named
from the disassembly with the register state that selects it, and either that names a defect site and a fix follows with
bounded headless boots that present through the stop's window and past it with zero stops across N runs, each with its
bound and frame count; or the read fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The trap (S2.15, the slice's instrument).** `games/nfs/runtime/{game_patches.c,loader.c,threads.c,runtime.h}`, no
  behaviour patch and no new file. The SDK's three semaphore count stores are `udf #0` before the text is sealed
  (`sdk+0x33b924` and `sdk+0x33b96c` = `str w8,[x20,#0x14]`, `sdk+0x33ba0c` = `str w8,[x19,#0x14]`), written only when
  the loaded word equals the verified one; the SIGILL handler performs that store from the faulting thread's registers
  and records the take. `runtime_game_patches` now runs for `sdk` as well as `main`. The ledger's `takes=` (row total)
  and `take=` (per thread) are that record.
- **The answer: `0x1001` is the only consumer of every frame-completion credit.** 12 clean 65 s boots
  (`.babysteps/scratch/S2.15-take-trap/probe-summary.txt`): `stops=0`, `badcmd=0`, t=62 s `[shot]` frame 1800–1854
  (S2.14's 1790–1855). r1 `…cc8`: acq 7069 / rel 11920 / takes 11920; `by=0x1001 rel 4851 acq 7069 take 11920`;
  workers `0x1002` 2869 rel, `0x1003` 2249, `0x1004` 1951, all `take=0`. r1 `…cd8`: acq 2371 / rel 2384 / takes 2384,
  `0x1001` own rel 13. r1 `…f08`: acq 1576 / rel 1586 / takes 1586, own rel 10. The `0x1019` key `…2e08`: acq 3018 /
  takes 3017, own rel 0. So `takes − acq` = the consumer's own `rel` on each key, in all 12 runs.
- **No credit is stolen or lost.** `takes` equals `rel` on all four completion-shaped keys in 48/48 rows; 248 of 249
  ledger rows read `takes − rel` within the live count (−1: 84, 0: 153). `last_dec` stays 0 on those keys in 12/12 runs.
- **The trap does report a cross-thread take.** On the shared semaphore `0x…e9b0`, `0x1008` takes 560 while posting
  327, and the 233 difference is exactly `0x1001`'s 233 releases there — so its `take=0` for the workers is a
  measurement, not a blind spot.
- **The worker loop's shape (S2.13's read, unchanged).** `main+0x756d70` dequeues from `x24=[x23+0x10]` and
  `x25=[x23+0x50]`; `main+0x756e60` is the timed wait on the worker's own event `x26=[x22+0x48]` with a timeout of
  `[x23+0x1528] × 0x0f4240` ns; `main+0x756e64` overwrites the wait's result, so a missed kick self-heals inside one
  ~33 ms interval and cannot be the mechanism.
- **The stop's own record is S2.13's and is unchanged** (uninstrumented r17): `present stop: last presented frame 1706
  ... (59.7 s since start)`; main kicks all three frame workers on their own keys and parks untimed on the completion
  key; nothing touches it again; the workers were woken and each went straight back to its own `TimedWaitEvent`.
- **Not the S2.9 shape.** All three cores `owner 0`; no `core waiter` line; every waiter `wkind=2`; no lock waiters.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1
  skipped, exit 0, 22.619 s today.
- **The work tree** `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable, not evidence; the runtime was rebuilt from `games/nfs/runtime/` with `make -C games/nfs WORK=<work>/nfs runtime`.

## Withdrawn — proven wrong, do not use

- **"A worker must be taking those credits through `TryAcquireSemaphore`" (S2.13's hypothesis).** Now **refuted by
  observation**, not merely withdrawn: `take=0` for `0x1002`/`0x1003`/`0x1004` on every completion key in every run.
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Its conclusion holds, its
  argument does not: both sides of that identity are invariant to who takes, so it cannot exclude a taker (Do-Not #15).
- **`rewait`/`unattributed` as evidence of a lost credit.** Both fire on the healthy loop (Do-Not #13). Still withdrawn.
- **Still withdrawn from S2.13:** "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the present stop is
  a lost wakeup/condition race"; "the workers stop being woken" / "the workers are parked".
- **One row S2.15 does not explain:** the one-shot semaphore `0x…3db8` (one signal each from `0x1010` and `0x1019`,
  count 1) reads `rel=2 takes=0` in all 12 runs. `rel` counts the cv's `SignalProcessWideKey` SVCs, so a signal that is
  not a `ReleaseSemaphore` inflates it. Recorded, not explained; it is off the completion keys.
- Numbers **1** is **14**; **8, 9, 10, 11** unchanged; **15** is new. Prior record numbers (8 shaders, frame 441,
  p50 33.33 ms, 255 tests, 721 strings, S2.13's r17 59.7 s / frame 1706) are history, not re-derived. S2.14's discarded
  GPU-contended batch is **not** usable numbers.

## Code to touch

| Work | Touch |
| --- | --- |
| Read the worker loop's body | `main+0x756d70`..`main+0x756e7c` in `.babysteps/scratch/S2.13-missing-wakeup/main-756e64.dis`: follow the dequeued work (x24/x25) and the body calls, and find whether the body reaches the completion `ReleaseSemaphore` (`sdk+0x33ba80`). `main.elf` is at `…/nfs-work/nfs/extracted/nfs100/elf/`. |
| Read the kick against the record | main's release on the completion key (`woke=0`): is it aimed at a worker, and does a caller in main then wait on the same key? |
| Re-read the ledger if needed | `games/nfs/runtime/threads.c` prints `takes=`/`take=`; `.babysteps/scratch/S2.15-take-trap/summary.py` reads the rows. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`; no behaviour change in an instrumenting commit. |

## Read these first, and only these

- `.babysteps/scratch/S2.15-take-trap/{probe-summary.txt,summary.py,r1.log}` — the ledger with `takes=`/`take=`.
- `.babysteps/scratch/S2.13-missing-wakeup/{main-756e64.dis,r17.log,sdk-sem.dis}` — the worker loop, the stop's transition, and the semaphore/critical-section disassembly.
- `docs/donot.md` #8–#15.

## Do-Not entries that apply here (8 of 15)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #9 — do not model a virtual core without rotating equal-priority threads.
- #10 — do not rest a probe's attribution on a separate control run.
- #11 — do not close a slice by appending its record while the STATE.md top sections still name the finished slice.
- #12 — do not size a stall's trace window from the dump.
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped,
  exit 0 today).
- Bounded headless boots (`probe.sh <n> 65` from `.babysteps/scratch/S2.16-*/`, the S2.12/S2.13 control schedule
  unchanged) on a machine with the GPU free — verify that before the batch, and discard the batch if another process
  held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.16 leaves behind, re-verify the Do-Not count (15 or higher), and commit the state
and the seed.
