---
slice: S2.6
stateFile: docs/STATE.md
donotCount: 6
seededFrom: 3b11ef8
---

# Next slice — S2.6: submit the encoded frames and observe the first picture

## The next action — verbatim

S2.6 — Submit the encoded frames and observe the first game-rendered picture. S2.5 resolved the `AudioOut buffer bounds` stop: the append descriptor's `data_offset` is unset by the game (allocator fill), the host now treats an offset past capacity as zero and logs it once, and the 60 s bounded boot opens `audout:u`, appends and drains audio, and runs past frame 900 at p50 33.33 ms with no next stop. The renderer still encodes without submitting (`GPU submission disabled: work is encoded but never submitted (no picture, no GPU results); TOTK_GPU_ALLOW_SUBMIT=1 re-enables it`). Re-run the headless boot with `TOTK_GPU_ALLOW_SUBMIT=1`, record the first submitted frame and any Metal validation failure, draw error, or crash with its own message, and capture a picture through the existing diagnostic path (`TOTK_INPUT_FILE` line `shot <path>` → `dm_gfx_shot_at`). Measure first and record only what the run shows. No gameplay claim.

## What is settled — do not re-derive

- **The audio stop was a misread field, not a bad buffer.** The first `AppendAudioOutBuffer` is command 7 with a 0x28-byte input buffer (`na=1 nx=1`): `next=0xcdcdcdcdcdcdcdcd, sample_buffer=0x…d5000, capacity=0x1000, data_size=0xc00, data_offset=0xcdcdcdcdcdcdcdcd`. The sample, capacity and size are the game's; `next` and `data_offset` are allocator fill because the title never initialises them. `append` now treats an offset past capacity as zero, logs it once, and applies the check and copy to that offset. `NFS_AUDIO_TRACE=1` prints the first four descriptors.
- **Audio plays and drains.** The 60 s bounded boot appends four buffers, starts the output, and never hits `AudioOut queue full`, so `GetReleasedAudioOutBuffers` returns buffers and the queue drains. The game is paced by the output (frame p50 33.33 ms).
- **The boot now has no stop.** It reaches frame 900+, translates two more shaders (`vs bb6f978b…`, `ps 34ebe360…`), and was ended by the timer at 60 s, not by a failure. The render loop keeps producing frames.
- **No picture yet.** GPU submission stays disabled unless `TOTK_GPU_ALLOW_SUBMIT=1`. The boot leaves `TOTK_PROBE_GPU` unset for `boot` (`games/nfs/runtime/loader.c:463`); probes set it.
- **A picture path exists.** `TOTK_INPUT_FILE` drives diagnostic lines in `games/nfs/runtime/input.c` (`shot <path>`, `shotn <n> <fmt>`, `wshot <path>`, `gpucapture`, `passdump`, `anomdump`); `shot` calls `dm_gfx_shot_at(2, path)` — the second frame after the command.
- **The work tree is re-derivable, not evidence.** It lives at `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` (7.1 GB); reconstruct is ~7 min from the NSP, then `make prepare runtime` and `make boot`. Temp goes to `/Volumes/X9BS/externalTmp/MacNativeGames` (`TMPDIR`), never the internal volume.
- **Budget:** `.babysteps/config.json` plans 180,000 work-unit tokens, but the host window binds — verify the live session model each slice and size to the usable window after the compaction reserve (S2.5: 200,000 context → ~156,000).
- Earlier (S2.1–S2.5): window paths, NVN names, device properties, HID, account, `friend:u`, friend-invitation event, shader translator, audio — all resolved as recorded in the state file.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5, #6 (from `docs/donot.md`): do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls; do not read a descriptor field as data before checking the game initialises it.
- Do not claim visual correctness without an observed game frame; do not claim gameplay.

## Code to touch

| Work | Touch |
| --- | --- |
| First submitted frame | `games/nfs/runtime/loader.c:463` sets `TOTK_PROBE_GPU` only for probes; `TOTK_GPU_ALLOW_SUBMIT=1` is the boot's switch. The submit gate and its present path are in `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` — record the first failure message there verbatim, with the command that produced it. |
| Picture capture | `games/nfs/runtime/input.c` — `TOTK_INPUT_FILE` lines `shot <path>`, `shotn <n> <fmt>`, `wshot <path>`; `dm_gfx_shot_at` and `dm_gfx_window_shot` in the graphics layer. |
| Submission health | `games/nfs/third_party/d2r_gfx/gfx/nvn/nvn_mtl.m` validation/driver messages (`MDIE`, validation errors); `runtime/gfx_probe.c` for the probe path. |

## Gate commands

- `python -m unittest discover -s tests` passes (with `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`); every claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from the previous seed; do not re-measure them as if new.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.5 — Resolve the `AudioOut buffer bounds` stop" (descriptor measurement, fix, resulting run, gate, boundary).
- `docs/nfs-port-feasibility.md` — "S2.4c — Carry the shader translator and translate a shader toward a frame" (renderer state and the `TOTK_GPU_ALLOW_SUBMIT` note).
