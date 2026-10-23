---
slice: S2.15
stateFile: docs/STATE.md
donotCount: 14
seededFrom: 956cc51
---

# Next slice — S2.15: Name the thread that takes the credit `0x1001` posts

## The next action — verbatim

S2.15 — Name the thread that takes the credit `0x1001` posts on the frame-completion semaphores. S2.14 built the
credit ledger and the account now closes on 13 clean 65 s boots (r1–r13, zero stops): on each of the three
completion semaphores the surplus of releases over blocking acquires equals `0x1001`'s own releases, `0x1001`'s
blocking acquires equal the three frame workers' releases, and the workers' own acquires are zero (3 keys × 13
runs, all off by at most 1). So no credit is taken by a third party: `0x1001` itself posts a credit
(`ReleaseSemaphore`, `woke=0` — the host found no cv waiter) and the credit leaves again with no blocking acquire,
which is what a fast-path `AcquireSemaphore` or a `TryAcquireSemaphore` (`sdk+0x33b940`: `cs.Enter(); count--;
cs.Leave()`) does — and that path issues no SVC, so the host recorded the take only as a count. The ledger's
sound cross-critical-section attribution (`last_dec`, set only when a 0x1A/0x1B pair bounds the drop) must be read
for these keys in a stopped run and reported; if it stays 0 there, only one route is left — make the guest's
acquire fast path visible (the `games/nfs/third_party/d2r` rewriter, or `runtime_game_patches(Module *)`'s hook,
extended to the `sdk` module) — and name the thread that leaves the semaphore's critical-section word with the
count lower. Instrument before changing behaviour; do not weaken the S2.9 equal-priority rotation (Do-Not #9);
do not touch `games/totk` or `games/by3`.

S2.15 closes when the thread that takes the posted credit is named from a recorded observation (the fast path made
visible, or `last_dec` on these keys), and either that names a defect site and a fix follows with bounded headless
boots that present through the stop's window and past it with zero stops across N runs, each with its bound and
frame count; or the probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The account closes, and it is the slice's result.** Per completion key, over 13 clean runs: `rel − acq` = `0x1001`'s own `rel` (off by at most 1), `acq` = the three workers' `rel` (off by at most 1), workers' `acq` = 0, and `count` = 0 or 1. r1 `…b18`: rel 11845 / acq 7026 / `0x1001` 4819 rel, 7026 acq / workers 7026 rel, 0 acq. r13 `…b18`: 12430 / 7302 / 5128 / 7302 / 0. r1 `…d58`: 1577 / 1564 / 13 / 1564 / 0. r1 `…b28`: 2369 / 2360 / 9 / 2360 / 0.
- **The three completion semaphores per run** (ASLR moves the high bits; these are the low-bit pattern): base `…af8` / cv `…b18`, base `…b08` / cv `…b28`, base `…d38` / cv `…d58`. Base = cv − 0x20, count at base+0x14, max at base+0x18, cs word at base+0x1c (= cv − 4), cv at base+0x20. `nn::os::Semaphore` is identified by `max == 0x7fffffff`; do not hardcode addresses.
- **The instrument (no behaviour change).** `games/nfs/runtime/threads.c`: `ctrace_push` carries `struct sem_obs { count, holder }` read at every 0x1C/0x1D on a cv and every 0x1A/0x1B on a contested cs; `holder` is the tag in the cs word; coalescing no longer folds a repeat on the same key when the count moved (a re-wait must survive); and one ledger row per semaphore with per-thread `rel` (cv signals), `acq` (blocking acquires), `enter` (contested cs entries), plus totals, printed as `[sem-trace]` rows with `by=` lines.
- **`0x1001` both waits and posts.** `0x1001`: rel 4819 / acq 7026 on `…b18` in r1 — it blocks for every completion and also returns credits with `woke=0`. The workers `0x1002`/`0x1003`/`0x1004` never block on a completion key and only release; `0x1005` polls its own key.
- **The take leaves no event.** SDK `AcquireSemaphore` (`sdk+0x33b8e0`) fast path and `TryAcquireSemaphore` (`sdk+0x33b940`) issue no SVC; `InternalCriticalSectionImplByHorizon::Enter`/`Leave` (`sdk+0x344520`/`0x344610`) are user-mode `ldaxr/stlxr` and reach `svc #0x1A`/`0x1B` only with the `0x40000000` waiters bit. So the count is the take's only trace.
- **Rate, measured clean.** With the instrument in place and the machine quiet: 13 bounded 65 s boots, **0 stops**, t=62 s `[shot]` frames 1851–1855 (`.babysteps/scratch/S2.14-credit-trace/probe-summary.txt`). S2.13's uninstrumented sample: 1 stop in 20, frames 1651–1853. The instrument's effect on the rate is **unmeasured** — do not claim one.
- **The stop's own record is unchanged and is S2.13's** (uninstrumented r17): `present stop: last presented frame 1706 ... (59.7 s since start)`; main kicks all three frame workers on their own keys and parks untimed on `addr=0x16bc68b14 key=0x16bc68b18`; nothing touches that key again; the workers were woken and each went straight back to its own `TimedWaitEvent` (`main+0x756e64` discards its result) producing no completion.
- **Not the S2.9 shape.** All three cores `owner 0`; no `core waiter` line; every waiter `wkind=2`; no lock waiters. The rotation is untouched.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0, 22.562 s today.

## Withdrawn — proven wrong, do not use

- **"A worker must be taking those credits through `TryAcquireSemaphore`" (S2.13's hypothesis, and S2.14's board).** Withdrawn by the account above: the surplus equals `0x1001`'s own releases and the workers' releases equal `0x1001`'s blocking acquires, so nothing was taken by a third party.
- **"The credit trace is a load: it raised the stop rate from 1/20 to 7/20 and cost ~20% of the frames."** Withdrawn — that batch ran while another local process held the GPU (Do-Not #14). Clean: 0 stops in 13, frames 1851–1855 against the uninstrumented 1651–1853.
- **`rewait` and `unattributed` as evidence of a lost credit.** Withdrawn: both fire on the healthy loop (r1 `…b18` 2295 / 6533 of 7026 blocking acquires; `…d58` 1550 / 1562 of 1564). Keep them recorded; do not read them as the fault (Do-Not #13).
- Still withdrawn from S2.13: "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the present stop is a lost wakeup/condition race"; "the workers stop being woken" / "the workers are parked".
- Numbers **1** is **14** now; **8, 9, 10, 11** unchanged. Prior record numbers (8 shaders, frame 441, p50 33.33 ms, 255 tests, 721 strings, S2.13's r17 59.7 s / frame 1706) are history, not re-derived. The discarded GPU-contended batch's 7 stops (r6 48.8 s / 1211, r8 44.1 / 1215, r9 55.4 / 1545, r12 19.4 / 490, r14 28.4 / 755, r17 32.9 / 763, r18 23.3 / 599) and its smoke's stop (19.6 s / 480) are **not** usable numbers.

## Code to touch

| Work | Touch |
| --- | --- |
| Read `last_dec` for the completion keys | `games/nfs/runtime/threads.c` — print `last_dec`/`last_dec_delta` per ledger row and read them in a stopped run (`.babysteps/scratch/S2.14-credit-trace/summary.py` reads the rows already). |
| Make the take visible if `last_dec` stays 0 | The guest acquire fast path: `games/nfs/third_party/d2r/` (the rewriter) or `runtime_game_patches(Module *)` in `games/nfs/runtime/`, extended to the `sdk` module's `AcquireSemaphore`/`TryAcquireSemaphore` entries. |
| Read the producer against the record | `main+0x756e64` (the worker loop) and its path to the completion `ReleaseSemaphore` — does the body reach a completion when it runs? `main.elf` is at `…/nfs-work/nfs/extracted/nfs100/elf/`. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`; no behaviour change in the instrumenting commit. |

## Read these first, and only these

- `.babysteps/scratch/S2.14-credit-trace/probe-summary.txt`, one stopped-free run's `[sem-trace]` block and `[cond-trace]` tail (e.g. `r1.log`), and `summary.py` — the ledger reader.
- `.babysteps/scratch/S2.13-missing-wakeup/{r17.log,main-756e64.dis,sdk-sem.dis,sdk-ics.dis}` — the stop's transition, the worker loop, and the semaphore/critical-section disassembly.
- `docs/donot.md` #8, #9, #10, #11, #12, #13, #14.

## Do-Not entries that apply here (7 of 14)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #9 — do not model a virtual core without rotating equal-priority threads.
- #10 — do not rest a probe's attribution on a separate control run.
- #11 — do not close a slice by appending its record while the STATE.md top sections still name the finished slice.
- #12 — do not size a stall's trace window from the dump.
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 today).
- Bounded headless boots (`probe.sh <n> 65` from `.babysteps/scratch/S2.15-*/`, the S2.12/S2.13 control schedule unchanged) on a machine with the GPU free — verify that before the batch, and discard the batch if another process held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.15 leaves behind, re-verify the Do-Not count (14 or higher), and commit the state,
the feasibility record and the seed.
