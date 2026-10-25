---
slice: S3.3f
stateFile: docs/STATE.md
donotCount: 35
seededFrom: the S3.3e close (the pass and the target are named; the draw, the texture and a removing switch are not)
plannedTokens: 120000
---
# Next slice — S3.3f: bisect the forward pass to the draw that writes the squares

## The next action

S3.3f — Bisect the forward scene pass to the draw that writes the squares. The pass is `543889` in frame 8840 of
`.babysteps/scratch/S3.3d-constants/boot-race6.log`: 330 draws, 36 distinct fragment keys, writing ser 10 and ser 5,
sampling ser 44 and the shadow atlas ser 53. `games/nfs/scripts/frame_attribution.py chain <boot.log>` prints that pass
list and `rank <capture.png> <texdump-dir>` names which dumped target first carries the blocks, so a run is judged by
one command. Take the keys from the pass, group them, and use the two environment switches that need no rebuild:
`TOTK_GPU_HILITE1=<key,...>` (up to four keys, solid colour to attachment 0 — the block region turning a colour names
the draw) and `DM_NVN_SKIP_FS=<key,...>` (drops those draws; the blocks vanishing names the class). Both are launch
environment, one run per group, and each run needs a capture that shows the car at the same screen position — verify
with a `shot` before arming a dump (Do-Not #35). The two candidates the measurement still admits are a draw that
re-shades ground/geometry with a failed shadow term and a draw that samples a mask at a coarse level; a draw whose
sample count is small while it covers the block region separates them, and `TOTK_ANOMALY=<dir>` + the `anomdump` input
command gives per-draw GPU sample counts for the last frames at one request.

**Closes when:** "one run with one switch removes the squares and the capture that shows it is named, or the draw is
named and its shader shows the zero."

**Ask the operator before starting a windowed boot.** S3.3e's third ladder was force-quit mid-run; the app takes the
screen, and a run he did not agree to is a run he stops.

## What S3.3e settled — do not re-derive

- **The squares are the scene target's own zero, not a dark shading value.** In `60-race-fixed.png` 3,099 pixels are
  exactly (0,0,0); 2,843 of them ring the car, and the majority of pixels inside each block are exactly 0 while the
  block edges are 1–30. The same pixels read 53–78 in the lit-world target before the pass that writes the scene.
- **The target is ser 10, the scene colour, written by pass 543889** — 330 draws, cleared to `0,0,0,1`, sampling
  ser 44 and the shadow atlas ser 53, and also writing ser 5 (fmt 60). Measured with
  `frame_attribution.py rank` over `.babysteps/scratch/S3.3d-constants/texdump-race6/` (frame 8637, the same frame as
  the capture) and the records of frame 8840:

  | serial | share of the block mask that is exactly zero |
  | --- | --- |
  | 44 lit world | **0.0 %** (clean) |
  | **10 scene colour** | **41.2 %** (carries them) |
  | 50 composite | 41.2 % |
  | 76 composite | 28.3 % |
  | 1 presented | 13.5 % |

- **The post chain is excluded, not suspected.** FXAA, the tone map, bloom, the 960×540 → 480×270 → 120×67 → 30×16
  chain and the present all sit downstream of ser 10 and inherit what it has; ser 44 is clean at those pixels.
- **No shader in the pass discards and none writes a literal zero.** All 36 fragment keys were checked in the work
  tree's shader cache: zero `discard_fragment`, no constant-0 colour output. The zero is therefore a **sampled value**.
- **Not tile-shaped, not screen-grid-shaped.** 10 of 93 touched 8 px tiles are fully black and none at 16 or 32 px, so
  it is not a DontCare/tile store. Block *sizes* cluster at ≈7.5 px and its multiples (a buffer about 256×144 over
  1920×1080) while block *positions* sit on no grid — a projected texel grid looks like this, a screen-space mask
  does not.
- **The frame's pass chain**, for orientation: `543887` 502 draws → G-buffer ser 6/7/8 (the car is not in it; ser 6
  is the road's normals) · `543888` 3 draws → ser 44 (samples the G-buffer, depth ser 9, ser 53) · `543889` → ser 10
  · `543899`/`543900`/`543901` composites → bloom chain → `543917` final.

## Withdrawn numbers — do not re-measure or re-report as new

- **"8–10 px in a 1920×1080 capture of a 1600×900 render"** is wrong about the render: the scene target ser 10 is
  1920×1080 in the dump. Nothing renders at 1600×900 in the frames measured.
- **"Against the sky"** was not reproduced: in the frames measured the staircase sits on road and ground beside the
  car. Treat the sky part of the report as the operator's sighting, not as a measured fact.
- Do-Not #14's batch — 7 stops, frames 1375–1803 by 65 s, the ~20 % frame cost — belongs to a run with the GPU
  contended. Do not re-report it.
- S3.3d's ser12/ser14/ser15 0 → (78.2, 90.2, 100.1) and the winding-flip result stand; do not re-measure them.

## Code and data to touch

| Work | Touch |
| --- | --- |
| Judging a run | `games/nfs/scripts/frame_attribution.py` (`chain`, `rank`) — no edit expected; it is the ruler |
| The pass's keys and draws | `.babysteps/scratch/S3.3d-constants/boot-race6.log` (frame 8840), read with `chain` |
| The two switches | `TOTK_GPU_HILITE1` (4 keys, colour to c0) and `DM_NVN_SKIP_FS` (16 keys) in `nvn_mtl.m` — environment only |
| If the draw's shader holds the zero | the translated MSL in `<work>/NFS_build/build/shader-cache/rt-<chain>/<fs>.b.metal`, and `.res.json` for which handle each binding holds |
| Do not | do not touch `games/totk` or `games/by3`; do not rebuild to test a switch; do not name a cause without a capture |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, 1 skipped, exit 0
  at the S3.3e close. It must pass again.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19).
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3f/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (Do-Not #11), append any new fault after #35 in `docs/donot.md`, and rewrite this seed under 120 lines. S4.2 (the
  validation matrix) is queued behind the artefact.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §What is measured · §Reading an empty target · §Ruled out by measurement, earlier ·
  §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §PNG filenames · §The translated shader
  for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · the two things the work tree needs · headless vs
  windowed · the reserved-region launch failure · the game slot · by cadence, not per-screen sleeps · a controller is
  OR-ed in · key codes · evidence location

## Do-Not entries that apply (#, body not quoted)

#14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35
