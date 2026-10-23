---
slice: S2.20
stateFile: docs/STATE.md
donotCount: 17
seededFrom: b12c9e6c
---

# Next slice — S2.20: Verify shader and visual correctness (roadmap S2.4)

## The next action — verbatim

S2.20 — Verify shader and visual correctness (roadmap S2.4), the next approved work unit now that the present-stop descriptor line is closed. S2.19 read the frame-task submitter's descriptor and named no defect: the descriptor is inline at `entry+8` in the `0x6b0`-stride submit slot, `main+0x754e10` is its only execute-closure writer and fills `desc+0x20` from a relocated function pointer on all 20 submitter paths (`[0xd28e68]` = `main+0x45b220` for `main+0x3e9110`; `[0xd29558]` = `main+0x45b6e0` for the `main+0x43d970` constructor), so no frame task can be submitted with a zero execute closure and the dispatcher's null branch `main+0x7570a8` is unreachable from any submitter; the completion field `desc+0x00` is zero by default and is never read as a call. The present stop is recorded as a known intermittent limitation, not a defect site: 1 of 20 bounded 65 s boots (S2.13 r17 at 59.7 s / frame 1706; 3 stops in 34 across the post-fix watchdog runs), with S2.10's zero-stop presenting runs to frame 3600 and 5447 — do not re-open the descriptor, release, publish or credit lines without a new observation (Do-Not #8, #13, #15, #17). Verify the shader and visual output and record its exact boundary: which of the 8 translated shaders are translated, bound and used by a drawn frame; what each captured frame shows, with its frame number (the EA logo, the `PRESS ⊕` title screen at frame 1718, the save-data notice after a ⊕ press, and the motion-blurred racing frames); and the precise limit of the visual-correctness claim. Do not claim gameplay or a final picture. Do not touch `games/totk` or `games/by3`.

S2.20 closes when each of the 8 translated shaders is accounted for against a drawn frame (translated, bound, and reached by a submission that presented) and each captured frame's content is recorded with its frame number and what it shows, so the visual-correctness claim is bounded to named captures and named shaders; or the read fails with its own message and the command that produced it. If the verification names no defect, the slice closes by recording the boundary, not by a fix.

## What is settled — do not re-derive

- **The present-stop line is closed with no defect named (S2.16–S2.19).** The producer's loop body issues no completion (S2.16); the publish is correct by construction (S2.17); the release is main's generic `sem_post` in the shared-state layer, with no frame-specific site (S2.18); the submitter's execute closure is a relocated function pointer on all 20 paths, so a zero closure is unreachable (S2.19).
- **The descriptor is inline at `entry+8` in the `0x6b0`-stride submit slot (S2.19).** `main+0x756140` walks entries at stride `0x6b0` calling `main+0x7554f0(entry, core)`; `main+0x7560f0` does one entry. `main+0x7554f0` passes builder `main+0x758190` `x1 = entry+8` (the descriptor) and the four words at `entry+0x30` as the task args. `main+0x756fd0` reads only `desc+0x10` (mask), `+0x14` (prio), `+0x18`/`+0x19` (flags), `+0x20` (execute closure → `task+0x30`).
- **`main+0x754e10` is the sole execute-closure writer (S2.19).** `cbz w1, +8; ret; str x2,[x0,#0x20]; ret`. `main+0x755210` zeroes `desc+0x20` first (`str xzr,[x0,#0x28]`) and writes `desc+0x10` = `0x80`, `desc+0x14` = `0x3f`.
- **The two `nnMain` submitters (S2.19).** `main+0x3e9110` (called at `main+0x44289c`, `entry = obj+0x10`, `desc = obj+0x18`, closure `[0xd28e68]` = `main+0x45b220`); the `main+0x43d970` constructor (entry `obj+0x480`, desc `obj+0x488`, closure `[0xd29558]` = `main+0x45b6e0`), submitted by `main+0x43db10` (`bl 0x7560f0` at `main+0x43db44`).
- **The closure `main+0x45b220` is the game's singleton lock/wait (S2.19).** It substitutes `[0xd296f0]`, then `pthread_self` (`main+0x743190`) and S2.18's `sem_wait(this+0x18)` wrapper `main+0x7433f0`. Not a release site.
- **The stop's rate and runs (S2.13/S2.17/S2.18).** 1 of 20 bounded 65 s boots (r17, 59.7 s, frame 1706); 3 stops in 34 across the post-fix watchdog runs (S2.11 run C, S2.12 r4, S2.13 r17); S2.10 presented to frame 3600 and 5447 with zero stops.
- **What is already captured.** S2.6 frame 441 non-black; S2.10 captures at frames 809/1406/2307/3206 and 1718/3518/5019 (EA logo, `PRESS ⊕` title, racing footage); S2.11's ⊕ press moved the title screen to a save-data notice. Visual correctness is claimed only for those captures; no gameplay is established.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0, 22.971 s on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"A frame task can be submitted with a zero closure" (S2.19).** Refuted: the execute closure is a relocated function pointer on all 20 submitter paths, and the completion field that is zero (`desc+0x00`) is never read as a call.
- **"A dequeued frame item with `[item+0x30] == 0` drops the completion" (S2.18).** Refuted: the runner's continuation runs regardless, and the branch is submitter-selected.
- **"main issues `nn::os::ReleaseSemaphore`" (S2.18); "the publish drops a wake" (S2.17); "the loop body reaches the completion" (S2.16); "a worker takes the credit via `TryAcquireSemaphore`" (S2.13/S2.15).** All refuted; see `docs/STATE.md`.
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Conclusion holds, argument does not (Do-Not #15).
- **Earlier withdrawn:** "the present stop is a lost wakeup/condition race"; "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the workers stop being woken".
- **History, not re-derived:** 8 shaders, frame 441, p50 33.33 ms, 255 tests, 721 strings, the one-shot semaphore `0x…3db8` row, S2.14's discarded GPU-contended batch.

## Code to touch

| Work | Touch |
| --- | --- |
| Account for the 8 shaders | `docs/shader-list.md` (the shared identifier list and its contribution process), `docs/first-launch-shaders.md` (seed preparation and first-launch checks), the translator under `games/nfs/prebuilt/` and its `make shader-tools` target, and the boot's shader-translation log (`.babysteps/scratch/S2.4c-NFS-shader-translator/`, `.babysteps/scratch/S2.6-NFS-first-frame/`). |
| Read the captures | `.babysteps/scratch/S2.10-capture-past-677/` (`r*-end.png`, `r*.log`) and `.babysteps/scratch/S2.11-press-plus/` (`r*-end.png`, `r*.log`, `r*.emit`). Record frame number and content per capture; do not re-derive the frames already recorded. |
| The shader tests | `tests/test_shader_list.py`, `test_shader_seed.py`, `test_shader_scope.py`, `test_warm_metal.py`, `test_variant_libs.py` — they already pin the list and the seed, so check them before writing a new claim. |
| Do not | Do not re-open the descriptor/release/publish/credit lines without a new observation; do not touch `games/totk` or `games/by3`; do not claim gameplay or a final picture; do not rest a claim on a control run that died before its window (Do-Not #10). |

## Read these first, and only these

- `docs/shader-list.md` and `docs/first-launch-shaders.md` — the shader identifiers and the seed/first-launch contract.
- `.babysteps/scratch/S2.19-submitter-read/read.md` — what S2.19 closed, so the stop is not re-opened.
- `.babysteps/scratch/S2.10-capture-past-677/` and `.babysteps/scratch/S2.11-press-plus/` — the captures and their logs.
- `docs/STATE.md` § "S2.6", "S2.10", "S2.11" — the recorded frame numbers and what each capture shows.
- `docs/donot.md` #8–#17.

## Do-Not entries that apply here (5 of 17)

- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #10 — do not rest a probe's attribution on a separate control run that dies before its window; a single run can carry its own before-capture.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice; advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #16 — do not read a zero-hit `search_text` as absence.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- Bounded headless boots (`probe.sh <n> 65`, the S2.12/S2.13 control schedule unchanged) **only if a fix follows**, on a machine with the GPU free — verify that before the batch and discard the batch if another process held it.
- Every claim tied to a recorded observation; no gameplay claim; no visual-correctness claim beyond the named captures.

## When this slice closes

Write the next seed from what S2.20 leaves behind, re-verify the Do-Not count (17 or higher), and commit the state and the seed.
