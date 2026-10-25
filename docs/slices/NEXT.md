---
slice: S4.1a
stateFile: docs/STATE.md
donotCount: 32
seededFrom: the S3.3c close (the save round-trip is observed; nothing is packaged)
plannedTokens: 150000
---
# Next slice — S4.1a: package the `.app`

## The next action

S4.1a — Package the app. The port reaches a lit, drivable race and the save written from inside the game is read
back by two boots, so the next thing a player needs is something to double-click. `build_app.py` already knows the
title (`TITLES['010029B0118E8000'] = 'nfs'`), the app identity (`APPS['nfs'] = ('Need for Speed Hot Pursuit',
'org.macnativegames.nfs', 'NFS')`, :67), and the code/data steps (`step_code_nfs` :1537, `step_data_nfs`), but the
**app** step is TOTK/BOTW-shaped: `step_app` (:1226) builds `Contents/Resources/totk/{build/totk-native, rewritten/…,
extracted/…, build/shader-seed, build/shader-libs}` from `VERSIONS`/`data_bundle`/`place_variant_libs` and requires a
prepared startup shader seed. NFS has a different binary, load set, shader cache and no seed requirement.

Closes when: a repo-built `build_app.py` run produces a `.app` for `nfs100` that launches the game's own window from
the bundle with no dev-tree dependency — the bundle names its own resources, the first frame presents from a path
outside `games/nfs` and the work tree, and the log shows the save mount and the main menu. Say what the bundle
contains (file list with sizes), where it reads its data from, and which command produced it. A bundle that needs an
env var, a working directory or a file beside it to start is not closed.

## The operator's review point is still open

- His own windowed run of the lit race was the standing observation before this slice and it is not closed by
  packaging. Offer it again at the close: `runCommand`, repo-built, log and save-file hashes named.

## What S3.3c settled — do not re-derive

- **The write trigger is the EA User Agreement.** An **empty** profile dir (`user-data/save/788fd3e89add37f8ef8bbba5a2a0cc5c/`)
  makes the game show the agreement; accepting it (scroll with held D-pad presses, then Ⓐ) writes `save:/MUD64.8`
  **one second after `MAIN_MENU.BNDL`** (`write` at 143.191 s, menu at 142.325 s).
- **The round trip.** That file is 552,976 bytes, born 2026-10-25 13:23:22, `sha256 31ff5c2e48f439e9338762f5fcbddc695a5463d5958dc6500e57f20d258875b8`. Two further boots
  open it **read only** (`read` at 62.221 s / 62.077 s), leave hash and mtime alone, and skip the agreement — the
  acceptance is in the bytes.
- **An absent save is not created by a boot.** Three boots mounted, attempted the read, failed, wrote nothing and
  re-showed the agreement.
- **The file is fill**: 54.7% `0x00`, 44.2% `0xcd`, one 258-byte live header. The round trip is a claim about bytes;
  no usable profile and no valid save is claimed (Do-Not #6). The stale 384 bytes at `0x1d094..0x1edbd`
  (mtime 2026-10-24 20:07:57) are unattributed and unclaimed.
- **An fs open is not an operation**: `[totk-fs] read|write <path>` is logged before the `stat` (Do-Not #31), and
  a window close exits at once unless a save is already in flight (Do-Not #32).
- **No runtime code changed.** `fs_host.c`'s open/write/close/commit path was already complete.

## Code and data to touch

| Work | Touch |
| --- | --- |
| The app step | `build_app.py` `step_app` (:1226); `sign_app` (:1182); `install_app`; `place_data_bundle`, `place_variant_libs`, `seed_digests` — decide parameterise vs a `step_app_nfs` |
| App identity | `build_app.py:67` `APPS['nfs']` (present), `TITLES`, `VERSIONS['nfs100']`, `NFS100` |
| The launcher's resource folder | `games/nfs/…` `build/app-launcher` + `launcher.m` (it looks under `Contents/Resources/<folder>`; TOTK's is `totk`) |
| What the bundle must carry | `build/nfs-native`, `rewritten/nfs100/*`, `extracted/nfs100/romfs/runtime-index.bin`, `build/shader-libs/`, the shader cache/libs the NFS loader reads (`make shader-libs`), `tools/` only if the runtime needs it |
| Not applicable | NFS has no startup shader seed (`TOTK_SHADER_SEED` is inert in a dev-tree run), no AOC volumes, no `pso_manifest.tsv` pair |
| Do not | Do not touch `games/totk` or `games/by3`; do not make the bundle depend on the work tree |

## Read these first, and only these

- `.operator/product/launch-path.md` — what a launch still needs, and the release-claim boundary.
- `.operator/specs/nfs-port.md` § Manifest authority, § Runtime load set — what the bundle may not re-derive.
- `.operator/guides/running-the-nfs-boot.md` — repo build, work tree, windowed launch, the slot wait.
- `docs/donot.md` #6, #11, #19, #22, #24, #31, #32.

## Already measured — do not re-measure as new

- Gate baseline: **261 tests, 1 skipped** (`.babysteps/scratch/S3.3c-save/gate.log`, 24.6 s, exit 0).
- The save file, its two read boots and the three absent-save boots: `.babysteps/scratch/S3.3c-save/`
  (`roundtrip.txt`, `agreement-a1.txt`, `boot-a1-*.log`, `boot-r*.log`).
- The lit race frame: `.babysteps/scratch/S3.3d-constants/60-race-fixed.png`.
- Do-Not count is **32**; the last entry is #32.

## Do-Not entries that apply here (7 of 32)

- #6 — a guest-written save may be fill; content establishes nothing.
- #11 — advance all five `docs/STATE.md` sections in the closing commit.
- #19 — build from the repo; re-sync the work tree and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #24 — do not build a bundle to stop a dev-tree boot compiling its shaders; read the lookup that consumes it.
- #31 — an fs open logged in `[totk-fs]` is an attempt, not a result.
- #32 — a window close does not let the guest finish a save.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests,
  1 skipped at the S3.3c close).
- A packaged launch is the observation this slice rests on; its log and the bundle's own file list are the evidence.

## When this slice closes

Advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit (Do-Not
#11); append any new fault after the last entry in `docs/donot.md`; rewrite this seed from what S4.1a leaves
behind, under 120 lines.
