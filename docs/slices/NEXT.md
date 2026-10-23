---
slice: S2.22
stateFile: docs/STATE.md
donotCount: 20
seededFrom: f845abd
---

# Next slice — S2.22: keep the audio alive through an interruption

## The next action — verbatim

S2.22 — Keep the audio alive through an interruption. **The game's audio is now heard** (operator: "game audio that time", `stereo.log`): the queue had been created on the `AQDefaultDevice` pseudo-device and is now bound explicitly to the default device's UID and follows it when the default changes. Do not re-open that — `NFS_AUDIO_TEST=1` proved the device and process with a 440 Hz tone through the same queue, and the stereo trace (`peak` 15885, 99.5% non-zero, `mean` 1302) ruled out the downmix and a DC offset. The one open defect is the **interruption**: a system alert once cut the stream with a pop while `kAudioQueueProperty_IsRunning` stayed true, so the `IsRunning` monitor cannot see it. Detect a stalled feed instead — `appends` advancing while `played` does not, with `state == STATE_STARTED` — and rebuild the queue (`AudioQueueDispose`, `AudioQueueNewOutput`, re-bind `kAudioQueueProperty_CurrentDevice`, `AudioQueueStart`, release the lost tags so the guest does not stall). `Start`/`Stop`/`Flush`/`Close` and the default-vs-queue UIDs are already logged. Do not claim gameplay. Do not touch `games/totk` or `games/by3`.

S2.22 closes when an interruption is reproduced and the sound returns without restarting the boot, with the log naming the detection and the rebuild; or the read fails with its own message and the command that produced it. Then S3.1 (input, storage, saves) resumes — see the S3.1 note below.

## What is settled — do not re-derive

- **S2 is complete at S2.20; visual correctness is bounded to the named captures.** The frames and their numbers are in the S2.20 section (`docs/STATE.md`): S2.6 frame 441 (first non-black, a white bar lower right); S2.10 frames 809 the EA logo, 1406 the NFS logo screen, 2307 and 3206 racing footage, 1718 the logo screen with `PRESS ⊕`, 3518 without it, 5019 a cockpit; S2.11 frames 1371/1487 (logo screen, prompt up at 1487) and 1788/2087/2629/3229/4276 (the save-data notice); S2.20 frame 1778 (`boot-end.png`).
- **The 8 translated shaders are 4 vs + 4 ps, and frame 2 binds and presents them (S2.20).** Keys are in the S2.20 section. A shader named only in a run with `TOTK_GPU_ALLOW_SUBMIT` off is not evidence of drawing — that run encodes but never submits — and that was the leg S2.20 closed with one boot.
- **Presenting is stable and the old stop is fixed (S2.9/S2.10).** S2.9's equal-priority core rotation (4 ms `CORE_SLICE_NS`) fixed the 354–677 stop; S2.10's runs present to frame 3600 (130 s) and 5447 (185 s) with zero stops. What remains is intermittent: 1 of 20 bounded 65 s boots (S2.13 r17) and 3 in 34 post-fix watchdog runs; do not pin it to a frame (Do-Not #8) and do not re-open the descriptor/release/publish/credit lines (S2.16–S2.19; Do-Not #13/#15/#17).
- **The guest reacts to input (S2.11).** A ⊕ press moved the boot from the `PRESS ⊕` title screen to the save-data access notice, which waits for Ⓐ.
- **Audio output recovers; the attract opener is silent (S2.21).** `audio_out.c` opens one Core Audio `AudioQueue` and a `device_monitor` thread now restarts it when Core Audio stops it (a device change, an interruption); `NFS_AUDIO_TRACE` reports appends / loudest sample / played samples / queue depth / volume every ~2 s. The opening attract screens produce `peak` 0 for ~12 s before real audio — "no sound" right after launch is the game, not the port, and a mix claim needs the trace, not silence (Do-Not #20).
- **A run must be built from the repo, not from the work tree (Do-Not #19).** `nfs-work/nfs` is an `rsync -a` snapshot whose preserved mtimes make a stale tree look current; before offering a run, re-`rsync` with `build_app.py`'s excludes (`/extracted /rewritten /build /analysis /.venv /tooling /user-data`) and force it (`rm -rf build/obj build/nfs-native`).
- **Host diagnostics are English for `games/nfs` only (S2.20 check).** Every Han string in `games/nfs` is a `TOTK_UI("中文","English")` pair (plus the `swkbd_host.c` keyboard fixtures, which are input data). `games/by3` (444 emitted lines) and `games/totk` (474) still emit Chinese — S2.7's pass was NFS-only. Do not read an NFS language claim as covering them, and do not touch those trees from this port.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0, 22.608 s on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"A frame task can be submitted with a zero closure" (S2.19).** Refuted: the execute closure is a relocated function pointer on all 20 submitter paths.
- **"A dequeued frame item with `[item+0x30] == 0` drops the completion" (S2.18).** Refuted.
- **"main issues `nn::os::ReleaseSemaphore`" (S2.18); "the publish drops a wake" (S2.17); "the loop body reaches the completion" (S2.16); "a worker takes the credit via `TryAcquireSemaphore`" (S2.13/S2.15).** Refuted; see `docs/STATE.md`.
- **"No credit is taken by a third party", read off the release/acquire balance (S2.14).** Conclusion holds, argument does not (Do-Not #15).
- **Earlier withdrawn:** "the present stop is a lost wakeup/condition race"; "no signal event appears in r4's rings" (0.6 s ring, Do-Not #12); "the workers stop being woken"; "the present stop is a defect at the submitter's descriptor".
- **History, not re-derived:** frame 441, p50 33.33 ms, 255 tests, 721 strings, the one-shot semaphore `0x…3db8` row, S2.14's discarded GPU-contended batch.

## Code to touch

S3.1 is not yet sliced, so this is what its first unit (the save path) will touch, not a decided plan:

| Work | Touch |
| --- | --- |
| Save-data HLE | `games/nfs/runtime/` (there is **no** `SaveData` HLE today) and the service registration it must join (`games/nfs/runtime/system_services.c`, `runtime_nfs_override`); the SDK `nn::fs`/save surface it must answer. |
| The captures | `.babysteps/scratch/S2.11-press-plus/` (`P-pre1/P-pre2.png`, `P-post1..5.png`, `bootP.log`, `emit-P.log`) — the notice and its Ⓐ. |
| Input (already served) | `games/nfs/runtime/hid_service.c`, `gamepad.m`, `input.c` — the pad read the notice already sees. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim gameplay; do not rest a claim on a run that died before its window (Do-Not #10); do not read a rate taken with the GPU held (Do-Not #14). |

## Read these first, and only these

- `docs/STATE.md` § "S2.11" (the notice, its frame and its Ⓐ), "S2.20" (the shader/visual boundary and the captures), and the S2.2b/S2.3 HID records.
- `.babysteps/scratch/S2.11-press-plus/bootP.log` and the `P-*.png` captures.
- `.operator/specs/nfs-port.md` § "Claim boundaries" — the visual and language boundaries.
- `.operator/specs/nfs-account.md` — the account HLE pattern (a service that must not fall through).
- `docs/donot.md` #7–#18.

## Do-Not entries that apply here (10 of 20)

- #7 — do not call a log bilingual because its source comments carry an English sentence.
- #8 — do not name a stall by the frame one run happened to stop at; report the range and the runs.
- #10 — do not rest a probe's attribution on a separate control run that dies before its window.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #12 — do not name a stall's cause from a trace window that ends after the stall began.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #17 — do not name main's own call by the SDK function an unwind frame reaches.
- #18 — do not gate a slice's close condition behind a prohibition on the evidence that condition demands.
- #19 — do not treat a work tree as the repo; `rsync -a` mtimes hide the drift, so rebuild from the repo before any run.
- #20 — do not read the absence of a sound as a broken path without measuring the samples.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- A bounded headless boot is evidence only when it is the observation a claim rests on; its stop rate or frame touch is reportable only with the GPU otherwise free — a batch taken while another local process held the GPU is discarded (Do-Not #14). Check the GPU before any batch.
- Every claim tied to a recorded observation; no gameplay claim; save-data behaviour is not claimed until the guest's Ⓐ is answered by a host call and a frame shows the result.
- **Offer the operator a run after any change that alters what he sees or hears** (`.operator/operator.md`), built from the repo (Do-Not #19). The next review point is when the boot gets past the licence/attract screens; do not schedule one before that.

## When this slice closes

Write the next seed from what S3.1's first unit leaves behind, re-verify the Do-Not count (18 or higher), and commit the state and the seed.
