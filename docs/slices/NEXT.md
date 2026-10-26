---
slice: S3.3i
stateFile: docs/STATE.md
donotCount: 40
seededFrom: the S3.3h close (ser10 is RGB10A2Unorm; the block pixels are a stored zero and the clear is excluded)
plannedTokens: 120000
---
# Next slice — S3.3i: name the term that makes the draw's output zero

## The next action

S3.3i — Name the term that makes `418ba45ad47d16af`'s fragment output ≤ 0 at the block pixels. S3.3h proved the
target cannot hold a NaN: the forward pass's `c0` is `ser10` `fmt=90`, `MTLPixelFormatRGB10A2Unorm` (NVN `0x3D`,
`nvn_fmt_mtl.c:65`), so a NaN the draw computes is converted to 0 at the store. At a 929-pixel mask on a race frame
(texdump frame 8751) 65.7 % of the pixels are exactly code 0 in all three channels, hard-edged, and the pass clear is
excluded by coverage, so the zero is the draw's own output. What the target cannot separate is "the arithmetic
produced a negative that the unorm store clamped" from "a NaN reached the output". One gated dev switch that creates
this serial's NVN `0x3D` as `MTLPixelFormatRGBA16Float` for a single run makes the raw bytes separate them: the
shader's `_172 = fast::clamp(_170, 0, 1)` then `_174 = log2(_172)` turns a negative `_170` into an exact 0
(`log2(0) = -inf`, `exp2(-inf·c) = 0`), while a NaN arising **after** that clamp — `1/(depth − m_41.w)` (`_142`) or a
weight normalisation `1/_379` — survives to the output and stores as a NaN half. Read the same mask from the same
frame. The reading is positive-only: a NaN that enters `clamp` may flatten to 0 in either target, so an exact zero
does not exclude a NaN upstream of `_172`.

**Closes when:** "one race frame's `ser10` at the block pixels is shown under a float target to be a NaN or an exact
zero, with the producing term named."

**Ask the operator before starting a windowed boot.**

## What S3.3h settled — do not re-derive

- **`ser10` is `RGB10A2Unorm`** (`fmt=90` in both the pass's `P … c0` record and the texdump's `format`), from NVN
  `0x3D` → `nvn_fmt_mtl.c:65`. 10-bit unorm has no NaN encoding; the port's mapping is faithful to the game's request.
- **The raw is packed, not float32.** `DM_NVN_SNAPRAW=1` writes `<png>.raw` — header `width height format
  bytesPerRow`, then 4-byte packed 10:10:10:2 (R in the LSBs). The unpack is validated: `round(code/1023·255)`
  matches the dump's own PNG at all 2,073,600 texels, max difference 0.
- **The mask pixels are exactly 0** (`.babysteps/scratch/S3.3h/texdump-b/`, `analyse-h.py`): 929 px (presented ≤ 2
  where the lit world > 60), all three channels code 0 at 65.7 % (610 px), codes 1–7 at 82 px more; lit control of
  783,767 px at mean 619/1023 and 0.06 % zero. 86 components, largest 33 px; only 1.0 % of zero pixels border a code
  1–7 one — hard edges, so the zero is written, not an 8-bit PNG rounding.
- **The clear is excluded by coverage:** pass 548581, `vc=4 prim=5`, `vp` and scissor `0,0,1920,1080`, `dt=0 dw=0
  cull=0 blend=0`, and the key has no `discard_fragment`; it writes `out.m_44 = (x,y,z,1)` unconditionally.
- **The frame is reachable without the deadlock:** `NFS_AUDIO=0`, no guest-placement retry, race up by ~F6600, dump
  armed at F6500/F8500 and landed at 6695/8751. Reuse `.babysteps/scratch/S3.3h/run-s33h.sh`.
- A mask count near 510 (S3.3g) is not reproducible frame to frame — the car sits differently; the rule is what
  carries over, not the count.

## Withdrawn numbers — do not re-measure or re-report as new

- **"a NaN or ±inf reaches an RGBA16F target"** — withdrawn. `ser10` is 10-bit unorm; a NaN cannot be stored in it.
- **"`0x7FC00000` in `ser10`"** — unreachable in that format; the S3.3h brief was wrong on the instrument (Do-Not #40).
- **"the zero is at 64.7 %"** — frame-specific. S3.3h's own frame reads 65.7 % of a 929-px mask.
- S3.3g's withdrawals stand (the ≈7.5 px cluster as a 256-wide buffer; "one of the five bindings returns 0").

## Code and data to touch

| Work | Touch |
| --- | --- |
| The float target | `games/nfs/third_party/d2r_gfx/gfx/tex/nvn_fmt_mtl.c:65` and the creation path (`tex_desc`, `attachment_format` in `nvn_mtl.m`) — one env-gated switch, one serial, no other format changed |
| The raw bytes | `DM_NVN_SNAPRAW=1` (`nvn_mtl.m:11379`), no edit |
| The mask and ruler | `.babysteps/scratch/S3.3h/` — `texdump-b/`, `analyse-h.py`, `analyse-h-shape.py`, `run-s33h.sh` |
| The shader | `<work>/NFS_build/build/shader-cache/rt-totk-native-v4-unverified/418ba45ad…-c037c52f5abc_b.metal` (lines 110–200) |
| Do not | do not touch `games/totk` or `games/by3`; do not change the target's format for a second serial; do not name a cause without a capture |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, exit 0 at the
  S3.3h close. It must pass again.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19). S3.3i *does* change source,
  so the rebuild is the point.
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3i/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (Do-Not #11), append any new fault after #40 in `docs/donot.md`, and rewrite this seed under 120 lines.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §The black squares are the scene target's own zero · §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §The translated shader for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · headless vs windowed · the reserved-region
  launch failure · the game slot · by cadence, not per-screen sleeps · a controller is OR-ed in · key codes ·
  evidence location

## Do-Not entries that apply (#, body not quoted)

#11 · #14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35 · #36 · #37 · #38 · #39 · #40
