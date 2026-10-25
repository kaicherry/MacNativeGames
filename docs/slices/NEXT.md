---
slice: S3.3g
stateFile: docs/STATE.md
donotCount: 36
seededFrom: the S3.3f close (the draw is named; which binding of it returns 0 is not)
plannedTokens: 120000
---
# Next slice — S3.3g: name the binding through which the fullscreen resolve writes the zero

## The next action

S3.3g — Name the binding through which `418ba45ad47d16af` writes the zero. The draw is the single fullscreen 4-vertex
resolve of forward pass 543889, and the blocks vanish when it is highlighted
(`.babysteps/scratch/S3.3f/h2-03.png` against `h03.png`); its translated fragment shader
(`NFS_build/build/shader-cache/rt-totk-native-v4-unverified/418ba45ad47d16af…-c037c52f5abc_b.metal`, lines 60–101)
fetches depth, rebuilds a world position, and samples `fp_t_tcb_A` twice, `fp_t_tcb_10` at `level(0.0)`,
`fp_t_tcb_C` at `float2(x,0.0)` and `fp_t_tcb_22` at `(0.5,0.5)`. `.res.json` gives the bindings (handles 14, 10, C, 22
and the depth texture); `DM_NVN_TEXBIND=1` maps a handle to a serial in a run that reaches the race, and `DM_NVN_TEXDUMP`
gives that serial's pixels — so the question is answered by the target that reads 0 along the car's silhouette, not by
another key sweep. `DM_NVN_SKIP_FS=418ba45ad47d16af` removes the blocks and the world together, so it names the draw and is
not a fix.

**Closes when:** "one named target is shown returning 0 at the block pixels, or one run with one switch removes the
blocks and keeps the world."

**Ask the operator before starting a windowed boot** — the app takes the screen, and a run he did not agree to is a run
he stops.

## What S3.3f settled — do not re-derive

- **The draw is `418ba45ad47d16af`.** One draw, `count=4`, viewport and scissor `0,0,1920,1080`, `blend_en=0`, `cull=0`,
  `front=0(ccw=1)`, `inst=1`, `idx=none` (`boot-s33f-hilite2.log`, `HILITE draw` lines). Highlighted, it floods the
  scene with its own colour (HILITE colour 2, yellow) and the blocks are **gone from the same crop**; the car, its soft
  shadow and the HUD are drawn after it and survive.
- **The four heaviest discarding decal keys do not write them.** `f65d49d4`, `2b6329f2`, `0afe74c5`, `800ee37f` (ten small
  primitives each, blending on, pinned-LOD sample, alpha-test discard) highlighted: the blocks stay **black and
  uncoloured** beside the car (`h03.png`, crop `blocks-h03.png`).
- **The pass has 62 fragment keys, not 36.** A reader that takes `blend=` as decimal drops the 172 draws whose blend is
  hex `0xff`. **17 of the 62 keys contain `discard_fragment`** (88 draws) — so a discard was never excluded, only
  unexamined. No key writes a literal zero, so the zero is a **sampled** term (unchanged from S3.3e).
- **The blocks are a quantised shadow-like term, not a hole.** They sit in the car's own shadow and stack along its left
  silhouette; the shadow's soft grey parts are correct and drawn after the resolve.
- The two probes are env-only: `TOTK_GPU_HILITE1=<≤4 keys>` + `TOTK_GPU_HILITE_NOBLEND=1` (colour to attachment 0,
  key order green/cyan/yellow/blue), `DM_NVN_SKIP_FS=<≤16 keys>`. No rebuild to test a switch.

## Withdrawn numbers — do not re-measure or re-report as new

- **"36 fragment keys"** — a parse artifact (Do-Not #36). The pass has 62, and every count derived from that list
  (draws per key group) was computed over the opaque half only.
- **"None of the pass's fragment keys contains `discard_fragment`"** — false as a claim about the pass; 17 of 62 do.
- **"≈7.5 px / a 256×144 buffer"** — the size cluster stands, but no 256×144, 512×288 or 128×72 target exists in
  `S3.3d-constants/texdump-race6/`, so the buffer identity is unconfirmed and was not claimed.
- The rest of S3.3e's exclusions stand (post chain excluded; ser 10 carries 41.2 % of the block mask, ser 44 0.0 %).

## Code and data to touch

| Work | Touch |
| --- | --- |
| The draw's shader | `<work>/NFS_build/build/shader-cache/rt-totk-native-v4-unverified/418ba45ad47d16af…-c037c52f5abc_b.metal` and `.res.json` |
| Judging a run | `games/nfs/scripts/frame_attribution.py` (`chain`, `rank`) — no edit expected; it is the ruler |
| Grouping a pass | `.babysteps/scratch/S3.3f/group_pass.py` (draws+keys, hex-safe) and `shader_scan.py` (discard/LOD/zero) |
| The switches | `TOTK_GPU_HILITE1`, `TOTK_GPU_HILITE_NOBLEND`, `DM_NVN_SKIP_FS` in `nvn_mtl.m` — environment only |
| Do not | do not touch `games/totk` or `games/by3`; do not rebuild to test a switch; do not name a cause without a capture |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, 1 skipped, exit 0
  at the S3.3f close. It must pass again.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19). S3.3f changed no source, so the
  work tree's binary is still the repo's.
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3g/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit (Do-Not
  #11), append any new fault after #36 in `docs/donot.md`, and rewrite this seed under 120 lines. S4.2 (the validation
  matrix) stays queued behind the artefact.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §The black squares are the scene target's own zero · §Ruled out by measurement,
  earlier · §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §PNG filenames · §The translated shader
  for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · the two things the work tree needs · headless vs
  windowed · the reserved-region launch failure · the game slot · by cadence, not per-screen sleeps · a controller is
  OR-ed in · key codes · evidence location

## Do-Not entries that apply (#, body not quoted)

#11 · #14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35 · #36
