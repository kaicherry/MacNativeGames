---
slice: S3.3b
stateFile: docs/STATE.md
donotCount: 27
seededFrom: the S3.3a close (a race runs and is drivable; real-time 3D samples no texture)
plannedTokens: 160000
---
# Next slice — S3.3b: make real-time 3D sample its textures

## The next action — verbatim

S3.3b — Make real-time 3D sample its textures: find why a drawn triangle carries the right shape, the right alpha and the right lighting gradient but none of its texture, and fix it. The race and the car showroom are both affected; the menu backdrops, the HUD and the mirrors are not. Do not touch `games/totk` or `games/by3`.

S3.3b closes when a repo-built windowed run draws one textured surface in the race — a capture and its log named — or the stop it waits on is named with the stage (upload, handle resolution, sampler, or lighting pass) and the evidence that places it there.

## What S3.3a settled — do not re-derive

- **The race is reached and is drivable.** `.babysteps/scratch/S3.3a-controllable/12-race-a.png` is the race proper: timer 0:43, 6th, 7.5 miles, speedo 108. The operator's `boot-operator.log` runs the race to **5100 frames / 174 s**; he drove it. The menu → briefing → race load takes about 40 s after `Ⓐ CONTINUE` (the `VEHICLES/VEH_*` and `SEACREST/TRK_UNIT*` reads run t≈244→270).
- **Real-time 3D is untextured, and it is not only the car.** Magnified crops of a race frame — `zoom-road.png`, `zoom-car.png`, `zoom-hill.png` — are smooth shading gradients with no grain, no markings and no decal at any scale. The car is a cream-to-black gradient, not a painted body.
- **The showroom is unlit** (operator, 2026-10-24): SELECT COLOR is "really dark, like the lights aren't on". So the real-time path loses texture *and* light together.
- **Everything that looks right is not real-time 3D.** The briefing backdrop (`in-race.png`) and the display-settings backdrop (`62-race-a.png`) are pre-rendered images; the HUD and mirror are 2D. Their being correct is not evidence that the 3D path works.
- **Textures upload with real data.** In the operator's race log the BC1_sRGB histogram is `0x46:284 tex/28 MB(zero 4)` — 280 of 284 uploads carried non-zero source bytes. The `(zero N)` suffix is the per-format count of all-zero sources (added in S3.3a, `nvn_mtl.m` fmtstats block).
- **Every format maps.** No `RED_ONCE(0, "texture format … has no Metal equivalent")` and no `NO METAL EQUIV` in any run. The race's formats are `0x27` RGBA16Float, `0x42` BC1, `0x44` BC3, `0x46` BC1_sRGB, `0x48` BC3_sRGB, `0x9a` BGRA8Unorm. Format handling is not the defect; do not re-litigate it.
- **The off-road crash left no trace.** `boot-operator.log` has no fault, no `SIGSEGV`, and no crash report exists; it ends `window closed; graceful guest shutdown pending` after a normal load report.
- **`TOTK_IGNORE_GAMEPAD=1`** (`runtime/gamepad.m`) refuses to attach a physical controller, so a scripted run is attributable. A connected Pro Controller is otherwise OR-ed into every frame.

## What the evidence leaves

The upload is sound and the format table is sound, so the fault is downstream of both, in one of three places. Test them in this order — the first is also the cheapest to instrument.

1. **Handle resolution at draw.** Textures are bound as `NVNtextureHandle` values (`hle_nvn.c` `nvnCommandBufferBindTexture` → `bind_handles(OP_BIND_TEX)`), so something must turn a handle into an `id<MTLTexture>` when the draw is encoded. Count unresolved handles there; a shader with an unbound texture argument reads zero, which would leave the lighting gradient intact and the albedo black.
2. **Sampler binding.** `nvnCommandBufferBindSeparateSampler` and the combined-handle sampler id in bits 20+ share the texture slots. A wrong or default sampler changes filtering and addressing but not the presence of grain; rule it out by checking a surface that is flat in the source.
3. **A G-buffer or lighting pass.** The race submits **114.9 render passes per frame** and the showroom loses its lights, so a broken intermediate target (`0x27` RGBA16Float is in the histogram) would drop albedo and light together — the one hypothesis that explains both symptoms at once.

## Code to touch

| Work | Touch |
| --- | --- |
| Handle resolution | `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` (`bind_handles`, `resolve_deferred`, `life_bind` already decodes `id`/`ser`), and the draw encoder in `nvn_mtl.m` that consumes `OP_BIND_TEX`. |
| Where a texture is looked up | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_tex.c` (`TextureGetHandle` is in the null-device list as `DeviceGetTextureHandle`), `nvn_tex.h`. |
| Instrumentation | `nvn_mtl.m` already carries `DM_NVN_UPLOG` (note: it also logs `[pkg_upload_path]`), `DM_TEX_FMT_STATS`, `DM_NVN_FMT_LOG`. |
| Do not | Do not touch `games/totk` or `games/by3`; do not change the format table; do not "fix" this by forcing a default texture. |

## Read these first, and only these

- `.operator/guides/running-the-nfs-boot.md` — repo build, work-tree symlinks, the cadence rule for menus, the key table.
- `.operator/specs/nfs-port.md` — Claim boundaries.
- `docs/donot.md` #14, #19, #22, #27.

## Do-Not entries that apply here (4 of 27)

- #14 — discard a rate/frame batch taken under GPU contention.
- #19 — build from the repo; re-sync and force the rebuild.
- #22 — a headless boot reaches states a windowed one does not; change one variable at a time.
- #27 — pass a Mac keycode below 128 to `key`, not a pad bit.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass.
- **Offer the operator a run** after the change, built from the repo (Do-Not #19), with the log captured. This slice's subject is visible on screen, so his run is the evidence — and he reaches the race in a fraction of the time a scripted cadence takes.

## When this slice closes

Advance "Where the work is", "Next action", "Completed", "In progress" and "Queued" in the same commit (Do-Not #11); append any new fault after the last existing entry; rewrite this seed from what S3.3b leaves behind.
