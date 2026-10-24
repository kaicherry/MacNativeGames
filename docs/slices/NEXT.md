---
slice: S3.3d
stateFile: docs/STATE.md
donotCount: 29
seededFrom: the S3.3b close (the textures arrive; the shading that turns them into colour returns zero)
plannedTokens: 160000
---
# Next slice — S3.3d: restore surface radiance

## The next action — verbatim

S3.3d — Make the race's 3D surfaces lit: the G-buffer proves the textures reach the shader, so the fault is in the shading that turns them into colour. In one race frame three shaded targets are black while the G-buffer beside them is correct. Read the shading math and its constants for one lit draw, name the term that is zero, and fix it. Do not touch `games/totk` or `games/by3`; do not "fix" it by forcing a default texture or a constant light.

S3.3d closes when a repo-built windowed run draws one lit, textured surface in the race — a capture and its log named — or the zero term is named with the shader, the constant and the value that places it there.

## What S3.3b settled — do not re-derive

- **Textures reach the shader.** In passdump frame 8916 (`.babysteps/scratch/S3.3b-texbind/boot-race4.log`) the G-buffer pass **549787** (520 draws, 2,740 sampled-texture binds) writes ser6/ser7/ser8 at 1920×1080 plus depth ser9. A texdump of the adjacent frame 8915 shows **ser7 = the road, cliff, grass and trees fully textured** (`texdump/f-700.png`) and **ser6 = correct normals** (`f-600.png`). Upload, handle resolution and sampling are therefore all working on the same surfaces, in the same frame, with the same textures.
- **Handle resolution is excluded by measurement.** `DM_NVN_TEXBIND` (`nvn_mtl.m`, `texbind_note`/`texbind_roll`) counts sampled-texture binds that resolve to the all-zero dummy: **0 dummy** in every sampled frame of the race, against 2,330–3,426 real binds per frame (`boot-race2.log`, `boot-race4.log`). A shader reading a zero albedo would have shown here and did not.
- **The shadow atlas is not missing.** ser53 (1024×3072 depth) is rendered by pass **549771** in the same frame and is sampled through a real comparison sampler (`B … samp reg15 sid=266 cmp=1/2`). An absent shadow map is excluded.
- **Every shaded output is black.** ser44 (pass **549788**, 3 draws — sky/atmosphere, `f-4400.png`), ser10 (pass **549789**, 330 draws, 2,032 binds — the shaded scene, `f-1000.png`), the final ser76 (`f-7600.png`) and the captured race (`50-race-a.png`) all show correct sky, fog, HUD and pre-rendered backdrops with **zero surface radiance**. The loss is not one target.
- **The ambient chain is black at its root.** The environment cube ser12 (256×256, six faces, RG11B10Float) is entirely black (`f-1200.png`), and the 32×32 pair ser14/ser15 derived from it are black (`f-1400.png`, `f-1500.png`). ser14/ser15 are read 82 times by 549789, so the main pass's ambient input is zero by construction.
- **The pass dump's draw line is lowercase `d`.** One `d` line per draw (with `vs=`/`fs=`); uppercase `D` is a different record (`prim=`, or a bloom note). Counting `D` makes a 330-draw pass look like one fullscreen draw. Do-Not #29.
- **`screenshot` does not create its directory.** `texdump <path>/f-%d.png` with the directory absent arms, finds 46 colour targets, and fails every write after the frame is already dumped. `mkdir -p` in the runner first. Do-Not #28.

## What the evidence leaves

Two independent shaded paths are black in the same frame and they share their lighting inputs, not their geometry:

1. **The forward scene pass 549789** reads depth ser9, the sky/fog buffer ser44, the shadow atlas ser53, the environment cube ser12, the ambient pair ser14/ser15, and the material textures — and writes ser10 black. Its *own* second attachment (ser5, RG16Unorm, `f-500.png`) carries the car's geometry correctly, so the raster and the shader ran.
2. **The environment-cube path** (549712 → ser12) is black too, and its inputs are the cube G-buffer ser16/ser17/ser18 (normals, albedo, material) — ser16/ser17 are correct (`f-1600.png`, `f-1700.png`) and ser18 is black like ser8.

The cheapest read is the shading math itself: the translated Metal for every shader key is on disk at
`NFS_build/build/shader-cache/rt-<chain>/<key>_b.metal`, and the pass dump prints each draw's key (`fs=`) and its fragment constants (`B … cb reg0 … f=a,b,c,d`). Read the fragment shader of one lit draw in 549789 and the constants it consumes, and find which term is zero: the sun term (direction/colour in a constant buffer), the shadow factor (PCF over ser53), the ambient factor (ser14/ser15, already known black), or the material/AO channel (ser8/ser18, also black).

The two black channels — ser8 (material) and ser18 (material) — are the same attachment index in the G-buffer and in the cube G-buffer, and they are the only pair that is black in a buffer that is otherwise correct. Read the shading math for which of them multiplies the result before spending a boot on it.

## Code to touch

| Work | Touch |
| --- | --- |
| Shading math (read first) | `NFS_build/build/shader-cache/rt-<chain>/<key>_b.metal` for the `fs=` of a lit draw in 549789; `.res.json` beside it names each binding's handle. |
| The draw and its constants | `nvn_mtl.m` `encode_args` (constants at `PD("B %llu st%d cb reg%d …")`, line ~7190). |
| G-buffer material attachment | `nvn_mtl.m` — how the MRT write mask and the fragment outputs of the scene pass reach c2; ser8 is black for every surface. |
| Instrumentation already there | `DM_NVN_PASSDUMP` (per-draw keys, targets, samplers), `DM_NVN_TEXDUMP` (colour targets as PNG), `DM_NVN_TEXBIND` (dummy binds), `DM_NVN_REGLUM`, `DM_NVN_UB`/`TOTK_UBO_CAPTURE` (constant buffers). |
| Do not | Do not touch `games/totk` or `games/by3`; do not force a default texture, a constant light or a fixed albedo. |

## Read these first, and only these

- `.babysteps/scratch/S3.3b-texbind/` — `boot-race4.log` (frame 8916 passes), `texdump/` (the frame's targets), `run-race4.sh` (the ladder that reaches the race), `passes.py`/`chain.py`/`trace.py` (the readers written for them).
- `.operator/guides/running-the-nfs-boot.md` — repo build, work-tree symlinks, the cadence rule, the key table.
- `.operator/specs/nfs-port.md` — Claim boundaries.
- `docs/donot.md` #14, #19, #22, #27, #28, #29.

## Do-Not entries that apply here (6 of 29)

- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #27 — pass a Mac keycode below 128 to `key`, not a pad bit.
- #28 — create a diagnostic's output directory before the run, not after it fails.
- #29 — the per-draw line is lowercase `d`.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests, 1 skipped at the S3.3b close).
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured. This slice's subject is what he can see, so his run is the evidence, and he reaches the race faster than a scripted ladder.

## When this slice closes

Advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit (Do-Not #11); append any new fault after the last existing entry; rewrite this seed from what S3.3d leaves behind.
