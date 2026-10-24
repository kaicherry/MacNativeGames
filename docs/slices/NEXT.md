---
slice: S2.23
stateFile: docs/STATE.md
donotCount: 20
seededFrom: 7f50555
---

# Next slice — S2.23: give the audio a host-side cushion

## The next action — verbatim

S2.23 — Give the audio a host-side cushion so GPU frame hitches stop starving it (out of band, operator-reported). The game's audio is heard headless but silent with `TOTK_GPU_ALLOW_SUBMIT=1`: the device queue holds 2 × 256-frame buffers (~10.6 ms) and is fed guest-driven, so a 120–166 ms frame empties it (measured: `queued=0` at 10 of 25 trace samples with submission vs `queued=2` at 63 of 67 without; the captured stereo is normal-band −27 dBFS and audible via `afplay`; the device is the running system default at volume 1). The fix is to copy appended PCM into a host ring buffer and feed the device from a dedicated thread holding a ~100–200 ms cushion, decoupling the device from the guest's frame cadence.

S2.23 closes when a run with `TOTK_GPU_ALLOW_SUBMIT=1` plays continuously through frame hitches of ≥120 ms, with the trace reporting the ring's depth rather than `queued=0`. Then S3.1 (input, storage, saves) resumes.

## What is settled — do not re-derive

- **The audio path is correct; the fault is starvation.** The device is the running system default (`kAudioDevicePropertyDeviceIsRunningSomewhere` = 1), volume 1, the queue drains (`played` advances), and `NFS_AUDIO_DUMP` captures normal-band stereo (RMS −27.4 dBFS, L/R correlation +0.50, energy 200 Hz–8 kHz) that is audible via `afplay`. Do not re-open routing, content or level.
- **The measured contrast.** Headless **without** submission: `queued=2` at 63 of 67 trace samples, frames mostly 33 ms, audio heard. Headless **with** submission: `queued=0` at 10 of 25 samples, frames 120–166 ms, silent. The 2 × 256-frame queue (~10.6 ms) is shorter than one submission frame.
- **The guest is paced by released buffers** (`GetReleasedAudioOutBuffers` + the buffer event), so it cannot stay further ahead than the queue depth; the queue depth *is* the whole cushion.
- **The attract opener is silent for ~12 s** (Do-Not #20), and the boot is far slower with submission (frame 600–1800 in 100–120 s vs frame 4200 in 150 s) — a run cut short may not have opened `audout` at all. Check `AudioOut open` before reading "no sound" as a fault.
- **Do-Not #19:** a run must be built from the repo (`rsync` with `build_app.py`'s excludes, then `rm -rf build/obj build/nfs-native`). The work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs` is re-derivable, not evidence.
- **Gate.** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` — 255 tests, 1 skipped, OK on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"The queue is on the wrong device / the pin is the problem."** Refuted: the pin was removed (the queue follows `AQDefaultDevice`) and the tone through it stayed audible; the default never changed (16/16 samples) and the device reports running.
- **"The game's mix is inaudible / the level is too low."** Refuted: the captured bytes are audible via `afplay`, and removing the downmix's 0.5 halving (+6 dB) changed nothing live.
- **"The device goes silent while the queue drains."** Not supported: `running=1` and `played` advancing throughout.
- **"Video switching resets the audio."** Not a reset — the video/GPU work *starves* the queue (the operator's hypothesis, confirmed in mechanism).
- **Earlier:** "detect a stalled feed by `appends` advancing while `played` does not" (S2.22); the frame-completion descriptor/release/publish/credit lines (S2.16–S2.19; Do-Not #8/#13/#15/#17).

## Code to touch

| Work | Touch |
| --- | --- |
| The ring buffer and feeder | `games/nfs/runtime/audio_out.c` — `AudioOut` (add the ring), `append` (copy PCM into the ring instead of enqueueing directly), `played`/`release_head` (keep the released-tag accounting so the guest stays paced), `open_device` (start the feeder thread), `audio_out_close` (join it). |
| Evidence | `.babysteps/scratch/S2.22-audio-interruption/{headless-audio.log,headless-submit.log,level-run.log,mix.raw,mix.wav}` — the audible/silent pair, the queue-depth counts, the captured mix. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim gameplay; do not read a rate taken with the GPU held (Do-Not #14); do not re-open routing, content or level. |

## Read these first, and only these

- `.operator/specs/nfs-audio.md` § "Why it is silent under GPU submission (S2.23, open)" and § "The path".
- `docs/STATE.md` § "Next action".
- `docs/donot.md` #11, #14, #19, #20.

## Do-Not entries that apply here (4 of 20)

- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #19 — do not treat a work tree as the repo; rebuild from the repo before any run.
- #20 — do not read the absence of a sound as a broken path without measuring the samples.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- A bounded boot is evidence only when it is the observation a claim rests on; its stop rate or frame touch is reportable only with the GPU otherwise free (Do-Not #14).
- Every claim tied to a recorded observation; no gameplay claim.
- **Offer the operator a run after any change that alters what he hears** (`.operator/operator.md`), built from the repo (Do-Not #19).

## When this slice closes

Write the next seed from what S2.23 leaves behind (S3.1 is next), re-verify the Do-Not count (20 or higher), and commit the state and the seed.
