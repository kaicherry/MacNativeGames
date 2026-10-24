---
slice: S3.1
stateFile: docs/STATE.md
donotCount: 20
seededFrom: 45619c7
---

# Next slice — S3.1: input, storage and saves

## The next action — verbatim

S3.1 — Implement input, storage, and saves (roadmap S3), the next approved work unit now that S2 (graphics and first frame) is complete at S2.20 and the out-of-band audio line (S2.21/S2.22/S2.23) is closed. **Slice S3.1 into work units before starting it**; its first unit and its close condition are set when it is sliced. The measured entry blockers: a windowed `TOTK_GPU_ALLOW_SUBMIT=1` run mounts a save directory through the fs host (`host save storage: user-data/save/db047cf775247afa9b3bd6d11bcf9740`, `fs mount save:`), runs to frame 2404, and then stops at exit 1 on an unimplemented **`pctl`** service (`sm: service "pctl" not implemented`); the `savedata` service is still absent from `games/nfs/runtime`, and whether the save-data access notice (S2.11 frame 1788) was passed is not established by the log. Input itself is already served by the HID service (S2.2b) and the pad is read (S2.11). Do not re-open the descriptor, release, publish or credit lines without a new observation (Do-Not #8, #13, #15, #17). Do not touch `games/totk` or `games/by3`.

S3.1 closes when: not yet defined — S3.1 must be sliced into work units first, and the unit that touches the save path sets its own close.

## What the next session does first

Name the work units, then start the first one. Two service gaps are measured: `pctl` is what the boot actually stops on (frame 2404, exit 1), and `savedata` is absent while the fs host already mounts a save directory. Decide the first unit from what the windowed run shows at the save-data notice — if the notice is still waiting for Ⓐ, that is `savedata`; if it is passed, `pctl` is the stop to clear. Input is not the first unit: the HID service already serves it and the pad is read. Write the unit's close condition into `docs/STATE.md` § "Next action" before starting it.

## What is settled — do not re-derive

- **S2.20 bound the visual claim.** The first 8 translated shaders (4 vertex + 4 pixel) are translated, bound, drawn and presented in frame 2; visual correctness is claimed only for the named captures. No gameplay.
- **The audio line is closed (S2.21–S2.23).** The queue is bound to the system default and follows it (S2.21); a wedged queue is detected by a started, queued queue whose `played_samples` stops advancing and is rebuilt on the default device (S2.22); appended PCM now waits in a 240 ms host ring fed by its own thread holding 40 ms enqueued, and the tag returns at append so the guest runs ahead (S2.23).
- **S2.23 observed:** under `TOTK_GPU_ALLOW_SUBMIT=1`, `played` advanced ~97,700 frames per 2 s line with no stalled interval, `ring=` 213–240 ms, `dev=` 37 ms, `queued` 48–52, zero drops, through windows whose worst frame was 203.88 ms and 124.72 ms. The operator's windowed run then heard it hold through a **122.97 ms** frame window with the mix loud, and reported "looks/sounds good" — the close condition is met, not inferred. That run ended at exit 1 on the unimplemented `pctl` service (see the next action).
- **The gate** `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0 on 2026-10-23.

## Withdrawn — proven wrong, do not use

- **"The guest is paced by released buffers, so the queue depth is the whole cushion."** Withdrawn by S2.23: the tag returns at append, and the ring is the cushion.
- **"The audio is silent because of routing, content or level."** Refuted in S2.22/S2.23: default device running at volume 1, `played` advancing, captured stereo normal-band and audible via `afplay`.
- **"Detect a stalled feed by `appends` advancing while `played` does not."** S2.22: a stall stops appends too; the detector keys on `played` with the queue non-empty.
- **The frame-completion descriptor/release/publish/credit lines** (S2.16–S2.19; Do-Not #8/#13/#15/#17).
- **The S2.22 stall detector is built but has never fired live.** Do not read its silence as health or as a fault.

## Code to touch

| Work | Touch |
| --- | --- |
| The save service (first unit, to be confirmed when sliced) | `games/nfs/runtime/` — a new `savedata` HLE beside the other service files, registered where the runtime registers its IPC services; `services.c`/the HLE table as that tree does it. Read `games/nfs/runtime/services.c` and a neighbouring service before adding one. |
| Evidence | The boot's save-data notice (S2.11 frame 1788) and the `savedata` open/commit log lines of a bounded headless boot; `.babysteps/scratch/`. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim gameplay; do not re-open the audio path unless a run shows a new fault; do not read a rate taken with the GPU held (Do-Not #14). |

## Read these first, and only these

- `.operator/specs/nfs-port.md` § the manifest authority, the load set and the provider-resolution stance (a new service is a provider question, not a new tree).
- `.operator/specs/nfs-audio.md` § "The host ring" — only if the slice touches audio.
- `docs/STATE.md` § "Next action".
- `docs/donot.md` #6, #8, #11, #13, #14, #15, #17, #19, #20.

## Do-Not entries that apply here (9 of 20)

- #6 — the append descriptor's `data_offset` is unset by this title; an offset past capacity means zero.
- #8 — do not close the present-stop line by a rate or by runs taken while the GPU was contended.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.
- #17 — do not name main's own call by the SDK function an unwind frame reaches.
- #19 — do not treat a work tree as the repo; re-sync it with `build_app.py`'s excludes and force the rebuild before any run.
- #20 — do not read the absence of a sound as a broken path without measuring the samples.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passes (255 tests, 1 skipped, exit 0 on 2026-10-23).
- A bounded boot is evidence only when it is the observation a claim rests on; its stop rate or frame touch is reportable only with the GPU otherwise free (Do-Not #14).
- Every claim tied to a recorded observation; no gameplay claim.
- **Offer the operator a run after any change that alters what he can see or hear** (`.operator/operator.md`), built from the repo (Do-Not #19). The next review point is the boot getting past the save-data notice into the game proper.

## When this slice closes

Write the next seed from what S3.1 leaves behind, re-verify the Do-Not count (20 or higher), and commit the state and the seed.
