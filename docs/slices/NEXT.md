---
slice: S2.17
stateFile: docs/STATE.md
donotCount: 16
seededFrom: 84be907
---

# Next slice — S2.17: Read the publish side — who puts a frame task into a worker's ready registry?

## The next action — verbatim

S2.17 — Read the publish side: who puts a frame task into a worker's ready registry, and why none was there when main
kicked? S2.16 read the worker loop's body and it holds no completion: `main+0x756d70`..`main+0x756e7c` calls only the
two registry selects (`main+0x7598c0` from `x24 = [x23+0x10]` under mask `w1 = [x22+0x10]`; `main+0x759510` from
`x25 = [x23+0x50]`), the waiter-node allocator (`main+0x7584b0`), a release helper (`main+0x7584e0`), the lock
(`main+0x759e50`) and the timed-wait pair (`main+0x7f3b60`/`main+0x7f3b70`), and none resolves to the completion
`ReleaseSemaphore` (`sdk+0x33ba80`). The dequeued item goes to `main+0x756b10` and then to the dispatcher
`main+0x757060`, which tail-calls the item's own function pointer, so the completion is issued by the closure the loop
selected, never by the loop. The loop skips it exactly when both selects return null — register `x0` tested by `cbnz
x0` at `main+0x756d84` and `main+0x756d94`, gated by the mask `w1 = [x22+0x10]` at `main+0x756d74` (`cbz w1` → the
blocking pop `main+0x759a60` at `main+0x756e84`) — and the stop's own dump (S2.13 r17) shows all three workers parked
at that wait with a non-zero mask, i.e. an empty registry. Read the other side: what pushes a task onto
`[x23+0x10]`/`[x23+0x50]` and what writes `[x22+0x10]`, together with main's per-frame path between its three kicks
and its park on the completion key — the refcounted closure runner `main+0x756720`/`main+0x7568e0` reached from
`main+0x756798`, which is where `SignalEvent` (`sdk+0x338e50`) is issued. The stop is rare (1 of 20 bounded 65 s boots,
S2.13 r17 at 59.7 s / frame 1706) and S2.10's runs presented to frame 3600 and 5447 with zero stops: if the publish
read also names no defect, close the line by naming the rate and the runs rather than by inventing a fix (Do-Not #8).
Instrument before changing behaviour; do not weaken the S2.9 equal-priority rotation (Do-Not #9); do not add a counter
that fires on the healthy path — under work-stealing a worker routinely finds the registry empty for no fault (Do-Not
#13); do not touch `games/totk` or `games/by3`.

S2.17 closes when the site that publishes a frame task into a worker's ready registry — or the branch that skips the
publish — is named from the disassembly with the register state that selects it, and either that names a defect site
and a fix follows with bounded headless boots that present through the stop's window and past it with zero stops across
N runs, each with its bound and frame count; or the read fails with its own message and the command that produced it.
If the publish read also names no defect, the slice closes by naming the stop's rate and its runs (1 of 20 bounded 65 s
boots; S2.10's zero-stop runs to frame 3600 and 5447), not by a fix.

## What is settled — do not re-derive

- **The loop's body (S2.16).** `main+0x756d70`..`main+0x756e7c` issues **no** completion and holds no branch that
  decides one; its only `bl` targets are the eight listed in `read.md`. **The completion is the closure's**: the
  dequeued item reaches the dispatcher `main+0x757060`, which tail-calls the item's own function pointer (`ldr x4,
  [x19, #0x30]` at `main+0x7570a4`; null → `ret` at `main+0x7570fc`; `br x4` at `main+0x7570bc`; flag bit 3 → `br x5`
  at `main+0x7570f8` with `x0 = worker`). Full register/offset detail is in `.babysteps/scratch/S2.16-producer-read/`.
- **The two gates that skip it, and their registers.** `w1 = [x22+0x10]` at `main+0x756d70` is the ready-priority
  **mask** (used as an AND gate at `main+0x759940`); `cbz w1, 0x756e84` sends a zero mask to the blocking pop
  `main+0x759a60`. Otherwise `x0` is the select result (`main+0x7598c0`) and the second registry's pop result
  (`main+0x759510`), tested by `cbnz x0` at `main+0x756d84` and `main+0x756d94`. Both null → `main+0x756d98` re-pushes
  a waiter node onto both and re-parks at `main+0x756e60`. **A wake with an empty registry is a wake with no completion.**
- **The stop's dump agrees.** r17 parks `0x1002`..`0x1005` at `sdk+0xd884dc <- sdk+0x33906c <- main+0x756e64` — the
  select path, so the masks were non-zero and the registries empty. Main's own release, `signal key=0x16bc68b18
  by=0x1001 woke=0`, is immediately followed by `wait key=0x16bc68b18 by=0x1001`: not aimed at a worker, it is the
  fast-path re-acquire of its own post S2.15 accounted.
- **The credit account (S2.15).** `0x1001` is the only consumer of every frame-completion credit in 12 clean 65 s
  boots; `takes` equals `rel` on all four completion-shaped keys (48/48 rows); the three workers' `take` is 0 there in
  all 12 runs and `takes − acq` equals the consumer's own `rel` (r1 `…cc8` acq 7069 / takes 11920 / own rel 4851).
- **The loop's shape and the stop's rarity, unchanged.** The wait is `main+0x756e60` on `x26 = [x22+0x48]`, timeout
  `[x23+0x1528] × 0x0f4240` ns, and `main+0x756e64` discards its result. r17: `present stop: last presented frame
  1706 … (59.7 s since start)`, 1 of 20 bounded 65 s boots; S2.10's runs reached frame 3600 and 5447, zero stops.
- **Not the S2.9 shape.** All three cores `owner 0`; no `core waiter` line; every waiter `wkind=2`. **Gate.**
  `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit
  0, 23.521 s today. **Work tree** `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable, not evidence.

## Withdrawn — proven wrong, do not use

- **"The loop's body reaches the completion"** (this slice's own question). Refuted: the range contains no
  `ReleaseSemaphore` and no call that resolves to `sdk+0x33ba80`. **"The branch that skips the completion is a defect
  site"** is withdrawn with it: it is the null registry select, the correct branch for an empty registry.
- **"A worker must be taking those credits through `TryAcquireSemaphore`" (S2.13).** Refuted by observation: `take=0`
  for `0x1002`/`0x1003`/`0x1004` on every completion key in every run.
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Its conclusion holds, its
  argument does not (Do-Not #15). **`rewait`/`unattributed`:** both fire on the healthy loop (Do-Not #13).
- **Still withdrawn from S2.13:** "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the present stop
  is a lost wakeup/condition race"; "the workers stop being woken" / "the workers are parked".
- **One row S2.15 does not explain:** the one-shot semaphore `0x…3db8` reads `rel=2 takes=0`; `rel` counts the cv's
  `SignalProcessWideKey` SVCs, so a non-`ReleaseSemaphore` signal inflates it. Prior record numbers (8 shaders, frame
  441, p50 33.33 ms, 255 tests, 721 strings, r17's 59.7 s / frame 1706) are history, not re-derived; S2.14's discarded
  GPU-contended batch is **not** usable numbers. There is no numbered "Numbers" list in this state file.

## Code to touch

| Work | Touch |
| --- | --- |
| Read the publish site | what pushes a task onto `x24=[x23+0x10]` / `x25=[x23+0x50]`, and what writes the mask `[x22+0x10]`. `main+0x7584b0` allocates the 0x30-byte waiter node; `main+0x759450` / `main+0x7595e0` / `main+0x759e30` initialise 0x30-byte vtables at `0xd37000+0x190/0x198`. |
| Read main's per-frame path | the refcounted closure runner `main+0x756720`/`main+0x7568e0`, its caller `main+0x756798`, and where `SignalEvent` (`sdk+0x338e50`) is issued. `main.elf` is at `…/nfs-work/nfs/extracted/nfs100/elf/`. |
| Read the kick against the trace | `.babysteps/scratch/S2.13-missing-wakeup/r17.log` `[cond-trace]` — main's three kicks, its `woke=0` self-post, its wait, each worker's completion signal. |
| Re-read the ledger if needed | `games/nfs/runtime/threads.c` prints `takes=`/`take=`; `.babysteps/scratch/S2.15-take-trap/summary.py` reads the rows. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`; no behaviour change in an instrumenting commit; do not add a counter that fires on the healthy path. |

## Read these first, and only these

- `.babysteps/scratch/S2.16-producer-read/{read.md,main-756cb0.dis,main-756b10.dis,main-757060.dis,helpers.dis}` — the read S2.16 closed, with the command that produced it.
- `.babysteps/scratch/S2.13-missing-wakeup/{r17.log,main-756e64.dis,sdk-sem.dis}` — the stop's transition, the worker loop, the semaphore/critical-section disassembly.
- `docs/donot.md` #8–#16.

## Do-Not entries that apply here (9 of 16)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #9 — do not model a virtual core without rotating equal-priority threads.
- #10 — do not rest a probe's attribution on a separate control run.
- #11 — do not close a slice by appending its record while the STATE.md top sections still name the finished slice.
- #12 — do not size a stall's trace window from the dump.
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.
- #16 — do not read a zero-hit `search_text` as absence.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 today).
- Bounded headless boots (`probe.sh <n> 65` from `.babysteps/scratch/S2.17-*/`, the S2.12/S2.13 control schedule unchanged) **only if a fix follows**, on a machine with the GPU free — verify that before the batch, and discard the batch if another process held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.17 leaves behind, re-verify the Do-Not count (16 or higher), and commit the state and the seed.
