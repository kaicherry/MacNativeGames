---
slice: S3.3k
stateFile: docs/STATE.md
donotCount: 43
seededFrom: the S3.3j close (the reciprocal is refuted; in.m_41.w is the vertex block's 1.00001, and both dump frames are race frames holding NaN)
plannedTokens: 110000
---
# Next slice — S3.3k: name the `0 × inf` that makes the NaN

## The next action

S3.3k — Name the `0 × inf` that actually makes the NaN. S3.3j excluded `_142 = 1/(_55 − in.m_41.w)`: the term it
subtracts is the *vertex* block's `_m0[3].x` = 1.00001, so the denominator bottoms out at 1e-5, never zero, and a
period-1.0 alternative is excluded by the NaN set's own depth (0 of 8,361 within 16 texels of the far plane). Two sites
remain, and one intermediate read off the draw separates them: **`_184 = _180·_162`**, `0 × inf` only where
`_144 = m_41.y·_142` is 0 **and** `_162 = sqrt(_148² + _144² + _146²)` is 0 — the varying's world position exactly
(0,0,0), a value the port interpolates; or **`_411 = _355·_385`** with `_385 = 1/_379` and `_379 = fma(_355,_122,_124)`
over the 1×1 `t22` pair, `0 × inf` when `_379 = 0` and `_355 = 0`. Do it on a race frame — the S3.3j recipe reaches
one by frame ~6,600, so arm the dump after a `shot` confirms the race: F6620 and F8548 hold 5,739 and 6,832 NaN, and
F8548's presented `f-100.png` carries the blocks. Write `_144`, `_162`, `_355` and `_379` into a spare serial for one
frame and read which is NaN or infinite at a NaN pixel. Judge against S3.3j's counts (5,739 · 6,832 · S3.3i's race
8,361 at F8724), and name the presented frame per dump (Do-Not #41).

**Closes when:** "one intermediate's per-pixel value at a NaN pixel names the site, and the presented frame's blocks are
gone with the rest of the frame unchanged against `g-b.png`; a clamp on any single term still ships only after a count
names it."

**Ask the operator before starting a windowed boot.** `NFS_AUDIO=0` (Do-Not #38). Screens are reached by taking a
`shot` and pressing on, never by a ladder copied from another run (Do-Not #35).

## What S3.3j settled — do not re-derive

- **The reciprocal is not the term.** `_140 = _55 − in.m_41.w` never reaches zero: `in.m_41.w` = **1.00001** and the
  depth is 0.9495–1.0, so `_140 ∈ [−0.0505, −1e-5]` and `_142` is finite. `_184 = _180·_162` is therefore not fed by an
  infinite `_142`.
- **`in.m_41.w` is the *vertex* stage's uniform.** VS: `out.m_29.w = vp_c3._m0[3].x`; `m_29` is the fragment's
  `in.m_41`. Dump `F8548_p525374_st0_cb0_705dbaa100.bin`, 224 B, `_m0[3] = (1.00001, −0.150002, 1, 0)`.
- **The fragment's cb reg 0 is a different block** — 96 B, `_m0[3] = (7.5, 4.21875, 0, 0)` = 1920/256 × 1080/256, the
  reprojection scale. Reading float 12 from it answers a question about the vertex block. Do-Not #42.
- **The filter branch is closed by the draw's own record**: `B … st1 samp reg3 sid=258 mag=0 min=2` over a one-level
  texture — nearest — so no filtered depth fetch exists. No source change was needed, and none was made.
- **The far plane is excluded by where the NaNs are**: at F8724, 0 of 8,361 NaN texels lie within 16 texels of the far
  plane, and the depth under them is 0.9626–0.9928 (`.babysteps/scratch/S3.3j/probe-j-shift.py`).
- **Both dump frames are race frames**, not menus: `f-4400.png` is the same road-and-hillside scene at F6620 and F8548,
  and F8548's presented `f-100.png` is the race with the car lit, the HUD and the blocks. `ser10` under the float target
  is 5,739 NaN at F6620 and 6,832 at F8548. The presented-black mask rule reads only 212 px on `f-100.png` (80 NaN) —
  these blocks are not near-zero in the presented image, so that 0.38 is not a refutation of anything.
- **The Select Car showroom carries the same blocks** (`g02.png`, zoomed `crop-roof.png` at 3×): along the windshield
  frame and roofline. It draws the car lit and textured; it is not a clean frame. The race's blocks are the same shape
  (`crop-race-car.png` at 3× on `f-100.png`: windshield frame, roll hoops, mirror mounts).
- **The run reached the race.** The copied ladder was still on SELECT RACE LOCATION (`g01.png`) and SELECT CAR (`g02.png`)
  when those screenshots were taken, and passed into the race before the first arming — Do-Not #43: read the scene from
  the dumped frame, not from a screenshot taken earlier in the same run.
- **The chain, from `…-c037c52f5abc_b.metal`**: `_55` depth sample (line 67) · `_140 = _55 + (−_126)` · `_142 = 1/_140`
  (111) · `_144 = _128·_142` · `_146`/`_148` · `_156 = 1/_144` (118) · `_162 = sqrt(_158)` (120) · `_170`–`_172` clamp
  · `_180 = _178·_156` (138) · `_184 = _180·_162` (139) · `_228 = exp2(−|_184|)` · `_355` (line 239) · `_379` (245) ·
  `_385 = 1/_379` (249) · `_411 = _355·_385` · `out.m_44.xyz` from `_411`/`_413`/`_419` + fma.
- **Gate**: 264 tests, rc 0 at the S3.3j close.

## Withdrawn numbers — do not re-measure or re-report as new

- **"`_140 = 0` at the blocks, and the NaN is `0 × inf` at `_184` fed by `_142`"** — withdrawn: the denominator ranges
  −0.0505 … −1e-5.
- **"`in.m_41.w` is a reconstructed varying"** — withdrawn: it is a uniform constant written by the vertex shader.
- **"7.5 is `in.m_41.w`"** — that is the *fragment* block's float 12 at the same register (Do-Not #42).
- **S3.3h's `0x7FC00000` withdrawal stands** (Do-Not #40); the 929-px and 2,010-px masks are frame-specific counts.
- The race's 8,361 (F8724) and ~6,026 (F6612) stand as counts, not as evidence for the reciprocal.

## Code and data to touch

| Work | Touch |
| --- | --- |
| The intermediate write | a gated temporary in the shader-cache *variant* path or a `DM_NVN_*` write in `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m`; the translated MSL is read, never hand-edited as the fix |
| The instrument | `DM_NVN_FLOAT_TARGET=10` + `DM_NVN_SNAPRAW=1` + `DM_NVN_TEXPNG_FS=418ba45ad47d16af` + `DM_NVN_CBDUMP=<dir>` + `NFS_AUDIO=0` |
| The ruler | `.babysteps/scratch/S3.3j/analyse-j.py` (NaN count, mask, presented-frame choice), `probe-j-shift.py` (distance to the far plane), `run-s33j.sh` |
| Do not | do not touch `games/totk` or `games/by3`; do not read a NaN as ±inf; do not ship a clamp before a count names the site; do not read a stage's uniform out of the other stage's block |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, rc 0. Capture
  the real exit status, not a pipe's.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19). This slice likely changes source.
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3k/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (Do-Not #11), append any new fault after #43 in `docs/donot.md`, and rewrite this seed under 120 lines.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §The black squares are the scene target's own zero (the S3.3j paragraph) · §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §The translated shader for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · headless vs windowed · by cadence, not per-screen
  sleeps · key codes · evidence location

## Do-Not entries that apply (#, body not quoted)

#11 · #14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35 · #36 · #37 · #38 · #39 · #40 · #41 · #42 · #43
