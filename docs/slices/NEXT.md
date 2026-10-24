---
slice: S3.3a
stateFile: docs/STATE.md
donotCount: 27
seededFrom: the S3.2a close (the post-save-mount sm inventory is empty; the menu path is service-free)
plannedTokens: 160000
---
# Next slice — S3.3a: reach the first controllable frame (menus → race start)

## The next action — verbatim

S3.3a — Reach the first controllable frame (menus → race start): drive the repo-built windowed boot past the pre-race briefing (`Ⓐ CONTINUE` on `game-c.png`) into the race, and establish a frame the player can steer. S3.2a withdrew S3.2b's subject: six windowed boots past the save mount open **no** `sm` service after it, so there is no offline service to implement and the menu path is service-free. Do not touch `games/totk` or `games/by3`.

S3.3a closes when a repo-built windowed run goes from the pre-race briefing into the race and the car responds to a scripted press — a capture and its log named — or the stop it waits on is named with the service, fault or condition behind it.

## What the next session does first

1. Rebuild from the repo (Do-Not #19): `rsync -a` from `games/nfs/` with `build_app.py`'s excludes into `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs/`, `rm -rf build/obj build/nfs-native`, `make WORK=. runtime`.
2. Reuse the exact script that reached the pre-race briefing (`.babysteps/scratch/S3.2a-inventory/run-gamestart.sh`): ⊕ 58 s, Ⓐ 64 s, Ⓐ 72 s, Ⓐ 97 s, Ⓐ 127 s, Ⓐ 162 s, Ⓐ 192 s, Ⓨ 225 s. It ends on "ROADSTERS REBORN … Ⓐ CONTINUE".
3. From there press Ⓐ (`key 38`) to enter the race, then hold accelerate — `key 32` is Ⓨ, the footer's START; confirm which button the race uses before assuming. Capture a shot before and after, and read `[totk-fs]` / `[totk-ipc]` for what the transition asks.
4. Capture the log under `.babysteps/scratch/S3.3a-controllable/`.

## What is settled — do not re-derive

- **No `sm` service gates the menu path.** Six repo-built windowed boots opened nothing after the save mount; the only opens are `hid`, `friend:u`, `audout:u`, all before it, and no unregistered name was ever asked (`.babysteps/scratch/S3.2a-inventory/inventory.md`). `⊕ CONNECT TO AUTOLOG` fails by itself with CONNECTION FAILED, asking `sm` for nothing.
- **The boot reaches the pre-race briefing.** `game-c.png`: "ROADSTERS REBORN / RACE · SPORTS", bounty reward table, `Ⓐ CONTINUE`; the track and its props are drawn behind it. That is past the menu, the race-location map, the event list, car select and colour select — all without a new service.
- **The `sm` dispatch is `games/nfs/runtime/ipc.c:252`**, not `services.c` (`services.c` is memory and virtual-core). A miss logs `sm: service "X" not implemented` and calls `runtime_fail("IPC service not implemented")`.
- **The `key` schedule command takes a Mac keycode below 128** (`runtime/input.c:160`). ⊕ = `key 36`, Ⓐ = `key 38`, Ⓨ = `key 32`; `1024` is the pad bit ⊕ becomes, not a code the file takes (Do-Not #27).
- **A windowed boot reaches guest state a headless one does not** (Do-Not #22), and the scripted windowed run does reach it (S3.1b, S3.1d, S3.2a).
- **The gate** passed 261 tests, 1 skipped, exit 0 (`.babysteps/scratch/S3.2a-inventory/gate.log`).

## Withdrawn — proven wrong, do not use

- **"S3.2b: movies and the offline services the inventory names."** The inventory names no offline service; the queue item is withdrawn and movie playback is a visual check inside S3.3, not a service slice.
- **"A service gates leaving the main menu."** None does — measured across six boots, and recorded in `.operator/specs/nfs-port.md` § Adding an IPC service.
- **"The `sm` dispatch is in `services.c`."** It is `ipc.c:252`.
- **"The save is a working save."** It is 44.2% `0xcd` / 54.7% `0x00`; the menu shows a fresh default profile (Do-Not #6). The race may write one — check before reading its contents as state.

## Code to touch

| Work | Touch |
| --- | --- |
| The measurement | No runtime change until a stop names one. Read `games/nfs/runtime/input.c` (the `button()` map and the pad read), `games/nfs/runtime/services.c` (`runtime_core_checkpoint`), and `games/nfs/runtime/ipc.c` (`sm`) only if the run stops on one. |
| Evidence | `.babysteps/scratch/S3.3a-controllable/`: the windowed log, the before/after captures, and the run script. |
| Do not | Do not touch `games/totk` or `games/by3`; do not register a service family the guest never asks `sm` for; do not read a rate or frame count taken under GPU contention (#14). |

## Read these first, and only these

- `.operator/guides/running-the-nfs-boot.md` — the repo build and the windowed boot.
- `.operator/specs/nfs-port.md` — Claim boundaries; Input path; Adding an IPC service.
- `docs/donot.md` #6, #14, #19, #22, #27.

## Do-Not entries that apply here (5 of 27)

- #6 — do not read a struct the SDK copies in full as data; a fill pattern is not a value (and the save is fill).
- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #27 — pass a Mac keycode below 128 to `key`, not a pad bit.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 160k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured. This slice's subject is the operator's own experience — a controllable frame — so his run is the evidence.

## When this slice closes

Advance "Where the work is", "Next action" + gate + closes-when + Do-Not count, "Completed", "In progress" and "Queued" in the same commit (Do-Not #11); append any new fault after the last existing entry; rewrite this seed from what S3.3a leaves behind. The queue after this is S3.3b (complete a representative race).
