---
slice: S3.1b
stateFile: docs/STATE.md
donotCount: 22
seededFrom: 4fda528
plannedTokens: 60000
---
# Next slice — S3.1b (second limb): make the `pctl` reply match what the SDK reads
## The next action — verbatim

S3.1b (second limb) — Make the reply to the `pctl` open match what the SDK reads after it, so the boot
stops crashing at `sdk+0x3489dc`. The first limb is met: the operator's windowed runs logged
`[nfs-pctl] pctl #1 cmd=1 type=6 x0 a0 b0 w0 c0 data=8` / `in: 00000000 00000000` for each of `pctl`,
`pctl:s`, `pctl:a` and `pctl:r`, naming `pctl:s` and then `pctl:a` as the next unregistered name on
successive runs, and registering the family cleared all four. The guest then took a `SIGSEGV`
(`signal=0xb`, `esr=0x92000006`, `pc=sdk+0x3489dc`) inside the same SDK chain that opened them
(`sdk+0x345634` → `0x34840c` → `0x34882c` → `0x3489dc`), with no fifth `pctl` request sent. The
candidate cause is the reply to `cmd=1`: the handler answers `rc 0` with a 4-byte zero datum and no
moved handle, and the SDK then dereferences. Reproduce it **windowed** — the four opens do not occur
headless (same binary and schedule, `TOTK_HEADLESS=1` reached no save mount and no open; `TOTK_AUTOLOAD`
unset, so autoload was never on; Do-Not #22) — then read `sdk+0x3489dc`. S3.1c — the `savedata` service, still absent from
`games/nfs/runtime` while the fs host mounts the save directory — follows; S3.1d is the save round-trip.
Do not touch `games/totk` or `games/by3`.

S3.1b closes when the reply matches what the SDK reads and the log shows the game proceeding past the
`pctl` setup — no `SIGSEGV` at `sdk+0x3489dc`. Its first limb is met: the windowed runs logged
`[nfs-pctl]` lines and the boot passed all four `pctl` opens.

## What the next session does first

Reproduce it in a **windowed** run, not headless: the same binary and the same `TOTK_INPUT_FILE` schedule
opened the four ports windowed and none headless, and `TOTK_AUTOLOAD` was unset in every run, so autoload
was never on (Do-Not #22). One windowed boot with the same two presses — ⊕ (Mac key 36) at `PRESS ⊕`,
then Ⓐ (key 38) on the notice — reaches the four opens and the crash (`.babysteps/scratch/S3.1b-pctl/
windowed-verify.log`); retry if it dies at `reserve guest stack…`, the windowed placement is a coin toss.

Then read `sdk+0x3489dc` (with `0x34882c`/`0x34840c`/`0x345634`, the chain that opened the four) to name
what the SDK does with each reply, and change `pctl_service.c` to answer it: the `cmd=1` reply currently
returns `rc 0` with one 4-byte zero word and no moved handle. Do not guess the shape — read it.

Build from the repo, never a stale work tree (Do-Not #19): `rsync -a` `games/nfs` with `build_app.py`'s
excludes, `rm -rf build/obj build/nfs-native`, then `make WORK=. runtime` from the work tree; a clean
`-Werror` build is evidence. Headless adds `TOTK_HEADLESS=1`. A windowed launch can die before the guest
runs at all at `FAIL: reserve guest stack, heap and alias regions below 2^39` — the placement is
randomised and the windowed path maps more of the low half, so retry rather than report it as a build
fault.

## What is settled — do not re-derive

- **The save-data notice is passed.** Ⓐ accepts it, and the next log lines are `host save storage:
  user-data/save/db047cf775247afa9b3bd6d11bcf9740` and `fs mount save: -> …`. S2.11's ⊕-only run put the
  notice up at frame 1788 and sat on it to frame 5100 with no mount (`host save storage` predates it, `8e05a5e`).
- **The fs host mounts the save directory with no `savedata` service present**, and the boot proceeds.
- **The `pctl` family takes one handler:** `pctl_service.c` registers `pctl`, `pctl:s`, `pctl:a` and
  `pctl:r`, answers `rc 0` with every client out-buffer (C) zeroed, and logs `[nfs-pctl] <session> #n
  cmd=… type=… x/a/b/w/c… data=…`. Each open's first request is `cmd=1 type=6`, `data=8` of zeros.
- **The next stop is the SDK's own crash, not a missing name** (`pc=sdk+0x3489dc`, `signal=0xb`,
  `esr=0x92000006`, no fifth `pctl` request).
- **The NFS runtime build globs `runtime/*.c`** (`games/nfs/Makefile:22`), so a new runtime source file
  needs no manifest edit; the per-version manifest is for extracted inputs only.
- **The gate** passed 255 tests, 1 skipped, exit 0 on 2026-10-23, and does not cover runtime C; no rate
  or frame count from these runs is reportable (Do-Not #14).

## Withdrawn — proven wrong, do not use

- **"The `pctl` open follows the save mount automatically" and "a scripted schedule standing in for the
  operator proves the buttons are not the difference."** Both withdrawn: headless runs carrying the mount
  to frames 3900 and 5700 produced no open, and the difference is windowed-vs-headless (Do-Not #22); a
  scripted **windowed** run reproduces the open.
- **"`pctl:s` and `pctl:r` are names no run asks for."** `pctl:s` and `pctl:a` were each named by a run,
  and `pctl:r` was asked as soon as it was registered.
- **The `cmd=1` reply shape is not established.** Do not read the crash as proof of a shape until
  `sdk+0x3489dc` is read.

## Code to touch

| Work | Touch |
| --- | --- |
| `pctl`, to the observed reply | `games/nfs/runtime/pctl_service.c` — the handler has no `switch (rq->cmd)` and no reply-shape case; `cmd=1` is answered as a plain success with one 4-byte zero word and no moved handle. Read `sdk+0x3489dc` before changing it. |
| Reference | `games/nfs/runtime/ipc.c` (`sm`, `respond`, `runtime_ipc_new_session`, `rs->move[]`/`rs->objects[]` — how a session or object id is handed back) and `hid_service.c` as the handler pattern. |
| Evidence | `.babysteps/scratch/`: the windowed run that reproduces the crash, and one `[nfs-pctl] <session> #n cmd=…` line per call. |
| `savedata` (S3.1c) | `games/nfs/runtime/` — a new `savedata` HLE beside the other service files. |
| Do not | Do not touch `games/totk` or `games/by3`; do not claim gameplay; do not read a stop rate or frame count taken with the GPU held (Do-Not #14); do not answer an unrecognised command's value as if its shape were known; do not ask the operator to boot before the windowed reproducer works. |

## Read these first, and only these

- `.operator/specs/nfs-port.md` § "Adding an IPC service" and the provider-resolution stance.
- `docs/STATE.md` § "S3.1b" and "Next action".
- `docs/donot.md` #6, #8, #11, #13, #14, #15, #17, #19, #20, #21, #22.

## Do-Not entries that apply here (11 of 22)

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
- #22 — do not read a scripted schedule's failure to reproduce a live session as a fact about the buttons; check which switch the live path flips that the script does not.

## The gate

- `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` must pass (255
  tests, 1 skipped); the suite covers the test tree, not runtime C. Every claim tied to a recorded
  observation; no gameplay claim; a bounded boot is evidence only when it is the observation a claim
  rests on, and its stop rate or frame touch is reportable only with the GPU otherwise free (Do-Not #14).
- **Budget:** planned at 60k tokens. The cap is 160k per unit, 170k only when tight; if this brief plus
  the touched files exceed 170k, split the unit before work.
- **Offer the operator a run after any change that alters what he can see or hear**
  (`.operator/operator.md`), built from the repo (Do-Not #19). The review point is the boot getting past
  the save-data notice into the game proper — which it has not yet done.

## When this slice closes

Write the next seed from what S3.1b leaves behind (S3.1c, the `savedata` service), re-verify the Do-Not
count (22 or higher), and commit the state and the seed.
