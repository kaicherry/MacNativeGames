---
slice: S3.3c
stateFile: docs/STATE.md
donotCount: 30
seededFrom: the S3.3d close (the race is lit; no run since S3.1d has exercised a save write)
plannedTokens: 150000
---
# Next slice — S3.3c: save/load across a race

## The next action

S3.3c — Save/load across a race, and verify the persisted state survives a restart. S3.1d established only the
**read** path: a repo-built boot reads `save:/MUD64.8`, the notice is passed, and the main menu is reached. Nothing
yet shows the game **writing** the save directory, nothing shows those bytes being committed, and nothing shows
the same bytes coming back into a second boot.

S3.3c closes when a run writes a save from inside the game — a race that ends, an event that completes, or a
setting changed — and the file under the host save directory is shown to change, to be readable by a second
repo-built boot, and to be unchanged by a third boot that only reads it. Name the file, its size and its hash at
each step. Or: name, with the fs call and the branch that drops it, the write the game issues and does not get.

## What S3.3d settled — do not re-derive

- **The race's real-time 3D is lit and textured.** `.babysteps/scratch/S3.3d-constants/60-race-fixed.png` from
  `boot-race6.log`, drawn by a repo-built binary with no diagnostic environment: textured asphalt and markings,
  rock, shrubs, sky, the car's decals and its `ND4SPD` plate, and the shadow under the car.
- **The zero term was the NVN→Metal front-face mapping, not the shading.** NVN names the front face with y up and
  Metal evaluates winding with y down, so the literal mapping put the visible surface on the back for every
  back-face-culled draw. `TOTK_GPU_WINDING_FLIP=1` now restores the old (wrong) mapping as a one-run diagnostic.
  Do-Not #30.
- **An empty shaded target is not a zero until its draws are shown to rasterize.** Read `cull=` on the draw and
  `front=` on the frame before reading the shader.
- **The probe chain is alive.** ser12 (256×256×6, all mips), ser14 and ser15 went from exactly 0 to
  (78.2, 90.2, 100.1), (78.9, 91.2, 100.8) and (77.9, 90.0, 99.6) with the fix; ser10 (112.4, 111.5, 99.5) and
  ser44 (65.0, 69.2, 69.5) moved with them.
- **The probe's stencil gate is real state and not a fault.** Its pass attaches a `Depth32Float_Stencil8` plane,
  clears it to 0 and its deferred shade tests `LESS` against reference `0x80`; with the winding fixed that same
  gated quad draws. `DM_NVN_DBG_SER=<serial>` prints a colour target's load/store actions and its depth and
  stencil planes once per pass — the P record carries none of that.

## What the save path looks like today

- The fs host mounts the save directory at boot and logs the host path: `host save storage:
  user-data/save/db047cf775247afa9b3bd6d11bcf9740`, then `fs mount save: -> …`. The mount is the `nn::fs`
  `save:` family; the `savedata` / `ns:save_data` IPC services exist (`S3.1c`) but `main` imports no `nn::ns`,
  so they are not the path this game saves through (S3.1d).
- `runtime_fs_save_busy()` already reports "a save file is open for writing" and the fs host has unit checks for
  create/open/read around it (`games/nfs/runtime/fs_host.c:353-363`), so the bookkeeping exists; what is unproven
  is that the game's own writes reach disk and commit.
- `autoload.c` parses `save:/slot_NN/<name>`, so more than one file name is expected in that directory.

## Questions this slice answers, in order

1. Does any run of the game **open a file under `save:` for writing**? `[totk-fs]` lines plus the file's mtime
   answer this without a new instrument; a run that ends a race should produce it.
2. If a write happens, does the file change on the host, and does a second boot read the changed bytes rather
   than the packaged image? Hash the file before, after and after the second boot.
3. If no write happens, which fs call does the game issue last before it would have written, and what does the
   host return for it? `TOTK_CMD_LOG` already logs the fs command path.
4. Does a read-only third boot leave the file byte-identical? A save path that rewrites on load is a defect even
   when the bytes are right.

## Code and data to touch

| Work | Touch |
| --- | --- |
| The save mount and its commit semantics | `games/nfs/runtime/fs_host.c` (`save:` mount, `runtime_fs_save_busy`, create/open/commit) |
| Slot naming | `games/nfs/runtime/autoload.c` (`save:/slot_NN/<name>`) |
| The bytes on disk | the work tree's `user-data/save/<hash>/` — hash and size are the evidence; do not hand-edit it between runs |
| The run itself | `.babysteps/scratch/S3.3d-constants/run-fixed.sh` (the ladder that reaches the race; `<screenshot>` and `texdump <path>/f-%d.png` are input commands, not env vars) |
| Do not | Do not fabricate a save file, do not mark the round-trip passed from a log line alone, and do not touch `games/totk` or `games/by3`. |

## Read these first, and only these

- `.operator/guides/running-the-nfs-boot.md` — repo build, the work tree, the input cadence, the key table.
- `docs/STATE.md` § Completed, S3.1d and S3.3d paragraphs — the read path and the render fix as measured.
- `.operator/specs/nfs-port.md` — the `nn::fs` save path and its claim boundaries.
- `docs/donot.md` #11, #14, #19, #22, #27, #30.

## Do-Not entries that apply here (6 of 30)

- #11 — advance all five `docs/STATE.md` sections in the closing commit.
- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync the work tree and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #27 — pass a Mac keycode below 128 to `key`, not a pad bit.
- #30 — an empty target is not a zero until its draws are shown to rasterize.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (261 tests,
  1 skipped at the S3.3d close).
- **Offer the operator a run** after any change he can see (the lit race is already offered and open), built from
  the repo (Do-Not #19), with the log and the save-file hashes named.

## When this slice closes

Advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit (Do-Not
#11); append any new fault after the last entry in `docs/donot.md`; rewrite this seed from what S3.3c leaves
behind, under 120 lines.
