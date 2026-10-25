---
slice: S3.3e
stateFile: docs/STATE.md
donotCount: 34
seededFrom: the S4.1b close (the operator ran the packaged app through a full race; every packaging fix holds; one render artefact is left)
plannedTokens: 150000
---
# Next slice — S3.3e: the black-square artefact

## The next action

S3.3e — Name the black-square artefact. In a captured race frame the car's silhouette carries small black squares
in a staircase (about 8–10 px in a 1920×1080 capture of a 1600×900 render), along the rear, wheel arch, mirror and the
top of the windshield frame, and — crucially — against the sky and road and in the winding-flipped frame too, so they
are not a ground shadow and not the car's shaded colour (`.babysteps/scratch/S4.1a/zoom-left.png`, `zoom-roof.png`,
`edge-60-race-flip.png`). They are absent from the pre-rendered title and EA screens. The next step is the measurement
this project already has tooling for: capture one frame that shows them from the packaged app, with `DM_NVN_PASSDUMP`
for `cull=`/`front=`/`samples=` per draw and `DM_NVN_TEXDUMP=<dir>/f-%d.png` + `DM_NVN_TEXBIND` for the targets and
binds, then read which pass writes the squares and whether they are in a texture that is sampled (a tiled or partially
uploaded texture) rather than in the scene target itself. Cheap switches that split the candidates without a rebuild:
`DM_NVN_NOFXAA=1` (the FXAA full-screen draw reads the tone-mapped picture), `DM_NVN_RT_AB=0` (the runtime-extrapolation
path, off by default), and `DM_NVN_DBG_SER=<serial>` for a target's load/store actions. Closes when a captured frame
names the target or texture carrying the squares and one run with one switch removes them, or shows the cause is in the
draw itself.

## What S4.1b settled — do not re-derive

- **The packaged app is the thing to capture from.** The operator ran it through a full race (13,500 frames,
  `~/Library/Logs/MacNativeGames/nfs.log` 2026-10-25 15:19, 83.7 render passes/frame); audio is audible on the bound
  device, `601 loaded precompiled, 0 compiled on demand`, no developer HUD, Game Mode on. The app is
  `/Volumes/X9BS/externalTmp/MacNativeGames/apps/Need for Speed Hot Pursuit.app`; rebuilds over the same work folder
  take under a minute and keep the signed data bundle.
- **The artefact, as measured from the existing capture.** Blocks are axis-aligned, roughly 8–10 px at 1080p, hug the
  silhouette from outside, appear on the shadowed and lit sides, are unchanged by `TOTK_GPU_WINDING_FLIP` (so not the
  culled-draw fault of S3.3d), and are present where the background behind them is sky or road (so not a cast shadow).
  They do not appear in the game's pre-rendered title, EA privacy or agreement screens.
- **What is already ruled out for the 3D path (S3.3b/S3.3d).** Format table, upload content, handle resolution,
  sampling, the shadow atlas, layered-pass merging, constant delivery. See `.operator/specs/nfs-render.md` before
  naming a new suspect.
- **How the app is driven.** `~/Library/Application Support/MacNativeGames/NFS/input.txt` (the launcher sets
  `TOTK_INPUT_FILE`): `key <code> <ms>` (A=38, Y=32, D-pad down=5, B=40), `down/up <code>`, `shot`/`wshot <path>`,
  `shotn <n> <path with %d>`, `close`. The race start needs the S3.3a cadence, not per-screen sleeps; SELECT COLOR
  answers Ⓨ.
- **Diagnostics that exist.** `DM_NVN_PASSDUMP` (`P`/`d`/`B` records, `samples=` in the `P` record),
  `DM_NVN_TEXDUMP=<dir>/f-%d.png`, `DM_NVN_TEXBIND`, `DM_NVN_DBG_SER=<serial>`, `DM_NVN_NOFXAA=1`, `DM_NVN_RT_AB=0`,
  `DM_NVN_SHMAP`, `DM_NVN_DIRTYCHECK`/`DM_NVN_DIRTYFIX` (dirty-page tracking of packaged textures). Read
  `.operator/guides/nfs-frame-dumps.md` first — it names each record and where the translated Metal for a shader key
  lives.
- **Packaging now also carries the app icon** (`games/nfs/scripts/extract_app_icon.py` → `build/AppIcon.icns`), so a
  rebuild does not lose it.
- **Gate:** 264 tests, 1 skipped, exit 0.

## Code and data to touch

| Work | Touch |
| --- | --- |
| The frame capture | `wshot`/`shotn` from the app's `input.txt`, with the dump env vars passed through `open --env` |
| Where the squares are written | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` (pass records, FXAA at ~:9826, the runtime-extrapolation region path at ~:10719) |
| If they are in a texture | `nvn_tex.c` (block-linear layout/upload), `nvn_dirty.c` (page-level re-upload of packaged textures) |
| Do not | do not touch `games/totk` or `games/by3`; do not rebuild the app to test a switch (they are all environment); do not name a cause without the captured frame |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (264 tests, 1 skipped).
- A frame is a claim only with its capture named; a boot's stop rate is reportable only with the GPU otherwise free
  (`Do-Not #14`). Evidence goes in `.babysteps/scratch/S3.3e/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (`Do-Not #11`); append any new fault after #34 in `docs/donot.md`; rewrite this seed from what S3.3e leaves behind,
  under 120 lines (S4.2, the validation matrix, is queued).

## Read these first, and only these

- `.operator/specs/nfs-render.md` — what the 3D path is measured to do, the winding inversion, how to read an empty
  target, and what earlier slices ruled out.
- `.operator/guides/nfs-frame-dumps.md` — `P`/`d`/`B` records, `f-<n>.png` names, and finding a shader key's Metal.
- `.operator/guides/running-the-nfs-boot.md` — the input commands, the cadence rule, the game-slot wait and the
  ~0.1 s guest-reservation retry.

## Do-Not entries that apply (#, body not quoted)

#6 · #14 · #16 · #19 · #22 · #24 · #27 · #30 · #31 · #32 · #33 · #34
