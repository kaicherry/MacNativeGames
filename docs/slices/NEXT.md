---
slice: S3.1c
stateFile: docs/STATE.md
donotCount: 25
seededFrom: the S2.27 close (the wait summary prints; the prediction path does not engage in an attract boot)
plannedTokens: 150000
---
# Next slice — S3.1c: implement the `savedata` service

## The next action — verbatim

S3.1c — Implement the `savedata` service: its open, the mount/commit handshake, and the Ⓐ notice path. The
guest reaches the save-data access notice (⊕ at the `PRESS ⊕` title raises it, and that screen carries
`Press Ⓐ to continue`, S2.11) but sends no `savedata` request — zero occurrences in the S3.1a log — while the
fs host mounts the save directory and, in S3.1a's windowed run, wrote a 553 KB save-sized file at 44% `0xcd`
fill, so its content is not established. Register `savedata` and the family the guest opens, carry the object
the SDK reads in the reply (Do-Not #23), and reproduce the notice→accept path in a repo-built windowed boot.
Do not touch `games/totk` or `games/by3`.

S3.1c closes when a repo-built windowed boot opens `savedata`, completes the mount/commit handshake, and the
⊕→Ⓐ notice path proceeds past the save-data notice, or when the absence of a `savedata` request is
re-measured and recorded with the reason.

## What the next session does first

1. Read `games/nfs/runtime/pctl_service.c` (the constructor + `runtime_ipc_register` pattern) and
   `games/nfs/runtime/services.c`; read the S3.1a/S3.1b sections of `docs/STATE.md`.
2. Add a `savedata` handler beside `pctl_service.c` and register it (`runtime/*.c` is globbed, so a new file
   needs no manifest edit). Add the fs mount/commit handshake.
3. Re-measure the guest's opens on a windowed boot; name the exact `savedata` command ids the guest sends
   before answering them (Do-Not #23 — carry the object or fail).
4. Build from the repo (Do-Not #19) and capture the log under `.babysteps/scratch/S3.1c-savedata/`.

## What is settled — do not re-derive

- **The service pattern**: a constructor calls `runtime_ipc_register(name, handler, …)`; the whole `pctl`
  family is registered this way (`games/nfs/runtime/pctl_service.c:66`).
- **The guest sends no `savedata` request today** (zero in the S3.1a log). The fs host mounts the save
  directory and wrote a 553 KB save-sized file at 44% `0xcd` fill.
- **The save-data notice is reachable**: ⊕ (mac key 36) at the `PRESS ⊕` title raises it; the notice carries
  `Press Ⓐ to continue` (S2.11).
- **The per-submit wait summary prints** (S2.27); unrelated to savedata.
- **The gate** passed 261 tests, 1 skipped, exit 0.

## Withdrawn — proven wrong, do not use

- **"The save is a working save."** It is 44% `0xcd` fill.
- **"The prediction path engages in a bounded boot."** It does not in the attract loop (S2.27); a play run
  engages it only once driven into content beyond the attract loop.
- **"No preload in a dev tree"** and **"the `b-<chain>` bundle is the fix"** remain withdrawn (S2.26).

## Code to touch

| Work | Touch |
| --- | --- |
| The service | new `games/nfs/runtime/savedata_service.c` (constructor + `runtime_ipc_register`), beside `runtime/pctl_service.c`; registry/initializers in `runtime/services.c`. |
| Evidence | `.babysteps/scratch/S3.1c-savedata/`: the windowed boot log naming the opens, the `savedata` command ids, and the mount/commit handshake. |
| Do not | Do not touch `games/totk` or `games/by3`; do not answer a command before reading the ids the guest sends; do not read a rate or frame count taken under GPU contention (#14). |

## Read these first, and only these

- `.operator/specs/nfs-port.md` — service/provider-resolution stance.
- `.operator/specs/nfs-account.md` — the local-HLE service pattern (a service answered after init), as precedent.
- `.operator/guides/running-the-nfs-boot.md` — the repo build and the windowed boot.
- `docs/STATE.md` § "S3.1a", "S3.1b", "Next action".
- `docs/donot.md` #6, #11, #14, #19, #22, #23.

## Do-Not entries that apply here (6 of 25)

- #6 — a fill pattern is not data (the save).
- #11 — advance every top section of STATE.md at close.
- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #23 — a result and a returned value are two fields; carry the object or fail.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261
  tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 150k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Write the next seed from what S3.1c leaves behind, re-verify the Do-Not count (25 or higher), and commit the
state and the seed. The queue after this is S3.1d (save round-trip); S5 is provisional.
