---
slice: S3.3h
stateFile: docs/STATE.md
donotCount: 39
seededFrom: the S3.3g close (the draw's five bindings are excluded; the zero is the draw's own arithmetic)
plannedTokens: 120000
---
# Next slice — S3.3h: read `ser10` as floats at the block pixels

## The next action

S3.3h — Read `ser10` as float32 at the block pixels and say whether the zero is a NaN or a stored zero. S3.3g
excluded all five sampled bindings of `418ba45ad47d16af` at the mask (A min 38, t10 mid-grey noise, t22 bright,
depth 0.983, and the pass's own scene target `ser10` exactly 0 at 64.7 % of them), so the zero is written by the
draw's own arithmetic or at its store. `DM_NVN_SNAPRAW=1` writes a non-depth target's raw bytes
(`nvn_mtl.m:11379`), so one windowed boot to the race gives `ser10` as float32 at that mask: a NaN pattern
(`0x7FC00000` and its signs) names the arithmetic — the only divisions are `1/(depth − m_41.w)` and the
neighbourhood `rsqrt(abs(2·t10 − 1))` — while a stored `0x00000000` names the store or the blend and excludes the
arithmetic. Arm the dump from the input file on the frame counter (Do-Not #35), and `DM_NVN_TEXPNG_FS` is not
needed for a target. `NFS_AUDIO=0` avoids the S3.3g deadlock (Do-Not #38) and is the only switch this measurement
may carry; if it is dropped, the `threads` dump is what names a freeze.

**Closes when:** "one race frame's `ser10` raw bytes at the block pixels are shown to be NaN or a stored zero, with
the producing term named or the arithmetic excluded."

**Ask the operator before starting a windowed boot.**

## What S3.3g settled — do not re-derive

- **The five bindings are named, from `<key>.json` `bindings[].reg`** (`nvn_refl_from_json` keeps that order 1:1),
  and that is the join that works. `.res.json`'s handles (14, 10, 16, 12, 34) are translation-time values that do
  not match the frame's texture ids (Do-Not #37). At frame 8723: reg 3 = ser9 1920×1080 Depth32Float (the depth
  fetch); reg 1 = ser44 1920×1080 (A); reg 4 = ser140 256×256 (t10, screen coords at `level(0.0)`); reg 2 = ser86
  64×1 (C, at `float2(x, 0.0)`); reg 13 = ser67 1×1 (t22, at `(0.5, 0.5)`).
- **All five are excluded at the block pixels** (`boot-s33g.log`, `texdump-d/`, `g-d.png`, ~F8700, mask = presented
  black where the lit world is bright, 510 px on the car's left silhouette): `ser10` exactly 0 at 64.7 %; A mean 116
  / min 38 / never 0; t10 mean 133 with 3.9 % of its texels near `2x−1 = 0` against 6.9 % over the whole texture;
  t22 (255,57,0)/(255,155,0); depth mean 0.983, 25.7 % exactly at the far plane. So the zero is **not a sampled
  texel**.
- **The dump only records sampled textures when `DM_NVN_TEXPNG_FS=<key>` is set at launch** (`tp_note_fs` returns
  early otherwise; Do-Not #39) — a dump without it holds the 46 colour targets and none of the bindings.
- **The guest deadlock is a virtual-core deadlock, not load** (Do-Not #38): `h=0x1019` in
  `_pthread_cond_wait ← audio_out_handler+0x638` owned core 2 while `h=0x1004` (affinity `mask=0x4`, core 2 alone)
  was runnable holding a lock `h=0x1003` waited for. `NFS_AUDIO=0` avoids it; the deadlock is unfixed and queued.
- **Arm on the frame counter, not wall clock.** `frame_now` must take the *maximum* `frame [0-9]+` in the log — a
  `tail -1` reader matched a stale `frame 300` line and armed at the notice screen.

## Withdrawn numbers — do not re-measure or re-report as new

- **"the ≈7.5 px cluster is a 256-wide buffer"** — t10 *is* 256×256 and its screen grid is 7.5×4.2 px, but its
  values at the mask carry no signal (3.9 % near `2x−1 = 0` against 6.9 % overall). The size match was a coincidence.
- **"one of the five bindings returns 0"** — false at the 510-pixel mask, all five measured.
- S3.3f's withdrawals stand (62 keys not 36; 17 of 62 discard; the 256×144 buffer unconfirmed).

## Code and data to touch

| Work | Touch |
| --- | --- |
| The target's raw bytes | `DM_NVN_SNAPRAW=1` in the dump loop (`nvn_mtl.m:11379`); no source edit |
| The mask | `.babysteps/scratch/S3.3g/` — `mask-d.npy`, `analyse2.py`, `FINDINGS.md` (the ruler) |
| The shader | `<work>/NFS_build/build/shader-cache/rt-totk-native-v4-unverified/418ba45ad47d16af…-c037c52f5abc_b.metal` (lines 60–101) and `<key>.json` `bindings[].reg` |
| The switches | `DM_NVN_SNAPRAW`, `NFS_AUDIO`, `passdump`/`texdump` input commands — environment only |
| Do not | do not touch `games/totk` or `games/by3`; do not rebuild to test a switch; do not name a cause without a capture |

## Gate, and this slice's close

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 264 tests, exit 0 at the
  S3.3g close. It must pass again.
- Build from the repo into the work tree (`rsync` with `build_app.py`'s excludes, then `rm -rf NFS_build/build/obj
  NFS_build/build/nfs-native && make runtime`), never a work-tree snapshot (Do-Not #19). S3.3g changed no source.
- A capture is evidence only with its file named. Evidence goes in `.babysteps/scratch/S3.3h/`.
- Close: advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit
  (Do-Not #11), append any new fault after #39 in `docs/donot.md`, and rewrite this seed under 120 lines.

## Read these first, and only these

- `.operator/specs/nfs-render.md` — §The black squares are the scene target's own zero · §Claim boundary
- `.operator/guides/nfs-frame-dumps.md` — §Arming them · §The dump's records · §The translated shader for a key
- `.operator/guides/running-the-nfs-boot.md` — build from the repo · headless vs windowed · the reserved-region
  launch failure · the game slot · by cadence, not per-screen sleeps · a controller is OR-ed in · key codes ·
  evidence location

## Do-Not entries that apply (#, body not quoted)

#11 · #14 · #19 · #22 · #27 · #28 · #29 · #30 · #33 · #35 · #36 · #37 · #38 · #39
