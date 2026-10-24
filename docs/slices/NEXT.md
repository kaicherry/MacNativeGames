---
slice: S3.1
stateFile: docs/STATE.md
donotCount: 20
seededFrom: 677e474
---

# Next slice — S3.1: input, storage, and saves

## The next action — verbatim

S3.1 — Implement input, storage, and saves (roadmap S3), the next approved work unit now that S2 (graphics and first frame) is complete at S2.20 and the out-of-band audio line (S2.21/S2.22) is closed. S2.20 verified the shader and visual output to its bound: the first 8 translated shaders are 4 vertex + 4 pixel stages, all translated inside frame 2, which is bound (`ProgramSetShaders=6`, `CommandBufferBindProgram=11`), drawn (`CommandBufferDrawArrays=12`) and presented (`QueuePresentTexture=1`, `GPU 0.9 ms`) — named by one submission-enabled boot with a fresh `TOTK_SHADER_CACHE` (`.babysteps/scratch/S2.20-shader-visual/`); each captured frame's content and frame number are recorded in the S2.20 section; and visual correctness is claimed only for them (no gameplay, no final picture). The measured entry blockers for S3.1: the boot reaches the save-data access notice and waits for Ⓐ (S2.11 frame 1788), and `games/nfs/runtime` contains **no** `SaveData` HLE at all, so nothing can answer that Ⓐ yet; input itself is already served by the HID service (S2.2b) and the pad is read (S2.11). **Slice S3.1 into work units before starting it**; its first unit and its close condition are set when it is sliced. Do not re-open the descriptor, release, publish or credit lines without a new observation (Do-Not #8, #13, #15, #17). Do not touch `games/totk` or `games/by3`.

S3.1 closes when: not yet defined — S3.1 must be sliced into work units first, and the unit that touches the save path sets its own close.

## What is settled — do not re-derive

- **S2 is complete at S2.20; visual correctness is bounded to the named captures.** Frames and numbers are in `docs/STATE.md` § S2.20: S2.6 frame 441 (first non-black); S2.10 809 EA logo, 1406 NFS logo screen, 2307/3206 racing footage, 1718 the logo screen with `PRESS ⊕`, 3518 without, 5019 cockpit; S2.11 1371/1487 (logo screen, prompt at 1487), 1788/2087/2629/3229/4276 (the save-data notice); S2.20 1778.
- **The 8 translated shaders are 4 vs + 4 ps, and frame 2 binds and presents them (S2.20).** A shader named only in a run with `TOTK_GPU_ALLOW_SUBMIT` off is not evidence of drawing.
- **Presenting is stable; the old stop is fixed (S2.9/S2.10).** S2.9's equal-priority core rotation (4 ms `CORE_SLICE_NS`) fixed the 354–677 stop; S2.10 presents to frame 3600 (130 s) and 5447 (185 s) with zero stops. What remains is intermittent (1 of 20; 3 in 34 post-fix runs); do not pin it to a frame (Do-Not #8) and do not re-open the descriptor/release/publish/credit lines (Do-Not #13/#15/#17).
- **The guest reacts to input (S2.11).** A ⊕ press moved the boot from the `PRESS ⊕` title screen to the save-data access notice, which waits for Ⓐ. That Ⓐ is S3.1's entry.
- **Audio is closed out of band (S2.21/S2.22).** The queue is bound explicitly to the output device's UID and follows the default; the game's audio is heard. `device_monitor` restarts a queue Core Audio *stops*, and — S2.22 — rebuilds one that is fed but plays nothing while `kAudioQueueProperty_IsRunning` stays true (detected by `played_samples` frozen with the queue non-empty for 3 × 200 ms; a dedicated `aq_lock` guards the queue's lifetime). The attract opener is silent for ~12 s, so "no sound" at launch is the game (Do-Not #20). **The S2.22 recovery is not yet observed live.** One windowed run played ~4 min with no `rebuilt on` line, so the detector is quiet on the healthy path; offer the operator a run, and claim the recovery only when a run shows the `rebuilt on` line and sound returns without restarting the boot.
- **A run must be built from the repo, not from a work-tree snapshot (Do-Not #19).** `nfs-work/nfs` is an `rsync -a` snapshot whose preserved mtimes make stale sources look current; re-`rsync` with `build_app.py`'s excludes (`/extracted /rewritten /build /analysis /.venv /tooling /user-data`) and force it (`rm -rf build/obj build/nfs-native`) before any run.
- **Host diagnostics are English for `games/nfs` only (S2.20 check).** Every Han string in `games/nfs` is a `TOTK_UI("中文","English")` pair (plus the `swkbd_host.c` fixtures, which are input data). `games/by3` (444 lines) and `games/totk` (474) still emit Chinese; do not touch those trees from this port.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, OK on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"Detect a stalled feed by `appends` advancing while `played` does not" (S2.22).** The guest is paced 1:1 by released buffers (`stereo.log`: `queued` steady at 2, `appends` ≈ `played`), so a stall stops appends too; the detector keys on `played_samples` with the queue non-empty instead.
- **"A frame task can be submitted with a zero closure" (S2.19); "a dequeued item with `[item+0x30] == 0` drops the completion" (S2.18); "main issues `nn::os::ReleaseSemaphore`" (S2.18); "the publish drops a wake" (S2.17); "the loop body reaches the completion" (S2.16); "a worker takes the credit via `TryAcquireSemaphore`" (S2.13/S2.15).** Refuted; see `docs/STATE.md`.
- **"'No credit is taken by a third party', read off the release/acquire balance" (S2.14).** Conclusion holds, argument does not (Do-Not #15).
- **Earlier withdrawn:** "the present stop is a lost wakeup/condition race"; "no signal event in r4's rings" (Do-Not #12); "the workers stop being woken"; "the present stop is a defect at the submitter's descriptor".
- **History, not re-derived:** frame 441, p50 33.33 ms, 255 tests, 721 strings, S2.14's discarded GPU-contended batch.

## Code to touch

| Work | Touch |
| --- | --- |
| S3.1's save path (unlocated) | `games/nfs/runtime` has **no** `SaveData` HLE; the service layer that registers IPC handlers (`services.c`, the `runtime_ipc_register` pattern used by `audio_out.c` / `hid_service.c`) and `account.c` (the nearest existing HLE) are where a first unit will land. Slicing names the files. |
| Evidence | `.babysteps/scratch/S2.11-press-plus/` (the Ⓐ wait), `docs/STATE.md` § S2.11; `.babysteps/scratch/S2.22-audio-interruption/` if the audio run is offered. |
| Do not | Do not touch `games/totk` or `games/by3`; do not re-open the frame-completion lines; do not claim gameplay; do not read a rate taken with the GPU held (Do-Not #14). |

## Read these first, and only these

- `docs/STATE.md` § "Next action" (S3.1) and § "S2.11" (the Ⓐ wait) / "S2.22" (the audio close).
- `.operator/specs/nfs-audio.md` — only when offering or interpreting the audio run.
- `docs/donot.md` #5, #8, #11, #14, #18, #19, #20.

## Do-Not entries that apply here (7 of 20)

- #5 — do not let one `runtime_fail` cover several calls; log each call's own code at the site.
- #8 — do not name a stop by the frame one run happened to stop at; report the range and the runs.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #18 — do not gate a slice's close condition behind a prohibition on the evidence that condition demands.
- #19 — do not treat a work tree as the repo; `rsync -a` mtimes hide the drift, so rebuild from the repo before any run.
- #20 — do not read the absence of a sound as a broken path without measuring the samples.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- A bounded headless boot is evidence only when it is the observation a claim rests on; its stop rate or frame touch is reportable only with the GPU otherwise free — a batch taken while another local process held the GPU is discarded (Do-Not #14). Check the GPU before any batch.
- Every claim tied to a recorded observation; no gameplay claim; save-data behaviour is not claimed until the guest's Ⓐ is answered by a host call and a frame shows the result.
- **Offer the operator a run after any change that alters what he sees or hears** (`.operator/operator.md`), built from the repo (Do-Not #19). The next review point is when the boot gets past the licence/attract screens; do not schedule one before that.

## When this slice closes

Write the next seed from what S3.1 leaves behind, re-verify the Do-Not count (20 or higher), and commit the state and the seed.
