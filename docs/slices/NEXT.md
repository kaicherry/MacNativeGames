---
slice: S3.1d
stateFile: docs/STATE.md
donotCount: 25
seededFrom: the S3.1c close (savedata and ns:save_data registered with four sub-service factories)
plannedTokens: 160000
---
# Next slice — S3.1d: save round-trip

## The next action — verbatim

S3.1d — Save round-trip: the fs host wrote a 553 KB save-sized file at 44% `0xcd` fill in the S3.1a windowed run. Verify the guest's savedata read/write path against a known save image, restart the boot, and confirm the game proceeds past the save-data notice. Answer every `savedata` and `ns:save_data` command the guest sends; carry returned objects or fail (Do-Not #23). Do not touch `games/totk` or `games/by3`.

S3.1d closes when a repo-built windowed boot reads the established save image through the `savedata` service and the game proceeds past the save-data notice into the main menu or a race.

## What the next session does first

1. Read `games/nfs/runtime/savedata_service.c` (what S3.1c installed) and `games/nfs/runtime/fs_host.h` (the save directory mount). Read the S3.1c section of `docs/STATE.md`.
2. Build from the repo; boot windowed with the established save file placed where the fs host mounts it.
3. Trace the guest's `savedata` and `ns:save_data` commands against the log; answer any unimplemented commands with `rc 0` or `rc` naming the stub.
4. Verify the guest can read the established save image and proceed past the notice → accept path into content.
5. Capture the log under `.babysteps/scratch/S3.1d-savert/`.

## What is settled — do not re-derive

- **The save-data file:** 553 KB, 44% `0xcd` fill (S3.1a). Not a working save; the content is placeholder.
- **The service pattern:** constructor + `runtime_ipc_register`; four sub-service factories (saveDataBinary, saveDataAudit, saveDataConfig, ISaveData) installed by S3.1c.
- **The save-data notice:** ⊕ raises it; carries `Press Ⓐ to continue`.
- **The title ID:** `0x010029B0118E8000`.
- **The gate** passed 261 tests, 1 skipped, exit 0.

## Withdrawn — proven wrong, do not use

- **"The save is a working save."** It is 44% `0xcd` fill.
- **"No `savedata` request was ever sent."** S3.1c registered handlers; a windowed boot may send commands the headless run did not.

## Code to touch

| Work | Touch |
| --- | --- |
| The service | `games/nfs/runtime/savedata_service.c` — flesh out any commands the guest sends in a windowed boot; implement read/write handlers as needed. |
| Evidence | `.babysteps/scratch/S3.1d-savert/`: the windowed boot log naming the opens, the `savedata` command ids, and the result. |
| Do not | Do not touch `games/totk` or `games/by3`; do not answer a command before reading the ids the guest sends; do not read a rate or frame count taken under GPU contention (#14). |

## Read these first, and only these

- `.operator/specs/nfs-port.md` — service/provider-resolution stance.
- `.operator/guides/running-the-nfs-boot.md` — the repo build and the windowed boot.
- `docs/STATE.md` § "S3.1a", "S3.1c", "Next action".
- `docs/donot.md` #6, #11, #14, #19, #22, #23.

## Do-Not entries that apply here (6 of 25)

- #6 — a fill pattern is not data (the save is 44% `0xcd`).
- #11 — advance every top section of STATE.md at close.
- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #23 — a result and a returned value are two fields; carry the object or fail.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 160k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Write the next seed from what S3.1d leaves behind, re-verify the Do-Not count (25 or higher), and commit the state and the seed. The queue after this is S3.2a (inventory every `sm` open after saves).
