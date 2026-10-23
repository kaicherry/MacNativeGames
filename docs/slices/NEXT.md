---
slice: S2.5
stateFile: docs/STATE.md
donotCount: 5
seededFrom: d79115f
---

# Next slice — S2.5: resolve the AudioOut buffer bounds stop

## The next action — verbatim

S2.5 — Resolve the `AudioOut buffer bounds` stop. S2.4c carried the shader translator (`games/nfs/prebuilt/macos-arm64/totk_xlate` forked from BY3, plus the `make shader-tools` target that verifies it and builds `build/spirv-cross/spirv-cross` from the pinned SPIRV-Cross commit). The headless boot now translates 8 shaders (the first is `vs c10b455d…`) and runs the render loop — 300 encoded frames, GPU submission still disabled — before opening `audout:u` (`AudioOut open 48000 Hz x6 PCM16 -> Core Audio (stereo)`) and stopping at `[nfs] FAIL: AudioOut buffer bounds` (`audio_out.c:94` `append`: `data_offset + data_size <= capacity` fails on the first `AppendAudioOutBuffer`). Resolve that audio failure and continue; record only behavior demonstrated by the resulting run. No gameplay claim.

## What is settled — do not re-derive

- The shader translator is carried and `make boot` builds it: `games/nfs/prebuilt/macos-arm64/{totk_xlate,totk_xlate.sha256,README.md}` (forked from `games/by3/prebuilt/`; the NFS graphics layer is byte-identical to BY3's), plus the `shader-tools` Makefile target — copy the verified prebuilt to `<work>/build/shader-service/totk_xlate` and build `<work>/build/spirv-cross/spirv-cross` from SPIRV-Cross commit `aa217aeb6c9f0ace7a0ab233b28807edf45eb165`.
- The headless boot (`.babysteps/scratch/S2.4c-NFS-shader-translator/boot.log`, 198 lines) starts the native service, translates 8 shaders, and runs 300 NVN-encoded frames (frame 1: 27 functions / 32 calls; frame 2: 120 / 4,251 with 12 `CommandBufferDrawArrays`; p50 33.33 / p95 33.46 ms). GPU submission stays disabled (`TOTK_GPU_ALLOW_SUBMIT=1` re-enables), so no picture is observed.
- The next stop is audio: `audout:u` opens (48000 Hz × 6 ch PCM16 → Core Audio stereo), then `AppendAudioOutBuffer` fails the bounds check `data_offset + data_size <= capacity` at `games/nfs/runtime/audio_out.c:94` (`append`).
- Temp files go on the external volume: set `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames` for builds/tests (the internal data volume is full; see `.operator/guides/external-tmp.md`). The stale `games/nfs/NFS_build/` (7 GB) is removed.
- Earlier (S2.1–S2.4c): window-path blockers cleared; 516/523 NVN names resolved; device properties 0x00–0x61 answered; hid command 101 handled; local account override; `friend:u` notification queue answered; friend-invitation event bound; shader translator carried.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not claim visual correctness without an observed game frame; do not claim gameplay.

## Code to touch

| Work | Touch |
| --- | --- |
| AudioOut buffer bounds | `games/nfs/runtime/audio_out.c` `append` (~line 90-94) — the `AudioOutBuffer` descriptor parse and the `data_offset + data_size <= capacity` check. The descriptor is read as 5×uint64 (`next, sample_buffer, capacity, data_size, data_offset`, `_Static_assert` 0x28); the game's first `AppendAudioOutBuffer` (cmd 3/7) fails the check, so either the switchbrew layout does not match the game's actual struct or the buffer semantics differ. |
| Audio service | `games/nfs/runtime/audio_out.c` `manager_handler`/`append` — switchbrew "Audio services" (IAudioOutManager, IAudioOut). |

## Gate commands

- `python -m unittest discover -s tests` passes (with `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`); every claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from the previous seed.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.4c — Carry the shader translator and translate a shader toward a frame" (the audio stop is recorded there).
- switchbrew "Audio services" — the `IAudioOutManager`/`IAudioOut` command and buffer layouts.
