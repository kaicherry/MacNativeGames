---
slice: S3.3j
stateFile: docs/STATE.md
donotCount: 41
seededFrom: the S3.3i close (ser10 under RGBA16Float is NaN at 84.6 % of the mask; the term is 1/(depth − m_41.w))
plannedTokens: 120000
---
# Next slice — S3.3j: make the term stop producing the NaN

## The next action

S3.3j — Stop `1/(depth − m_41.w)` from producing the NaN at the block pixels. S3.3i named the term: `_142 = 1.0/_140`
with `_140 = _55 − in.m_41.w` is zero there, and the NaN is born after the shader's `clamp` as the `0 × inf` at
`_184 = _180·_162`; the block pixels are 84.6 % NaN under `RGBA16Float` and were code 0 under the unorm store. Two
terms can be out of place and one boot separates them: **`_55` is a `sample()` through the depth binding's own
sampler**, and a filtered depth fetch returns a value interpolated across texels, which crosses a varying's value
along a curve — while a depth fetch at the fragment's own UV should read that texel; or **`in.m_41.w` is reconstructed
by the port** and carries a value the console's varying would not. Run the same two frames under
`DM_NVN_FLOAT_TARGET=10` with the depth binding's filter forced to nearest (the smallest change in `nvn_mtl.m`'s
sampler mapping), and read `analyse-i.py`'s NaN count against S3.3i's 8,361 (frame 8724, x 672–1215 y 504–911) and
6,026 (frame ~6612).

**Closes when:** "the same two frames hold no NaN in `ser10` at the block region and the presented frame's blocks are
gone with the rest of the frame unchanged against `g-b.png`; a guard that simply clamps `_140` removes the artifact
without naming which term was wrong, so it may ship only after the count shows the term."

**Ask the operator before starting a windowed boot.** `NFS_AUDIO=0` (Do-Not #38).

## What S3.3i settled — do not re-derive

- **The block pixels are a NaN, not a clamped negative.** `.babysteps/scratch/S3.3i/texdump-b/` (frame 8724,
  `boot-s33i.log`, `g-b.png`): mask 2,010 px (presented `f-300.png` ≤ 2 where the lit world `f-4400.png` > 60) is
  **1,700 NaN (84.6 %)**, 310 positive, **0 exact zero, 0 negative**; 2-px ring 2,153 NaN of 5,391; lit control 759
  of 888,886 (0.09 %). Frame-wide: **8,361 NaN texels in `ser10`**, bbox x 672–1215 y 504–911.
- **RGB only, and no infinity anywhere.** Alpha is `1.0` (`out.m_44.w = 1.0`); NaN is in R, G and B at the same
  pixels; **0 ±inf in any channel** of either frame.
- **The second frame repeats it**: 6,026 NaN at frame ~6612, x 672–1247 y 416–911 (`texdump-a/`).
- **The instrument**: `DM_NVN_FLOAT_TARGET=<serial>` is 11 gated lines in `nvn_mtl.m` (off by default). `tex_desc`
  swaps that serial's NVN `0x3D` to `RGBA16Float`; `attachment_format`, the sampling-view path in `tex_by_id` and the
  clear's view path take the base format, because RGB10A2 over RGBA16Float is not a Metal format view — drop those
  three and the attachment view is refused and the clear skipped. It logs `[float-target] ser=10 1920x1080 lv1
  format 0x3d (RGB10A2Unorm) stored as RGBA16Float`. Raw header becomes `1920 1080 115 15360` (8 bytes/texel).
- **The chain, from the translated MSL** (`…-c037c52f5abc_b.metal`): `_55 = totk_depth_fetch(fp_t_tcb_E.sample(…)).x`
  (line 67, reg 3 = ser9 depth), `_126 = in.m_41.w`, `_140 = _55 + (−_126)`, `_142 = 1.0/_140` (111), `_144 = _128·_142`,
  `_156 = 1.0/_144` (118), `_162 = sqrt(_158)`, `_184 = _180·_162` (139), `_228 = exp2(−|_184|)`, `_266 = _228·_246`,
  `_355 = fma(_341,_266,_250)`, `out.m_44.xyz` from `_411 = _355·_385`. A zero `_140` ⇒ `_142` inf ⇒ `_144` inf ⇒
  `_156` 0 and `_162` inf ⇒ **`_184 = 0 × inf` = NaN**, born after `_172 = clamp(_170,0,1)`.
- **`1/_379` is excluded as a lone cause**: a zero `_379 = fma(_355,_122,_124)` with nonzero `_355` stores an
  infinity, and no channel holds one. `_142`'s infinity is consumed before the store — the observed shape.
- **The equality is inferred, not read.** The NaN set is thin and depth-valued (8,271 distinct depth values in
  8,361 px), which a varying-vs-sampled-depth zero produces and a solid region does not; but `in.m_41.w` is a varying
  no dump carries, and a homography fit of the set against the depth texture is not float-exact (rms 9.1e-4).
- **The presented serial differs per run** — 1 in S3.3h, 3 in this dump, 2 in the earlier one. Name it per dump
  (Do-Not #41); `analyse-i.py` prints the 1920×1080 candidates.
- **Gate**: 264 tests, rc 0 at the S3.3i close.

## Withdrawn numbers — do not re-measure or re-report as new

- **"the zero at the block pixels is a negative clamped by the unorm store"** — withdrawn: the float target holds 0
  exact zeros and 0 negatives there.
- **"65.7 % / 64.7 % code 0 is the fragment's value"** — those are the unorm store of a NaN, not the arithmetic.
- **S3.3h's `0x7FC00000` withdrawal stands** (Do-Not #40): no NaN bit pattern is readable in `fmt=90`.
- **The 929-px mask** is frame-specific; the same rule reads 2,010 px here. The rule carries over, not the count.

## Code and data to touch

| Work | Touch |
| --- | --- |
| The depth binding's filter | the NVN→Metal sampler mapping in `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` — force nearest for the depth fetch, as the one-run test |
| The reciprocal | the translated MSL `_142` (line 111) — read it, do not hand-edit; a translator change only after the count names the term |
| The instrument | `DM_NVN_FLOAT_TARGET=10` + `DM_NVN_SNAPRAW=1` + `DM_NVN_TEXPNG_FS=418ba45ad47d16af` + `NFS_AUDIO=0` |
| The ruler | `.babysteps/scratch/S3.3i/analyse-i.py` (float raw; NaN / inf / exact zero / negative per channel), `run-s33i.sh` |
| Do not | do not touch `games/totk` or `games/by3`; do not read a NaN as ±inf; do not ship a `_140` clamp before the count names the term |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, rc 0 at the
  S3.3i close. Capture the real exit status, not a pipe's.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19). This slice changes source.
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3j/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (Do-Not #11), append any new fault after #41 in `docs/donot.md`, and rewrite this seed under 120 lines.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §The black squares are the scene target's own zero · §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §The translated shader for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · headless vs windowed · the reserved-region
  launch failure · the game slot · by cadence, not per-screen sleeps · key codes · evidence location

## Do-Not entries that apply (#, body not quoted)

#11 · #14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35 · #36 · #37 · #38 · #39 · #40 · #41
