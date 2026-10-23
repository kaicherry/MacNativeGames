---
slice: S2.11
stateFile: docs/STATE.md
donotCount: 9
seededFrom: 701fd15
---

# Next slice — S2.11: press ⊕ at the title screen and record what changes

## The next action — verbatim

S2.11 — With presenting clean past the old stop, press the title screen's ⊕ button and record what the boot does next. S2.10 ran two bounded post-fix headless boots (`TOTK_PRESENT_WATCHDOG=2500`, `hletrace 2`) with live-appended `shot` captures: run A (130 s) reached frame 3600 (captures at frames 809/1406/2307/3206) and run B (185 s) reached frame 5447 (captures at 1718/3518/5019), neither printed a present stop, and the captures show the EA logo, the NFS Hot Pursuit Remastered title screen (a `PRESS ⊕` prompt at frame 1718) and motion-blurred racing footage frames. The guest has not been observed to react to input. Press ⊕ (`TOTK_INPUT_FILE` command `key 36 <ms>` or `down 36`/`up 36`; the frontend maps `Return` = Mac key code 36 to Plus, mechanism in `games/nfs/runtime/input.c`) in a bounded headless boot with `shot` captures before and after the press, and record whether the picture changes (a menu, a different screen) and what the next stop is, if any; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

S2.11 closes when a bounded run presses ⊕ and what the captures show before and after is recorded, and either the boot leaves the title/attract state (recorded by a capture) or the picture does not change and that is recorded, with the next stop recorded with its own message if one appears; or the first probe fails with its own message and the command that produced it.

## What is settled — do not re-derive

- **The stop is fixed and presenting is clean.** S2.9's equal-priority core rotation (`core_slice_ns` = 4 ms, `yield_eq`; Do-Not #9) closed the 354–677 stop. Zero present stops in: boot3 (90 s, frame 2400), boot4 (30 s, frame 600), S2.10 run A (130 s, frame 3600), run B (185 s, frame 5447). The stop range is history, not a current symptom.
- **What the captures show.** EA logo → NFS Hot Pursuit Remastered logo/title (`PRESS ⊕` visible at frame 1718 in run B; not at frame 1406 in run A, so the prompt's timing varies) → motion-blurred racing footage frames (red car + police SUV in forest; desert with police helicopter and orange car; cockpit view with a police car in the mirror). No gameplay is established; visual correctness is claimed only for the captured frames.
- **Capture mechanism.** `shot <path>` in `TOTK_INPUT_FILE` writes the next presented frame (~2 frames after the line is read) as PNG; lines are appended live while the boot runs (the input thread polls the file). The log line `[dm] [nvn-mtl] [shot] F<n> <path>` records the frame. `shotn <n> <path with %d>` takes n consecutive frames.
- **Input injection.** `TOTK_INPUT_FILE` `down <code>` / `up <code>` / `key <code> <ms>` call `runtime_input_key_synthetic`, which feeds the full-key controller the guest reads through HID. Frontend key codes: `J` = 38 is A, `Return` = 36 is Plus (⊕). `TOTK_INPUT_SCRIPT="sec:keycode:ms,..."` presses at wall times; `TOTK_INPUT_LOG=1` logs which pad the game reads.
- **Build and run.** Build from `games/nfs` with `WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs`; run from the work tree `./build/nfs-native rewritten/nfs100 boot` with `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1`, `TMPDIR` on `/Volumes/X9BS/externalTmp/MacNativeGames`, bounded with `timeout -k 5 N`. The work tree is re-derivable, not evidence.
- **Gate and code.** S2.10 changed no code; `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0. S2.9 commits `0c4e0b9`/`d8a90e4`; close commits `701fd15` (S2.9), and the S2.10 close commit carries this seed.

## Withdrawn — proven wrong, do not use

- "The present stop is a lost wakeup/condition race." Withdrawn: the wakeup arrived and the woken thread could not get its core (Do-Not #9).
- "The condition key word is 0 in every run." Withdrawn: a waiter is registered; the `0` belonged to the spinning signaller's key and to `0x1002`'s unwaited event.
- "The main thread `0x1001` is the spinning thread." Withdrawn; `0x1002` is the created guest thread.
- "Presenting stops at frame 600 / 354 / 677" as a pinned value. Withdrawn: report the range and the runs (Do-Not #8).
- Numbers **2, 3, 4, 5** — unchanged from earlier seeds; do not re-measure. Prior record numbers (8 shaders, frame 441, p50 33.33 ms; GPU p50 0.86–1.45 ms; 255 tests; 721 strings) are history, not re-derived.

## Code to touch

| Work | Touch |
| --- | --- |
| Press ⊕ | No code needed: `TOTK_INPUT_FILE` `key 36 <ms>` / `down 36` / `up 36` (mechanism in `games/nfs/runtime/input.c`); captures via `shot <path>`; PNGs into the slice's scratch dir. |
| Next stop | Only if a new stop appears: the file its message names; `games/nfs/runtime/threads.c` only if the stop is a synchronization stop, and name its waiter/signaller pair from a dump before changing anything. |
| Do not | Do not change `core_slice_ns` or `runtime_core_checkpoint`; do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S2.10 — Capture what the boot presents past the old stop".
- `docs/donot.md` #8, #9.
- `.babysteps/scratch/S2.10-capture-past-677/bootB.log` (the run whose capture shows the `PRESS ⊕` prompt), `bootA.log`.

## Do-Not entries that apply here (2 of 9)

- #8 — **Do not name a stall by the frame one run happened to stop at.** Report the range and the runs.
- #9 — **Do not model a virtual core without rotating equal-priority threads.** A woken sibling that can only run on that core is the same as no wakeup.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes.
- A bounded headless boot with `TOTK_PRESENT_WATCHDOG` that presses ⊕ and captures before and after; every claim tied to a recorded observation; no gameplay claim and no visual-correctness claim beyond captured frames.

## When this slice closes

Write the next seed from what S2.11 leaves behind, re-verify the Do-Not count (9 or higher), and commit the state, the feasibility record and the seed.
