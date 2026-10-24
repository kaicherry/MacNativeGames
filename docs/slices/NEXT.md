---
slice: S2.24
stateFile: docs/STATE.md
donotCount: 23
seededFrom: 42b72ca plus the S3.1b pctl fix (the commit that closes S3.1b)
plannedTokens: 150000
---
# Next slice — S2.24: enable the mid-pass depth-stencil clear that stops the race-course reveal

## The next action — verbatim

S2.24 — Enable the mid-pass depth-stencil clear that stops the race-course reveal. The S3.1b windowed run
(`.babysteps/scratch/S3.1b-pctl/windowed-fixed2.log`) reached the first Career Mode race — the operator
reports the picture was revealing the race course — and then aborted in the GPU submit path:
`[dm] [nvn-mtl] FATAL mid-pass / masked / scissored depth-stencil clear is isolated pending its standalone
probe (set TOTK_GPU_MIDCLEAR=1 to run it)`, stack `attachment_format` ← `mtl_exec` ← `submit_exec` ←
`h_nvnQueueSubmitCommands`. The path is `clear_ds_quad`
(`games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m:9082`) → `clear_ds_core` (:9046), gated by
`gpu_path_on("TOTK_GPU_MIDCLEAR", …)` (:4053), which `MDIE`s unless the switch is `1`; the isolation was
imposed after a macOS IGPUFamily kernel panic in full-game tests (:4048). Run the standalone probe first —
`./build/nfs-native rewritten/nfs100 probe-gpu-ds-clear` (`mtl_probe_ds_clear`, :12895, which sets
`TOTK_GPU_MIDCLEAR=1` itself and replays clears across active and restarted passes) — and only then a
bounded windowed boot with the switch on. S3.1c (`savedata`) is deferred, not dropped: the guest reached
the race with no `savedata` request, so it is not on the path to the first frame. The standalone probe
already passes: `probe-gpu-ds-clear` returned `checked` 6144 with `status: pass`
(`.babysteps/scratch/S2.24-midclear/probe-ds-clear.log`), so the next step is the bounded windowed boot
with the switch on.

S2.24 closes when `probe-gpu-ds-clear` passes with its `checked` count reported, and a bounded windowed
boot with `TOTK_GPU_MIDCLEAR=1` gets through the race-course reveal — no `FATAL mid-pass / masked /
scissored depth-stencil clear` in the log, and the operator reports the reveal plays. A path that panics
the host is a stop, not a result.

## What the next session does first

1. Read `nvn_mtl.m` 9046–9090 (`clear_ds_core`/`clear_ds_quad`), 4053 (`gpu_path_on`) and 12895–12930
   (`mtl_probe_ds_clear`). The probe already asserts what the path must preserve: colours, depth outside
   the scissor, stencil bits outside the mask, and depth during a stencil-only clear.
2. Build from the repo (Do-Not #19): `rsync -a` `games/nfs` with `build_app.py`'s excludes,
   `rm -rf build/obj build/nfs-native`, `make WORK=. runtime`; a clean `-Werror` build is evidence.
3. The probe is already green: `probe-gpu-ds-clear` → `{"status":"pass","checked":6144}` (32×32×6) at
   `.babysteps/scratch/S2.24-midclear/probe-ds-clear.log`. Re-run it only if the path or probe changes.
4. Only then a bounded windowed boot with `TOTK_GPU_MIDCLEAR=1` (windowed is where the race is reached;
   Do-Not #22). If the host panics, that is the isolation's reason confirmed — record it and stop.
5. Decide how the path becomes default only after the probe and the boot both pass.

## What is settled — do not re-derive

- **The `pctl` reply shape is fixed and closed.** `CreateServiceWithoutInitialize` (`cmd=1`) returns the
  service object through `rs->move[]`; the SDK then calls `Initialize` (`cmd=1` on the object) and
  `TryBeginFreeCommunication` (`cmd=0x3e9` = 1001). No `SIGSEGV` at `sdk+0x3489dc`.
- **The boot reaches the first Career Mode race** and stops in the race-course reveal on the isolated
  depth-stencil clear. No `savedata`, `movie` or video service is opened on the way.
- **The save directory mounts without a `savedata` service** (`host save storage` / `fs mount save`), so
  S3.1c is not on the path to the first frame.
- **`gpu_path_on` is the isolation mechanism**, shared with `TOTK_GPU_WIDEN`, `TOTK_GPU_BLOCKRT` and
  `TOTK_GPU_HILITE1`; it aborts with the message rather than running the path (nvn_mtl.m:4048–4058).
- **The depth-stencil clear's own probe passes**: `probe-gpu-ds-clear` → `checked` 6144, `status: pass`
  (scissored clears, partial stencil masks, preserved colours and untouched pixels across passes).
- **The gate** passed 255 tests, 1 skipped, exit 0 on 2026-10-23, and does not cover runtime C.

## Withdrawn — proven wrong, do not use

- **"The race stop is the intro video decoder."** Withdrawn: no `movie`/video service opens; the stop is
  `attachment_format` ← `mtl_exec` ← `submit_exec` ← `h_nvnQueueSubmitCommands`. The reveal is rendered.
- **"`savedata` is the next blocker."** Withdrawn: the boot reached the race with zero `savedata` requests.
- **"The `cmd=1` reply shape is unestablished."** Settled by the SDK proxies and the windowed run.

## Code to touch

| Work | Touch |
| --- | --- |
| The gated path | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` — `clear_ds_quad` (:9082), `clear_ds_core` (:9046), `gpu_path_on` (:4053). |
| The probe | `mtl_probe_ds_clear` (:12895), dispatched as `probe-gpu-ds-clear` by `games/nfs/runtime/gpu_path_probe.m:10` and `loader.c:499`. |
| Evidence | `.babysteps/scratch/S2.24-midclear/`: the probe output, the windowed log, the `checked` count. |
| Do not | Do not touch `games/totk` or `games/by3`; do not enable the path before the probe passes; do not read a rate or frame count (Do-Not #14); do not call a panic a result. |

## Read these first, and only these

- `.operator/specs/nfs-port.md` § "Claim boundaries" (what a visual claim may rest on).
- `docs/STATE.md` § "S3.1b (second limb)" and "Next action".
- `docs/donot.md` #8, #11, #14, #17, #19, #22, #23.

## Do-Not entries that apply here (7 of 23)

- #8 — do not name a stall by the frame one run happened to stop at.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU.
- #17 — do not name main's own call by the SDK function an unwind frame reaches.
- #19 — do not treat a work tree as the repo; re-sync and force the rebuild.
- #22 — do not read a scripted schedule's failure to reproduce a live session as a fact about the buttons.
- #23 — do not answer a command that returns an object with a result-only reply.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (255
  tests, 1 skipped); the suite covers the test tree, not runtime C.
- **Budget:** planned at 150k tokens; the cap is 160k per unit, 170k only when tight.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19). The review point (past
  the licence/attract screens, in the game proper) has been reached; this unit is the next step past it.

## When this slice closes

Write the next seed from what S2.24 leaves behind (the next stop after the reveal, or S3.1c if the race
then asks for saves), re-verify the Do-Not count (23 or higher), and commit the state and the seed.
