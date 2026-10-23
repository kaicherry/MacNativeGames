---
slice: S2.19
stateFile: docs/STATE.md
donotCount: 17
seededFrom: 01d156a5
---

# Next slice — S2.19: Name the frame-task submitter, and read the descriptor it passes, or close the line

## The next action — verbatim

S2.19 — Name the frame-task submitter, and read the descriptor it passes, or close the line. S2.18 read the release side and named no defect: main's import table holds no `nn::os::ReleaseSemaphore` — nor any `nn::os` semaphore (`objdump -R main.elf | grep -i semaphore` prints nothing; the 42 `_ZN2nn2os*` imports are the event/timer/thread/mutex/condition-variable set) — so the release a frame worker issues is musl's `sem_post` (GOT `0xd21448` → stub `main+0x7f3950` → `sdk+0x51bc10` → `nn::os::ReleaseSemaphore` `sdk+0x33ba80`), whose six call sites in main (`main+0x743dc8`, `0x7440f0`, `0x744548`, `0x744ef8`, `0x745220`, `0x7453d4`) all sit in one shared-state layer (functions `main+0x743bc0`, `main+0x744320`, `main+0x7451b0`, `main+0x745340`; refcount at `+0x20`, semaphores at `this+0x18`/`+0x70`/`+0x98`; `main+0x7433e0` = `sem_post(this+0x18)`, `main+0x7433f0` = `sem_wait(this+0x18)` with an EINTR retry), so there is no site unique to the frame path to name. The dispatcher's null branch is `main+0x7570a8` `cbz x4, 0x7570fc` on `x4 = [item+0x30]` (`ldr` at `main+0x7570a4`), and `[item+0x30]` is the submitter's descriptor field `[desc+0x20]` written by `main+0x756fd0` at `main+0x757008`/`main+0x757018` (`[desc+0x19]` picks the `w8 |= 0x8` branch); the branch cannot drop the completion, because the runner `main+0x756b10` lifts the item's completion node from `[task+0x28]` (`main+0x756b34`) before dispatch and its continuation `main+0x756ba4`..`main+0x756bac` runs after the dispatcher returns either way. Read the submitter: who builds the descriptor handed to the builders `main+0x757f90`/`main+0x758190` (and to `main+0x755700`/`0x7558e0`/`0x755f10`), what its `+0x20` (the execute closure that becomes `[item+0x30]`) and its `+0x00` (the completion-node closure set at `main+0x758024`/`main+0x758224` and lifted from `[task+0x28]`) hold, and whether a frame task can be submitted with either zero; if the closure is a distinct function, disassemble it for the release, and if no distinct site exists, close the line. Instrument before changing behaviour; do not weaken the S2.9 equal-priority rotation (Do-Not #9); do not read an unwind frame that names an SDK function as main's own call — check main's relocation table for the symbol it imports (Do-Not #17); do not add a counter that fires on the healthy path — under work-stealing a worker routinely finds the registry empty for no fault (Do-Not #13); do not touch `games/totk` or `games/by3`.

S2.19 closes when the site that builds the frame task's descriptor — the function whose `+0x20` becomes `[item+0x30]` and whose `+0x00` becomes the completion node's closure — is named from the disassembly with the register state that fills it, and either a submitter that can pass a zero closure names a defect site and a fix follows with bounded headless boots that present through the stop's window and past it with zero stops across N runs, each with its bound and frame count; or the read fails with its own message and the command that produced it. If it also names no defect, the slice closes by naming the stop's rate and its runs (1 of 20 bounded 65 s boots, S2.13 r17 at 59.7 s / frame 1706; S2.10's zero-stop runs to frame 3600 and 5447), not by a fix.

## What is settled — do not re-derive

- **main imports no `nn::os` semaphore (S2.18).** `objdump -R main.elf | grep -i semaphore` prints nothing; the 42 `_ZN2nn2os*` JUMP_SLOT imports are the event/timer/thread/mutex/condition-variable/memory set; `main-inventory.json` `imports` (461) agree. main imports musl `sem_init`/`sem_destroy`/`sem_post`/`sem_wait`/`sem_trywait` (`0xd21438`/`0xd21440`/`0xd21448`/`0xd21450`/`0xd21498`; stubs `main+0x7f3930`/`0x7f3940`/`0x7f3950`/`0x7f3960`/`0x7f39f0`), whose SDK bodies (`sdk+0x51bc08`/`0x51bbf8`/`0x51bc10`/`0x51bc28`/`0x51bc20`) reach `nn::os::{Initialize,Release,Acquire}Semaphore`. So any release the worker issues is a `sem_post`, not a direct `nn::os` call.
- **The six `sem_post` sites are all in main's shared-state layer (S2.18).** `main+0x743dc8`, `0x7440f0` (in `main+0x743bc0`, the function main parks in: `sem_wait` at `main+0x743c0c`, `sem_trywait` at `0x743c54`/`0x743c68`/`0x743d0c`/`0x743d2c`); `main+0x744548`, `0x744ef8` (in `main+0x744320`); `main+0x745220` (in `main+0x7451b0`); `main+0x7453d4` (in `main+0x745340`, posting `state+0x98`). Wrappers: `main+0x7433e0` = `add x0,x0,#0x18; b sem_post`, `main+0x7433f0` = `sem_wait(this+0x18)` (EINTR retry). Refcount at `+0x20`; semaphores at `this+0x18`, `+0x70`, `+0x98`.
- **S2.13's "main's stack is `nn::os::AcquireSemaphore` through `main+0x743c10`" is a misread of the import (S2.18).** `main+0x743c0c` is `bl 0x7f3960` = `sem_wait`; the SDK body `sdk+0x33b8e0` was one link below. Do not re-open this as "main calls `nn::os::AcquireSemaphore`" (Do-Not #17).
- **The dispatcher's null branch (S2.16/S2.18).** `main+0x757060` (`x0 = item`, `x1 = worker`): `main+0x757078`..`0x757084` bump `[item+0x40]`; `main+0x757088`..`0x757094` compare `[item+0x18]` vs `[item+0x8]`; `main+0x757098` `tbnz w8,#0x4` and `main+0x7570a0` `tbnz w8,#0x3` on `w8 = [item+0x38]`; `main+0x7570a8` `cbz x4, 0x7570fc` on `x4 = [item+0x30]` (`ldr` `main+0x7570a4`); `br x4` at `main+0x7570bc` with `x0..x3 = [item+0x50]`,`+0x58`,`+0x60`,`+0x68`; flag-3 `br x5` at `main+0x7570f8` with `x0 = worker`. `main+0x7570fc` is a real epilogue, so the null path returns to `main+0x756ba4`.
- **`[item+0x30] = [desc+0x20]`, submitter-supplied (S2.18).** `main+0x756fd0` (`x0 = task`, `x1 = core`, `x2 = desc`) writes it at `main+0x757008` (`[desc+0x19] != 0`, also sets flag bit 3) and `main+0x757018` (`[desc+0x19] == 0`); it also sets `[task+0x38]` from `[desc+0x10]`/`[desc+0x14]`/`[desc+0x18]` and `[task+0x50..0x68]` from `x3..x6`. The builders `main+0x757f90`/`main+0x758190` call it at `main+0x757ffc`/`main+0x7581fc` and enqueue with `b 0x758070`.
- **The completion node is at `task+0x28` (S2.18).** The runner `main+0x756b10` (`x0 = worker`, `x1 = task`) lifts it at `main+0x756b34`, clears it, prepares it (`main+0x756b78`..`main+0x756b94`), calls the dispatcher at `main+0x756ba0`, then runs `main+0x756ba4`..`main+0x756bac` (`str x0, [x23, #0x10]`) unconditionally; the no-node paths reach the same continuation (`main+0x756c58`/`main+0x756c60`). The node's own closure is `[desc+0x00]` (builders `main+0x758024`/`main+0x758224`). **A null `[item+0x30]` therefore does not drop the completion.**
- **The publish is correct by construction (S2.17).** `main+0x758070` enqueues into registry A (`core+0x10`, bit 1 of `[task+0x38]` set, inset `main+0x7598a0`→`0x759780`) or B (`core+0x50`, bit clear, push `main+0x759480`), skips when `[core+0x14d1] != 0`; insert-before-notify plus the waiter's re-check closes the race; the drain `main+0x756890`→`0x7568e0`→`0x756720` runs `[node+0x10]` (sleeping worker: `main+0x756fc0` = `SignalEvent(worker+0x48)`, `main+0x7f3b80`→`sdk+0x338e50`).
- **The loop body holds no completion (S2.16).** `main+0x756d70`..`main+0x756e7c`; skip is `x0` from `main+0x7598c0`/`main+0x759510` gated by `w1 = [x22+0x10]`.
- **The stop's record (S2.13 r17, uninstrumented).** `[cond-trace]` last block from line 9253 (~3.9 s): last signal `-2579.7 ms` (`signal key=0x78f284e8a8 by=0x1001 woke=0x1002`), then `wait key=0x16bc68b18 by=0x1001`; the final 2.55 s hold zero signals, only `timeout … woke=0` (~12–13 ms) per worker. Main's completion key has `count=0`, `max=0x7fffffff`.
- **The rate.** 3 stops in 34 bounded post-fix watchdog boots (S2.11 run C, S2.12 r4, S2.13 r17); S2.13 alone 1 of 20. S2.10's runs reached frame 3600 and 5447 with zero stops. Report the range and the runs (Do-Not #8).
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0, 22.374 s on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"main issues `nn::os::ReleaseSemaphore` on the frame-completion semaphore" (S2.17→S2.18).** Refuted: main's import table holds no `nn::os` semaphore; the release is musl `sem_post`.
- **"A dequeued frame item reaching `main+0x757060` with `[item+0x30] == 0` drops the completion" (S2.18).** Refuted: `[item+0x30]` is the submitter's `[desc+0x20]`, and the runner's continuation `main+0x756ba4`..`0x756bac` runs after the dispatcher returns regardless.
- **"The publish side drops a wake" / "the empty registry at the stop is a lost publish" (S2.17).** Refuted: insert-before-notify plus the waiter's re-check; r17 shows main parked at `-2579.7 ms` with zero signals in the final 2.55 s.
- **"The loop's body reaches the completion" (S2.16); "a worker must be taking those credits through `TryAcquireSemaphore`" (S2.13).** Refuted by S2.16 and S2.15 (`take=0` for `0x1002`/`0x1003`/`0x1004` on every completion key).
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Conclusion holds, argument does not (Do-Not #15). `rewait`/`unattributed` fire on the healthy loop (Do-Not #13).
- **Still withdrawn from earlier:** "the present stop is a lost wakeup/condition race"; "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the workers stop being woken" / "the workers are parked".
- **History, not re-derived:** 8 shaders, frame 441, p50 33.33 ms, 255 tests, 721 strings, the one-shot semaphore `0x…3db8` `rel=2 takes=0` row, and S2.14's discarded GPU-contended batch (not usable numbers).

## Code to touch

| Work | Touch |
| --- | --- |
| Name the submitter | the callers that build the descriptor `x1`/`x2` for the builders `main+0x757f90`, `main+0x758190`, `main+0x755700`/`0x7558e0`/`0x755f10`. The three thin thunks `main+0x7560b0` (`ldr x0,[x0]; b 0x758190`), `main+0x7560c0` (`ldp x9,x3,[x2]; ldr x0,[x0]; ldp x4,x5,[x2,#0x10]; mov x2,x9; b 0x758190`) and `main+0x7560e0` (`ldr x0,[x0]; b 0x757f90`); the batch submit `main+0x756140` (loops 0x6b0-stride entries calling `main+0x7554f0`/`0x755580`/`0x755700`); `main+0x7560f0`. Callers in `nnMain` `main+0x2d2fd0`: `main+0x43db44` (`bl 0x7560f0`), `main+0x3e91cc` (`b 0x756140`). |
| Read the descriptor | `[desc+0x00]` (completion-node closure, stored at `main+0x758024`/`0x758224`), `[desc+0x08]`, `[desc+0x10]` (mask), `[desc+0x14]` (prio), `[desc+0x18]`/`[desc+0x19]` (flags), `[desc+0x20]` (execute closure → `[item+0x30]`). `main+0x7554f0` copies `[holder+0x8..]` into `x2`'s source (`main+0x75551c`..`0x755530`). |
| Read the closure against the release | disassemble the closure the descriptor's `+0x20` holds; if it is a distinct function, look for a `bl` to the `sem_post` stub (`main+0x7f3950`) or a call into `main+0x743bc0`/`0x744320`. If it is a generic future post, there is no distinct release site to name. |
| Read the kick against the trace | `.babysteps/scratch/S2.13-missing-wakeup/r17.log` `[cond-trace]` **last block** (from line 9253) — main's kick at `-2579.7 ms` then `wait key=0x16bc68b18`; and each worker's `timeout … woke=0` tail. |
| Do not | Do not weaken the S2.9 equal-priority rotation; do not change `core_slice_ns` or `runtime_core_checkpoint` unless the named cause is one of them; do not touch `games/totk` or `games/by3`; no behaviour change in an instrumenting commit; do not add a counter that fires on the healthy path; do not read an unwind frame naming an SDK function as main's own call. |

## Read these first, and only these

- `.babysteps/scratch/S2.18-release-site-read/{read.md,relocs.txt,pltmap.py,callsites.py,main-757060-dispatcher.dis,main-756fd0-taskfill.dis,main-7554b0-submit.dis}` — the read S2.18 closed, with the commands that produced it.
- `.babysteps/scratch/S2.17-publish-read/{read.md,main-full.dis,publish.dis,registry-lib.dis,queue-ctor.dis}` — the publish side and the full disassembly to reuse.
- `.babysteps/scratch/S2.16-producer-read/{read.md,main-756b10.dis,main-757060.dis,helpers.dis}` — the dispatcher and the runner.
- `.babysteps/scratch/S2.13-missing-wakeup/{r17.log,main-756e64.dis,sdk-sem.dis}` — the stop's window and the semaphore disassembly.
- `docs/donot.md` #8–#17.

## Do-Not entries that apply here (8 of 17)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #9 — do not model a virtual core without rotating equal-priority threads.
- #12 — do not size a stall's window from the dump (this seed's tail is the *last* of r17's three `[cond-trace]` blocks, from line 9253).
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.
- #16 — do not read a zero-hit `search_text` as absence.
- #17 — do not name main's own call by the SDK function an unwind frame reaches; check main's relocation table for the symbol it imports.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- Bounded headless boots (`probe.sh <n> 65` from `.babysteps/scratch/S2.19-*/`, the S2.12/S2.13 control schedule unchanged) **only if a fix follows**, on a machine with the GPU free — verify that before the batch, and discard the batch if another process held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim.

## When this slice closes

Write the next seed from what S2.19 leaves behind, re-verify the Do-Not count (17 or higher), and commit the state and the seed.
