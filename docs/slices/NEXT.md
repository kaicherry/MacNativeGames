---
slice: S2.18
stateFile: docs/STATE.md
donotCount: 16
seededFrom: 9795558
---

# Next slice — S2.18: Name the site in main that issues the frame-completion `ReleaseSemaphore`, or the dispatcher branch that returns without it

## The next action — verbatim

S2.18 — Name the site in main that issues the frame-completion `ReleaseSemaphore`, or the dispatcher branch that returns without it. S2.17 read the publish side and named no defect: `main+0x758070` enqueues a task — into registry A at `core+0x10` when bit 1 of `[task+0x38]` is set (`main+0x758148` → `main+0x758350` → `main+0x7598a0` → `main+0x759780`), into registry B at `core+0x50` when it is clear (`main+0x758164` → `main+0x759480`), and not at all when `[core+0x14d1] != 0` (`main+0x758094` `cbz w8, 0x758144`, which builds a temporary worker on `core+0x210` and runs the task inline) — each push links the entry before it notifies the matching waiter list (`core+0x40`/`core+0x70`), which `main+0x756890` → `main+0x7568e0` → `main+0x756720` drains by calling each waiter node's closure `[node+0x10]` (`main+0x756798` `blr x8`), for a sleeping worker `main+0x756fc0` = `SignalEvent(worker+0x48)` (`main+0x7f3b80` → `sdk+0x338e50`). The dequeued item reaches the dispatcher `main+0x757060`, which tail-calls the item's own function pointer (`ldr x4, [x19, #0x30]` at `main+0x7570a4`; `br x4` at `main+0x7570bc`) and returns without calling it when that pointer is null (`cbz x4, 0x7570fc`). Read that closure: name `nn::os::ReleaseSemaphore` (`sdk+0x33ba80`) in main's import table, find its thunk and every `bl` to it, and read a frame worker's execute closure from `[item+0x30]` — where it issues the release on main's completion semaphore (`0x16bc68b18` in r17) — and test whether a dequeued frame item can reach `main+0x757060` with `[item+0x30] == 0` and drop the completion, leaving main parked exactly as r17's dump shows. Instrument before changing behaviour; do not weaken the S2.9 equal-priority rotation (Do-Not #9); do not add a counter that fires on the healthy path — under work-stealing a worker routinely finds the registry empty for no fault (Do-Not #13); do not touch `games/totk` or `games/by3`.

S2.18 closes when the site in main that issues `nn::os::ReleaseSemaphore` (`sdk+0x33ba80`) on a frame-completion semaphore — or the branch that reaches the dispatcher `main+0x757060` and returns without it — is named from the disassembly with the register state that selects it, and either that names a defect site and a fix follows with bounded headless boots that present through the stop's window and past it with zero stops across N runs, each with its bound and frame count; or the read fails with its own message and the command that produced it. If it also names no defect, the slice closes by naming the stop's rate and its runs (1 of 20 bounded 65 s boots, S2.13 r17 at 59.7 s / frame 1706; S2.10's zero-stop runs to frame 3600 and 5447), not by a fix.

## What is settled — do not re-derive

- **The publish entry (S2.17).** `main+0x758070` (`x0 = core`, `x1 = task`) is reached by tail-call from `main+0x757f90`, `main+0x758190`, `main+0x755700`/`0x7558e0`/`0x755f10`, and by `bl 0x758070` at `main+0x756c8c` (the task runner's continuation). It runs three branches: `main+0x758094` `cbz w8, 0x758144` on `w8 = [core+0x14d1]` — nonzero **skips the enqueue** and builds a temporary worker on `core+0x210` (`main+0x758098`..`main+0x758140`) to run the task inline; `main+0x758148` `tbnz w8, #0x1, 0x758168` on `w8 = [task+0x38]` — bit 1 clear → `b 0x759480` with `x0 = core+0x50` (registry B), bit 1 set → `b 0x758350` with `x0 = core+0x10` (registry A).
- **Which registry is which.** The pool ctor `main+0x757110` calls `bl 0x7595e0` on `core+0x10` (**registry A**, vtable ptr `[0xd37198] = 0xd198e0`) and `bl 0x759450` on `core+0x50` (**registry B**, vtable ptr `[0xd37190] = 0xd198d8`). A: select `main+0x7598c0`, `[reg+0x8]` non-empty flag / `[reg+0x10]` 16-byte-entry array / `[reg+0x18]`,`[reg+0x1c]` bounds / `[reg+0x24]` seq, entry mask byte at `entry+0x8` ANDed with the worker mask at `main+0x759940`; insert `main+0x7598a0` → `main+0x759780`; waiter list `core+0x40`. B: pop `main+0x759510` (head `[reg+0x10]`, next `task+0x48`); push `main+0x759480`; waiter list `core+0x70`.
- **The wake chain.** Each push notifies **after** linking the entry, only with a registered waiter: A notifies `core+0x40` (`bl 0x756890` at `main+0x759828`; re-check `main+0x759888`), B notifies `core+0x70` (`b 0x756890` at `main+0x7594fc`; re-check `main+0x75943c`). `main+0x756890` pops the list and calls `main+0x7568e0` → `main+0x756720`, which calls the node's closure `[node+0x10]` with `x0 = [node+0x18]` (`ldp x8, x0, [x19, #0x10]`; `blr x8` at `main+0x756798`). A sleeping worker's node closure is `main+0x756fc0` (installed at `main+0x756e14` from `[0xd37000+0x180]`) = `add x0, x0, #0x48; b 0x7f3b80` = `SignalEvent(worker+0x48)` (`main+0x7f3b80` → GOT `0xd21560` → `sdk+0x338e50`).
- **The mask is `[worker+0x10]`.** Written by `main+0x756a40` (`str w1, [x0, #0x10]`) from `main+0x756930` (set mask, run via `main+0x756a60`, restore; reached from `main+0x756388` in the per-frame path `main+0x756360`) and `main+0x7593b0`; zeroed by the worker ctor `main+0x7569f0`; read by the loop at `main+0x756d70`.
- **The publish names no defect.** The entry is linked before the waiters are notified, and each worker's add-waiter re-checks the registry's non-empty flag after linking (`main+0x759590` reads `[regA+0x8]`; `main+0x759400` reads `[regB+0x10]`). A lost wake needs pusher-check < worker-link **and** worker-re-check < pusher-link — impossible. An empty-*selectable*-registry kick is the mask-gated wake (the drain wakes every registered waiter regardless of mask), the healthy work-stealing case (Do-Not #13). Full detail: `.babysteps/scratch/S2.17-publish-read/read.md`.
- **The dispatcher (S2.16).** The dequeued item reaches `main+0x757060`, which bump-refcounts `[item+0x40]`, tests `[item+0x38]` flags and `[item+0x18]` vs `[item+0x8]`, loads `[item+0x30]` (`main+0x7570a4`) — **null → plain `ret` at `main+0x7570fc`, no closure called** — takes args `x0..x3` from `[item+0x50]`,`[+0x58]`,`[+0x60]`,`[+0x68]`, and tail-calls (`br x4` at `main+0x7570bc`; flag bit 3 → `br x5` at `main+0x7570f8` with `x0 = worker`). The runner `main+0x756b10` consumes the item's one-shot callback `[item+0x28]` at `main+0x756b34`.
- **The stop's own record (S2.13 r17, uninstrumented).** `[cond-trace]` prints "last 4096 condition events" three times; the **last block** (from line 9253) spans ~3.9 s and its **last signal is at `-2579.7 ms`** (`signal key=0x78f284e8a8 by=0x1001 woke=0x1002`), immediately followed by main's `wait key=0x16bc68b18 by=0x1001`. The remaining 2.55 s hold **zero** signals; every worker line is only `timeout … woke=0` (~12–13 ms) then `wait … woke=<self>`. Main's completion semaphore is `0x16bc68b18` (`count=0`, `max=0x7fffffff`).
- **The rate.** 3 stops in 34 bounded post-fix watchdog boots (S2.11 run C, S2.12 r4, S2.13 r17); S2.13 alone 1 of 20. S2.10's runs reached frame 3600 and 5447 with zero stops. Report the range and the runs (Do-Not #8).
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0, 23.636 s today.

## Withdrawn — proven wrong, do not use

- **"The publish side drops a wake" / "the empty registry at the stop is a lost publish"** (S2.17). Refuted: insert-before-notify plus the worker's re-check close the race, and r17's record shows main parked at `-2579.7 ms` with zero signals in the final 2.55 s — the registries were empty because the publisher had parked.
- **"The loop's body reaches the completion"** and **"the branch that skips the completion is a defect site"** (S2.16). Refuted/withdrawn: the loop contains no completion call, and the null select is the correct branch for an empty registry.
- **"A worker must be taking those credits through `TryAcquireSemaphore`" (S2.13).** Refuted by observation (S2.15: `take=0` for `0x1002`/`0x1003`/`0x1004` on every completion key in every run).
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Conclusion holds, argument does not (Do-Not #15). **`rewait`/`unattributed`:** both fire on the healthy loop (Do-Not #13).
- **Still withdrawn from earlier:** "the present stop is a lost wakeup/condition race"; "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the workers stop being woken" / "the workers are parked".
- **One row unexplained:** the one-shot semaphore `0x…3db8` reads `rel=2 takes=0`; `rel` counts the cv's `SignalProcessWideKey` SVCs, so a non-`ReleaseSemaphore` signal inflates it. Prior record numbers (8 shaders, frame 441, p50 33.33 ms, 255 tests, 721 strings) are history, not re-derived; S2.14's discarded GPU-contended batch is **not** usable numbers.

## Code to touch

| Work | Touch |
| --- | --- |
| Find the release site | the sdk `ReleaseSemaphore` import `_ZN2nn2os16ReleaseSemaphoreEPNS0_13SemaphoreTypeEi` (`sdk+0x33ba80`) in main's own import table and its thunk. `objdump -R main.elf \| grep -i ReleaseSemaphore`, then `grep -nE 'bl\s+0x<thunk>' <main-full.dis>`. The S2.16/S2.17 pattern for a thunk: `adrp x16, 0xd21000; ldr x17, [x16, #0xNNN]; add; br x17`. |
| Read the execute closure | the item's function pointer `[item+0x30]`, tail-called at `main+0x7570bc`; the null early-return at `main+0x7570fc`; the runner `main+0x756b10` and the continuation publish at `main+0x756c8c`. `main.elf` is at `…/nfs-work/nfs/extracted/nfs100/elf/`. |
| Read the dispatcher's guards | `main+0x757060`..`main+0x7570fc`: `[item+0x40]` refcount, `[item+0x38]` flags, `[item+0x18]` vs `[item+0x8]`, args `[item+0x50]`..`[item+0x68]`. |
| Read the kick against the trace | `.babysteps/scratch/S2.13-missing-wakeup/r17.log` `[cond-trace]` **last block** (from line 9253) — main's kick at `-2579.7 ms` then `wait key=0x16bc68b18`; and each worker's `timeout … woke=0` tail. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`; no behaviour change in an instrumenting commit; do not add a counter that fires on the healthy path. |

## Read these first, and only these

- `.babysteps/scratch/S2.17-publish-read/{read.md,publish.dis,queue-ctor.dis,registry-lib.dis,main-756600.dis}` — the read S2.17 closed, with the commands that produced it.
- `.babysteps/scratch/S2.16-producer-read/{read.md,main-757060.dis,main-756b10.dis,main-756cb0.dis,helpers.dis}` — the dispatcher and the loop body.
- `.babysteps/scratch/S2.13-missing-wakeup/{r17.log,main-756e64.dis,sdk-sem.dis}` — the stop's window, the worker loop, the semaphore disassembly.
- `docs/donot.md` #8–#16.

## Do-Not entries that apply here (7 of 16)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #9 — do not model a virtual core without rotating equal-priority threads.
- #12 — do not size a stall's window from the dump (this seed's tail is the *last* of r17's three `[cond-trace]` blocks, from line 9253 — not the first two).
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.
- #16 — do not read a zero-hit `search_text` as absence.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 today).
- Bounded headless boots (`probe.sh <n> 65` from `.babysteps/scratch/S2.18-*/`, the S2.12/S2.13 control schedule unchanged) **only if a fix follows**, on a machine with the GPU free — verify that before the batch, and discard the batch if another process held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.18 leaves behind, re-verify the Do-Not count (16 or higher), and commit the state and the seed.
