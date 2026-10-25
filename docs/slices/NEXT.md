---
slice: S4.1b
stateFile: docs/STATE.md
donotCount: 34
seededFrom: the S4.1a close (a self-contained 6.9 GB app launches from outside the repo and reaches the game's first-run screens; the main menu and the race in the packaged app are unobserved)
plannedTokens: 150000
---
# Next slice — S4.1b: the packaged app, driven

## The next action

S4.1b — Packaged smoke from the bundle. Launch the app S4.1a built and drive it past the first-run screens: the EA
User Agreement scrolls with a held D-pad-down (`down 5`, ~20–30 s, `up 5`) then Ⓐ, the same trigger that writes the
save and loads `MAIN_MENU.BNDL` in a dev-tree boot. Capture the main-menu frame, the `[nfs] fs mount save:` line and
the `MAIN_MENU.BNDL` line, then the race the same way S3.3a did, from the app alone: no work-tree path, no env var,
no working directory. Report what differs from the dev-tree captures (the packaged app has no `build/shader-libs`
unless a shader cache existed when it was sealed, so the first frame translates on demand). The operator's own windowed
run of the lit race is still open and remains the review point after this.

Closes when: the packaged `Need for Speed Hot Pursuit.app` shows its main menu and a race frame, launched by
double-click or `open`, with the save mount and the menu in `~/Library/Logs/MacNativeGames/nfs.log` — and nothing
outside the bundle read, set or present.

## What S4.1a settled — do not re-derive

- **The build command and its result.** `python3 build_app.py target-port/need-for-speed-hot-pursuit-remastered/game.nsp
  -k …/prod.keys --work /Volumes/X9BS/externalTmp/MacNativeGames/nfs-app -o "/Volumes/X9BS/externalTmp/MacNativeGames/apps/Need for Speed Hot Pursuit.app"`
  exits 0 (14 min cold, **17 s** warm) and produces a 6.9 GB app whose seal verifies. Logs:
  `.babysteps/scratch/S4.1a/package.log`, `package-2.log`. The data bundle is stamped by the RomFS image's size/mtime/inode,
  so a rebuild reuses it.
- **The bundle layout.** `Contents/MacOS/NFS`; `Contents/Resources/nfs/{build/nfs-native, build/shader-service/totk_xlate,
  build/spirv-cross/spirv-cross, rewritten/nfs100/*.patched.bin, extracted/nfs100/exefs/main.npdm,
  extracted/nfs100/romfs/runtime-index.bin, extracted/firmware/shared-fonts/}`; `Contents/Helpers/GameData-nfs100.bundle`
  (6.9 GB) with `extracted/nfs100/romfs/romfs.bin` a relative symlink into it. **No `build/shader-libs`** — the work tree
  had no runtime shader cache, so `make shader-libs` produced nothing and this bundle translates every shader on demand.
- **Where it writes.** Profile/save, game slot, RomFS proof and the shader cache are under
  `~/Library/Application Support/MacNativeGames/NFS/` (`TOTK_PROFILE_DIR`, `TOTK_GAME_SLOT_DIR`, `TOTK_ROMFS_PROOF_DIR`,
  `TOTK_SHADER_CACHE`, `DM_MSL_DIR`); the log is `~/Library/Logs/MacNativeGames/nfs.log` with `nfs.previous.log` kept. The
  bundle itself is never written.
- **It runs from outside.** Load set mapped from the bundle, translator and `spirv-cross` started from it, `save:` mounted
  to `…/NFS/profile/save/1d8452940529d7e44453c82f71e05f4e`, title screen / EA privacy policy / EA user agreement drawn from
  the bundle with keys answered (`boot-1.log`, `menu-final.png`, `main-menu.png`, `menu-after-back.png`). `strings` on the
  binary: **0** hits for the work-tree or `games/nfs` paths.
- **Not reached:** the main menu. A 12 s held scroll did not clear the agreement; the dev-tree ladder used held D-pad
  presses until the end of the text. That is this slice's observation, not a new fault.
- **Gate:** 264 tests, 1 skipped, exit 0 (`.babysteps/scratch/S4.1a/gate.log`).

## The operator's review point is still open

- His own windowed run of the lit race, from the packaged app now — offer it at this slice's close with `runCommand`
  replaced by `open "/Volumes/X9BS/externalTmp/MacNativeGames/apps/Need for Speed Hot Pursuit.app"`, and name the log and
  save-file hashes.

## Code and data to touch

| Work | Touch |
| --- | --- |
| Drive the app to the menu and the race | the app's own `input.txt` channel: `~/Library/Application Support/MacNativeGames/NFS/input.txt` (`down 5` / `up 5`, `key 38 200`, `wshot <path>`, `close`) |
| If the scroll still does not clear it | `games/nfs/app/launcher.m` (nothing should need to change: the runtime reads the file), or the ladder in `.babysteps/scratch/S3.3c-save/` as the reference cadence |
| Optional: precompiled shaders in the bundle | `make shader-libs` needs a filled `build/shader-cache/rt-<chain>`; boot the app once, then re-seal, and the second bundle loads them (`Do-Not #24` names the lookup that consumes them) |
| Do not | do not touch `games/totk` or `games/by3`; do not make the app depend on the work tree; do not re-slice packaging |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (264 tests, 1 skipped at
  the S4.1a close).
- Evidence is the packaged run's own log and frames, in `.babysteps/scratch/S4.1b/`. A frame is a claim only with its
  capture named; a boot's stop rate is reportable only with the GPU otherwise free (`Do-Not #14`).
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (`Do-Not #11`); append any new fault after #34 in `docs/donot.md`; rewrite this seed from what S4.1b leaves behind,
  under 120 lines (S4.2 — the validation matrix — is queued).

## Read these first, and only these

- `.operator/specs/nfs-port.md` § Manifest authority · § Runtime load set — the six-file load set the bundle carries, and
  the manifest that authorises every byte in it.
- `.operator/guides/running-the-nfs-boot.md` — the input commands, the cadence rule, the ~1 min game-slot wait, the ~0.1 s
  guest-reservation retry, and the packaged app's own data folder.
- `.operator/product/launch-path.md` § The launch path, in order (1–4 done after this slice; 5 = S4.2) · § Release-claim
  boundary.
- `games/nfs/README.md` — what the work tree and the app each carry.

## Do-Not entries that apply (#, body not quoted)

#6 · #11 · #14 · #19 · #22 · #27 · #31 · #32 · #33 · #34
