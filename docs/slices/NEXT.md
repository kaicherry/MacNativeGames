---
slice: S3.2a
stateFile: docs/STATE.md
donotCount: 26
seededFrom: the S3.1d close (the save read path is nn::fs; the boot reaches the main menu)
plannedTokens: 160000
---
# Next slice — S3.2a: inventory every `sm` open after the save mount

## The next action — verbatim

S3.2a — Inventory every `sm` open after the save mount: read the boot's `[totk-ipc]` opens from the Ⓐ accept onward, classify each as implemented, absent, or never-asked, and name the first service the boot needs to leave the main menu. S3.1d's windowed run to the main menu opened only `hid`, `friend:u` and `audout:u`, so the inventory's subject is what opens past the menu ("CONNECT TO AUTOLOG" is presented but not walked). Do not touch `games/totk` or `games/by3`.

S3.2a closes when every `sm` open after the save mount is listed with its owning source file and its implemented / absent / never-asked status, and the next service the boot needs is named.

## What the next session does first

1. Read `games/nfs/runtime/services.c` (the `sm` dispatch and the `service "X" not implemented` fail) and `runtime/system_services.c`, and list every `runtime_ipc_register` caller in `games/nfs/runtime`.
2. Reproduce S3.1d's windowed boot: force-clean the work build, `make -C games/nfs WORK=/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs runtime` (Do-Not #19), then windowed with the ⊕(36)/Ⓐ(38) schedule at ~58 s / ~64 s and `TOTK_INPUT_LOG=1 TOTK_FS_LOG=1`; read the `[totk-ipc] opened service` and `not implemented` lines from the Ⓐ accept onward.
3. Drive past the main menu (select CAREER, or press Ⓐ on "CONNECT TO AUTOLOG") and watch for the next `sm` open.
4. Capture the log under `.babysteps/scratch/S3.2a-inventory/`.

## What is settled — do not re-derive

- **The save path is `nn::fs`, not `nn::ns::savedata`.** `main.patched.bin` imports `nn::fs::MountSaveData` / `nn::fs::EnsureSaveData` and **no `nn::ns` symbol**; `savedata` / `ns:save_data` is never opened (S3.1d; Do-Not #26).
- **The boot reaches the main menu.** ⊕ (key 36) raises the notice, Ⓐ (key 38) accepts, the fs mounts `save:`, the guest reads `save:/MUD64.8` (`[totk-fs] 62.913 read save:/MUD64.8`), and the menu appears (`.babysteps/scratch/S3.1d-savert/window-end3.png`). Log `boot-windowed3.log`.
- **The only `sm` opens to the menu are `hid`, `friend:u`, `audout:u`.** No `pctl` open on this schedule, unlike S3.1a's operator run.
- **A windowed boot reaches guest state a headless one does not** (Do-Not #22), and the scripted windowed run does reach it (S3.1b, S3.1d).
- **The gate** passed 261 tests, 1 skipped, exit 0 (`.babysteps/scratch/S3.1d-savert/gate.log`).

## Withdrawn — proven wrong, do not use

- **"The save round-trip runs through the `savedata` service."** It runs through `nn::fs` (`read save:/MUD64.8`); the family is never opened (Do-Not #26).
- **"The save is a working save."** It is 44.2% `0xcd` / 54.7% `0x00`; the menu shows a fresh default profile (Do-Not #6).

## Code to touch

| Work | Touch |
| --- | --- |
| The inventory | `games/nfs/runtime/services.c` (the `sm` dispatch and its `not implemented` path), `runtime/system_services.c`, and every `runtime_ipc_register` caller in `games/nfs/runtime`. |
| Evidence | `.babysteps/scratch/S3.2a-inventory/`: the windowed boot log with its opens, and the classification table. |
| Do not | Do not touch `games/totk` or `games/by3`; do not answer a command before reading the ids the guest sends; do not read a rate or frame count taken under GPU contention (#14). |

## Read these first, and only these

- `.operator/specs/nfs-port.md` — Adding an IPC service; Save path; Provider stance.
- `.operator/guides/running-the-nfs-boot.md` — the repo build and the windowed boot.
- `docs/STATE.md` § "S3.1a", "S3.1b", "S3.1d", "Next action".
- `docs/donot.md` #14, #19, #22, #23, #26.

## Do-Not entries that apply here (5 of 26)

- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #23 — a result and a returned value are two fields; carry the object or fail.
- #26 — do not register or answer a service family the guest never asks `sm` for; read the module's own imports first.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 160k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured.

## When this slice closes

Advance "Where the work is", "Next action" + gate + closes-when + Do-Not count, "Completed", "In progress" and "Queued" in the same commit (Do-Not #11); append any new fault after the last existing entry; rewrite this seed from what S3.2a leaves behind. The queue after this is S3.2b (movies and the offline services the inventory names).
