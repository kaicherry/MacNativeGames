---
slice: S3.1b
stateFile: docs/STATE.md
donotCount: 21
seededFrom: 64ea0f4
---

# Next slice — S3.1b: reproduce the `pctl` open and record what the game asks it

## The next action — verbatim

S3.1b — Reproduce the `pctl` open and record the commands the game sends it, from the operator's windowed build, which is the review point. S3.1a is done: `games/nfs/runtime/pctl_service.c` registers the `pctl` service, so an open of it is answered (`rc 0`, each client out-buffer zeroed, the command logged) instead of stopping the boot. The open was **not** reproduced from a scripted input file: three repo-built runs (140 s and 200 s bounds; frames 3900, 5700) pressed Ⓐ past the save-data notice and swept fourteen buttons, and no `pctl` open occurred at all — `[nfs-pctl]` has never printed. The reproducing session is the operator's windowed build, so the first action is to offer it, capture the log, and read the `[nfs-pctl]` lines out of it. If the game asks a command whose shape the permissive answer does not fit, write that command's case from the logged shape. Then S3.1c — the `savedata` service, absent from `games/nfs/runtime` while the fs host already mounts the save directory. Do not touch `games/totk` or `games/by3`.

S3.1b closes when: the operator's windowed run logs at least one `[nfs-pctl] #n cmd=…` line and the boot passes it, or the command it asks is implemented to its observed shape and the log shows the game proceeding past it.

## What the next session does first

Offer the operator the windowed run (`.operator/operator.md`; this is the review point — the boot past the save-data notice, into the game proper). Tell him what changed: `pctl` is now answered, so where the recorded runs died at `sm: service "pctl" not implemented` the boot should carry on. Capture the log and read every `[nfs-pctl]` line; that line is the measurement. Do not spend more boots hunting an input schedule — three boots and 25 presses did not reproduce the open (Do-Not #21).

## What is settled — do not re-derive

- **The save-data notice is passed.** Ⓐ (Mac key 38, button bit 0) accepts it, and the next log lines are `host save storage: user-data/save/db047cf775247afa9b3bd6d11bcf9740` and `fs mount save: -> …`. S2.11's ⊕-only run put the notice up at frame 1788 and sat on it to frame 5100 with no mount, and `host save storage` had been in the tree since `8e05a5e` (2026-10-22 04:03), so that absence was real and not a missing code path.
- **The fs host mounts the save directory with no `savedata` service present**, and the boot proceeds; mounting does not need `savedata`.
- **`pctl` is the recorded stop and now has a handler:** `games/nfs/runtime/pctl_service.c`, registered through `runtime_ipc_register`, answering `rc 0` with every client out-buffer (C) zeroed, and logging `[nfs-pctl] #n cmd=… type=… x/a/b/w/c…`.
- **The NFS runtime build globs `runtime/*.c`** (`games/nfs/Makefile:22`), so a new runtime source file needs no manifest edit. Build from the repo (Do-Not #19): `rsync -a` with `build_app.py`'s excludes, `rm -rf build/obj build/nfs-native`, `make WORK=. runtime`.
- **Input needs no unit.** The HID service (S2.2b) serves it and the pad is read (S2.11).
- **The gate** passed 255 tests, 1 skipped, exit 0 on 2026-10-23.
- **The frame numbers here are not claims.** No rate or frame count is reportable from these runs (Do-Not #14).

## Withdrawn — proven wrong, do not use

- **"The `pctl` open follows the save mount automatically, ~20 s later."** Withdrawn by S3.1a: three runs carrying the mount to frames 3900 and 5700 produced no open. The reading came from the ordering of a log that is rate-limited (Do-Not #21).
- **"Pressing the buttons the operator pressed reproduces his stop."** S3.1a's 11-press schedule and 14-button sweep did not.
- **The `pctl` handler's answer is unexercised.** Do not read its presence as a verified answer to any command.
- The withdrawn S2.16–S2.19 descriptor/release/publish/credit lines (Do-Not #8/#13/#15/#17).
- The S2.22 stall detector is built but has never fired live.

## Code to touch

| Work | Touch |
| --- | --- |
| `pctl`, to the observed command | `games/nfs/runtime/pctl_service.c` — there is no `switch (rq->cmd)` yet; every command takes the permissive answer. Add a case per command the operator's log names, written from that command's logged descriptor shape. |
| Evidence | The operator's windowed log: `[totk-ipc] opened service pctl` and one `[nfs-pctl] #n cmd=…` line per call. `.babysteps/scratch/`. |
| `savedata` (S3.1c) | `games/nfs/runtime/` — a new `savedata` HLE beside the other service files; read `hid_service.c` and `friends_service.c` as the pattern. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim gameplay; do not read a stop rate or frame count taken with the GPU held (Do-Not #14); do not answer an unrecognised command's value as if its shape were known. |

## Read these first, and only these

- `.operator/specs/nfs-port.md` § the manifest authority, the load set and the provider-resolution stance (a new service is a provider question, not a new tree).
- `docs/STATE.md` § "S3.1a" and "Next action".
- `docs/donot.md` #6, #8, #11, #13, #14, #15, #17, #19, #20, #21.

## Do-Not entries that apply here (10 of 21)

- #6 — the append descriptor's `data_offset` is unset by this title; an offset past capacity means zero.
- #8 — do not close the present-stop line by a rate or by runs taken while the GPU was contended.
- #11 — do not close a slice while the STATE.md top sections still name the finished slice.
- #13 — do not read a counter that fires on the healthy path as evidence of the fault.
- #14 — do not read a stop rate or frame count taken while another local process held the GPU as a property of the build.
- #15 — do not read a release/acquire balance as naming who took a credit.
- #17 — do not name main's own call by the SDK function an unwind frame reaches.
- #19 — do not treat a work tree as the repo; re-sync it with `build_app.py`'s excludes and force the rebuild before any run.
- #20 — do not read the absence of a sound as a broken path without measuring the samples.
- #21 — do not read the presses a session's log shows as the presses it made.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (255 tests, 1 skipped at S3.1a).
- A bounded boot or a windowed run is evidence only when it is the observation a claim rests on; its stop rate or frame touch is reportable only with the GPU otherwise free (Do-Not #14).
- Every claim tied to a recorded observation; no gameplay claim.
- **Offer the operator a run after any change that alters what he can see or hear** (`.operator/operator.md`), built from the repo (Do-Not #19). The review point is the boot getting past the save-data notice into the game proper.

## When this slice closes

Write the next seed from what S3.1b leaves behind, re-verify the Do-Not count (21 or higher), and commit the state and the seed.
