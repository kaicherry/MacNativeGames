# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, the NFS native macOS port is in S2.6. The loader reaches `nnMain`, the HLE has run the first GPU frame, the `friend:u` open and the friend-invitation event are resolved, and the shader translator is carried (`games/nfs/prebuilt/` + the `make shader-tools` target): the latest headless boot translates 8 shaders, runs the render loop, opens `audout:u`, appends and drains its audio buffers, and passes frame 900 with **no stop** in a 60 s bounded run (GPU submission still disabled). No race, picture, or gameplay is established. The target remains the user-owned base NSP v0 at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are local build-time inputs and must not be committed. The target is Apple-silicon macOS; `games/totk/` and `games/by3/` are reference trees, not NFS runtime/build-time dependencies. The setup pass removed a stray 7.2 GB `target-port/need-for-speed-hot-pursuit-remastered/NFS_build/` (a re-derivable copy of the repo tree), added `NFS_build/` and `localTmp/` to `.gitignore`, and re-ran the code index (620 deleted files, duplicate NFS symbols gone).

S2.3 is complete: HID command 101 is handled, local account calls no longer fall through to the SDK's uninitialized account holder, and boot now stops at the missing `friend:u` service. S2.4 resolves that `friend:u` open (notification queue answered). S2.4b resolves the friend-invitation notification event null dereference (`nn::am::GetApplicationFunctions()` returning NULL); the boot now reaches the game's own environment init and stops at the shader translation service (translator absent). S2.4c resolves the shader-translation stop (translator carried + `shader-tools` target); the boot now translates 8 shaders and runs 300 encoded frames to the `AudioOut buffer bounds` stop. S2.5 resolves that stop: the append descriptor's `data_offset` is unset by the game (allocator fill), the host treats an offset past capacity as zero, and the boot now plays audio and runs past frame 900 with no next stop. No race, picture, or gameplay is established.

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The planned work-unit budget is 180,000 (`workBudgetTokens`, lowered from 185,000 by the explicit setup `--budget` flag; the context ceiling remains 350,000), not a hard live cutoff. `babysteps-budget` now reads `workBudgetTokens` — the key setup writes — and clamps it to the ceiling and the model limit. S2.5 ran on `opencode/deepseek-v4.1-flash-free` (200,000 context), so the verified host window, about 156,000 after the 22% reserve, was the binding number, not the configured 1,050,000 `modelContextLimit`. The verified active model for S1.3 was `opencode-go/mimo-v2.6-pro`, with 1,048,576 context and 131,072 output. Re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

S2.6 — Submit the encoded frames and observe the first game-rendered picture. S2.5 resolved the `AudioOut buffer bounds` stop: the append descriptor's `data_offset` is unset by the game (allocator fill), the host now treats an offset past capacity as zero and logs it once, and the 60 s bounded boot opens `audout:u`, appends and drains audio, and runs past frame 900 at p50 33.33 ms with no next stop. The renderer still encodes without submitting (`GPU submission disabled: work is encoded but never submitted (no picture, no GPU results); TOTK_GPU_ALLOW_SUBMIT=1 re-enables it`). Re-run the headless boot with `TOTK_GPU_ALLOW_SUBMIT=1`, record the first submitted frame and any Metal validation failure, draw error, or crash with its own message, and capture a picture through the existing diagnostic path (`TOTK_INPUT_FILE` line `shot <path>` → `dm_gfx_shot_at`). Measure first and record only what the run shows. No gameplay claim.

Gate: `python -m unittest discover -s tests` passes; every claim is tied to a recorded observation; no gameplay claim.

S2.6 closes when a submitted frame is captured, or the first submission failure is recorded with its own message and the command that produced it; visual correctness is claimed only for what the captured frame shows.

**The Do-Not list is `docs/donot.md`** — 6 entries, append-only. Never dropped, never trimmed.

## Completed

S0.1, S0.2, S0.3, S1.1, S1.2, S1.3a, S1.3b, S1.3c and S1.3 tasks are complete; the NFS loader boots through `nnMain`, but no NFS gameplay is established. The test fixture repair is complete, but it is not an NFS port task. S2.1 is complete: both window-path blockers resolved, 516 NVN function names resolved, device properties 0x00–0x5f answered. S2.2 is complete: properties 0x60/0x61 classified, first GPU frame runs. S2.2b is complete: hid IPC service registered (13 commands, Initialize through system properties). S2.3 is complete: boot traced through HID and account to the `friend:u` stop. S2.4's `friend:u` blocker is resolved: the notification queue opens and answers (`CreateFriendService` → `CreateNotificationService` → `GetEvent`); visual correctness is still unobserved. S2.4b's friend-invitation notification event blocker is resolved: the event is bound to a never-signalled host event; the boot reaches the game's own environment init and shader translation, where it stops at the absent shader translator; visual correctness is still unobserved. S2.4c's shader-translation blocker is resolved: the translator is carried (`prebuilt/` + `shader-tools`) and `make boot` builds it; the boot translates 8 shaders and runs 300 encoded frames to the `AudioOut buffer bounds` stop. S2.5's audio blocker is resolved: the unset `data_offset` is treated as zero and logged once, and the boot now plays audio, drains its queue, and runs past frame 900 with no stop; visual correctness is still unobserved.

## In progress

S2.6 — submit the encoded frames and observe the first game-rendered picture; begins after the audio stop is resolved and the boot runs with no stop; see `docs/slices/NEXT.md`.

## Queued

The roadmap below contains approved candidate work units. S2.6 is the next task; GPU submission (the first submitted frame) is its current entry blocker. The S1.3 splits are carried into S2.1/S3.2. Preflight later work against the 180,000 planned work-unit budget (the context ceiling is 350,000) and split further before work if needed. This is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **DONE**
- S0.2 — Use existing package/module analysis tools to map loading, rewrite/SDK, NVN, and service requirements; decide go/no-go for defining S1 implementation work. **DONE**
- S0.3 — Trace function-level startup and package providers on the existing NFS analysis carriers; record a precise blocker and split if no safe supported analyzer can answer. **DONE**
- S1.1 — Identify target extraction and build manifest. **DONE**

### S1 — Game-specific build and runtime bootstrap (approved)

- S1.2 — Produce a verified ARM64 rewrite. **DONE**
- S1.3a — Re-extract and decode the CNMT version. **DONE**
- S1.3b — Extract the RomFS and the six font hashes for the manifest. **DONE**
- S1.3c — Write the per-version manifest `games/nfs/assets/expected-v1.0.0.json` from the measured inputs. **DONE**
- S1.3 — Boot the loader through `nnMain`. **DONE**

### S2 — Graphics and first frame (approved)

- S2.1 — Map the required NVN surface. **DONE**
- S2.2 — Resolve NVN device properties 0x60/0x61 and continue the device init chain to the first frame. **DONE**
- S2.3 — Boot with the hid service implemented and trace to the next stop. **DONE**
- S2.4 — Verify shader and visual correctness.

### S3 — Services and playable race (approved)

- S3.1 — Implement input, storage, and saves.
- S3.2 — Implement audio, movies, and offline services.
- S3.3 — Complete a representative race and save/load.

### S4 — Packaged app and validation (approved)

- S4.1 — Package the Mac app.
- S4.2 — Validate stability, launch, save/load, and race completion.

### S5 — Post-baseline expansion (approved)

- S5.1 — Updates and DLC.
- S5.2 — Online-dependent features.
- S5.3 — Measured performance and polish.

### S0.1 — Staged NFS feasibility (2026-10-21)

- Report: `docs/nfs-port-feasibility.md`; preliminary conditional GO for a bounded Stage 1 investigation only. No boot or playability claim; no source/runtime code changed.
- Both baseline test runs passed 255 tests with one skip; each emitted a non-fatal subprocess `ResourceWarning`.
- Boundary linkage and path checks were not run because the slice changed no code, scripts, or build files. No new paid-for fault was found; Do-Not #1 remains unchanged.
- S0.1 closed with S0.2 promoted as the next evidence-only task.

### S0.2 — Static linkage, permissions, and bounded call-site evidence (2026-10-21)

- Report: `docs/nfs-port-feasibility.md`; the hactool source pin is `1d64a83450e025622f3468c28fc4164dad2c5ef6`.
- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, with a non-fatal subprocess `ResourceWarning`.
- Both boundary checks were not run (documentation-only). No new Do-Not fault was found; the current count remains 1.
- Retain `.babysteps/scratch/S0.1-NFS-feasibility/` and `.babysteps/scratch/S0.2-NFS-feasibility/` for the follow-up. S0.2 is complete; S0.3 is promoted as the next evidence-only task.

### S0.3 — Function-level startup and provider evidence (2026-10-21)

- Report: `docs/nfs-port-feasibility.md` S0.3. All four carrier SHA-256 values matched their inventories. Static function-level chain: `nnMain` → `sub_2a8120` → `sub_3d4440` → `sub_3d4620` → `sub_7683b0` → `sub_768ab0` → `sub_76e690` → `nvnBootstrapLoader("nvnDeviceGetProcAddress")`; `sub_76a220` then resolves `nvnDeviceInitialize`, `nvnQueuePresentTexture`, `nvnWindowAcquireTexture`, and `nvnWindowSetPresentInterval`.
- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, with the known non-fatal subprocess `ResourceWarning`.
- Boundary: documentation-only diff; no code or build files changed in `games/totk` or `games/by3`, and no new NFS runtime/build-time dependency on either. The report's predecessor references are evidence citations, not target dependencies.
- S0.3 splits carried: (a) indirect first-present target/runtime trace → S2.1; (b) `sub_63fea0` indirect/thread entry → S1.3/S3.2; (c) `.nss` provider identity/absence and RomFS inspection → S1.1/S1.3; (d) `nnosGet...` provider → S1.3.
- No implementation GO; no bootability or gameplay claim; no port runtime code changed. S1.1 is promoted as the next task.

### S1.1 — Extraction and build manifest (2026-10-22)

- Report: `docs/nfs-port-feasibility.md` S1.1. Reuse proven: `games/by3/scripts/extract_code.py --title 010029B0118E8000` re-extracted the NFS ExeFS in 1.2 s with SHA-256 values byte-identical to the S0.1 extraction (six files, hashes recorded in the report).
- Manifest scope recorded: `games/<game>/assets/expected-v<display>.json` with its seven required keys, the `VERSIONS`/`TITLES`/`APPS`/`GAME` wiring in `build_app.py:48-70`, and the `extracted/<version>/`, `rewritten/<version>/`, `analysis/` and log output paths `step_code` writes.
- New evidence: the meta NCA header reports SDK version 10.4.0.0; the package is base-only (one Program NCA, no BKTR). The CNMT version integer is still unread — the 304-byte section-0 payload does not begin with `CNMT` — and is split, not inferred.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, known non-fatal subprocess `ResourceWarning`. Log: `.babysteps/scratch/S1.1-NFS-manifest/gate.log`.
- Boundary: scope was the working-tree diff against HEAD. No linkage finding; two documentary path references in the report (`docs/nfs-port-feasibility.md:165,167`) and no build/runtime reach into `games/totk` or `games/by3`.
- Do-Not #2 added (whole-package SHA-256 over the multi-GB NSP). S1.2 is promoted as the next task; the CNMT version decode, the three-or-four module rewrites, RomFS extraction, the six-font input, and the `subsdk1` load-set question remain split.

### S1.2 — Verified ARM64 module rewrites (2026-10-22)

- Report: `docs/nfs-port-feasibility.md` S1.2. All five modules rewrote from the S1.1 ExeFS: rtld 4 rewrite points / 4 gates, main 7,506 / 0, sdk 11,812 / 94, subsdk0 2,963 / 0, subsdk1 10,203 / 0. Per-module NSO and `.patched.bin` SHA-256 are in the report's table and in `.babysteps/scratch/S1.2-NFS-rewrite/rewrite-results.json`.
- `cuts 0`, `obs 0` on every module: the registered D2R cuts and observation gates are keyed by build ID `db0507f36b84200ac05f75d76ed5026f` and none of the five NFS modules carries it. Each rewrite is the generic x18 / thread-pointer / SVC / counter substitution only.
- Independently checked, not just produced: `games/by3/third_party/d2r/structck.py` ends `STRUCT <module> OK … 失败项=0` for all five, with llvm-objdump as the denominator and `text改动字` equal to `改写点` on each. It ran with `LLVM_BIN=/opt/homebrew/opt/llvm@22/bin` because `llvm@23`, the path the check defaults to, is not installed. A second rewrite run into `rewritten-verify/` is byte-identical for all five.
- Scratch only: a Python 3.12 venv with capstone 5.0.9 and lz4 4.4.5 under `.babysteps/scratch/S1.2-NFS-rewrite/`. `games/by3/scripts/prepare_modules.py` is unchanged; the scratch copy differs at `:222` (module list) and nowhere else.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. Log: `.babysteps/scratch/S1.2-NFS-rewrite/gate.log`.
- Boundary: scope was the working-tree diff against HEAD (documentation-only). Linkage: no code, script, or build file changed, so no new runtime or build-time linkage into `games/totk` or `games/by3`; the report's predecessor references are citations. Paths: the report cites `games/by3/...` source paths as evidence and names the scratch copies, with no output written into either boundary tree.
- No new paid-for fault; the Do-Not count remains 2. No bootability or gameplay claim; no port runtime code changed; no game code executed. S1.2 splits: the CNMT decode (promoted to S1.3a), title-parameterized RomFS extraction, the six-font `shared_fonts_sha256` input, the `subsdk1` runtime load set, and the fact that no D2R cut applies to these modules.

### S1.3a — Re-extract and decode the NFS CNMT (2026-10-22)

- Report: `docs/nfs-port-feasibility.md` S1.3a. `version` is `0`, read from the CNMT at container 7,388,514,656 (`+0x08`, bytes `00 00 00 00`); `display_version` is `1.0.0`, read from `control.nacp` at container 7,388,070,208 (`+0x3060`, bytes `31 2e 30 2e 30 00 …`). The CNMT carries no display string — a printable-run scan of all 304 bytes is empty — so the display version is an NACP field and was read from the control NCA.
- Why S1.1/S1.2 could not answer it: the meta NCA header is AES-XTS(`header_key`) and its section body is AES-CTR, so raw container bytes contain neither `NCA0` nor `CNMT`. Decrypted, section 0 is a 512-byte CTR section whose first 32 bytes are the PFS0 hash table and whose 400-byte PFS0 starts at `+0x20`; the CNMT is byte-identical to the file S1.1 kept (`25cba08185cb6d1ea8a86b16f5b64f3a09aa6e14e86906678cb1c04280d1e1f4`).
- Cross-checks, not just a decode: the header's recorded section hash equals the fs header's SHA-256; the PFS0 hash table's SHA-256 equals the superblock `master_hash`; three of the four CNMT content hashes (control, legal, html) were re-derived from the container and match. The Program NCA was not hashed (Do-Not #2). The package's own `.cnmt.xml`/`.nacp.xml` sidecars agree with both values but are not a hash source — their Meta `<Hash>` is not the meta NCA's SHA-256.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. Log: `.babysteps/scratch/S1.3a-NFS-cnmt/gate.log`.
- Boundary: documentation-only diff. Linkage: no code, script, or build file changed, and the scratch decoders import `games/by3/scripts` (inside the boundary) at run time without changing it; no reach into `games/totk`. Paths: no output written into `games/totk` or `games/by3`.
- Do-Not #3 added (raw container bytes are not evidence of absence). No bootability or gameplay claim; no port runtime code changed; no game code executed. S1.3b is promoted: title-parameterized RomFS extraction for `romfs{size,sha256}` plus the six `shared_fonts_sha256` inputs.

### S1.3b — Extract the NFS RomFS and the six font inputs (2026-10-22)

- Report: `docs/nfs-port-feasibility.md` S1.3b. `romfs{size,sha256}` is 7,352,922,956 / `2bc692dec1bd6df0b13050474c8d7aa84450d34e2a7b4e3f52ad9f4fe827a788`, measured over exactly the image `build_app.py:step_data` checks: `romfs.bin` is the last IVFC level, `index.json['romfs_sha256']` covers those bytes, and the file's st_size equals `romfs_size`. All six IVFC levels block-verified (1/1/1/2/877/448,787 blocks); 19,169 files indexed. Extraction with streamed digest 357.96 s; a standalone `shasum -a 256 romfs.bin` re-run 14.14 s returned the same digest. Both runs explicitly sized (Do-Not #2).
- The six `shared_fonts_sha256` inputs are measured from the firmware dump (`registered/<id>.nca/00`, titles `0100000000000810`–`0100000000000814`) and recorded with bytes and offsets. Four match the reference pins in `expected-v1.4.3.json`; the two zh-cn fonts (title `0100000000000814`) differ and are valid BFTTFs — `font_report` binds a font from another firmware version by its actual SHA-256. The game RomFS contains zero font files and the NSP carries no font content, so the package cannot supply them and `step_data` requires `--firmware`/`--shared-fonts` for NSP builds.
- `games/by3/scripts/extract_shared_fonts.py` needs no game title parameter — it locates the fonts by their own title IDs and excludes meta NCAs by content type. The RomFS extractor does need one; its TOTK-hardcoded `program_nca(nca_inventory(...))` was replaced in a scratch copy by `--title 010029B0118E8000` through `open_program`/`program_header`, with the S1.1 manifest cross-checking the source and Program NCA record.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. Evidence gate 5/5 (checked image size and digest, independent digest re-run, all six IVFC levels with wall times, six font hashes with offsets, evidence record). Logs: `.babysteps/scratch/S1.3b-NFS-romfs/`.
- Boundary: scope was the slice's two new scratch scripts; the tracked tree is unchanged (the last commit is the S1.3b seed commit, not this slice's work). Linkage found, tooling only: the scratch drivers import `games/by3/scripts` (`extract_romfs_nfs.py:19-22`, `font_offsets.py:14-17`). Paths found, tooling only: `font_offsets.py:19-20` names the game package and firmware host paths. No tracked file changed; no build/runtime reach into `games/totk` or `games/by3`.
- Ledger at closeout capture (session `ses_e9d43d659ffei1v2rCK3FIdJgH`, `opencode-go/mimo-v2.6-pro`): input 172,333 / output 18,930 / reasoning 64,762 tokens, cache read 2,879,872; USD 0.1589. These are the session's totals and the slice's delta — the session ran only S1.3b.
- No new paid-for fault; the Do-Not count remains 3. No bootability or gameplay claim; no port runtime code changed; no game code executed. S1.3c is promoted: the per-version manifest `games/nfs/assets/expected-v1.0.0.json` from the measured inputs, including the font pin decision.

### S1.3c — Write the 1.0.0 manifest from the measured inputs (2026-10-22)

- `games/nfs/assets/expected-v1.0.0.json` written (commit `54f0289`) with the seven required keys plus `_comment`, every value copied from the S1.1–S1.3b recorded evidence and none re-measured: title `010029B0118E8000`, CNMT version `0`, display `1.0.0`, six ExeFS digests (S1.1), five rewrite digests (S1.2), RomFS 7,352,922,956 / `2bc692dec1bd6df0b13050474c8d7aa84450d34e2a7b4e3f52ad9f4fe827a788` (S1.3b).
- `rewritten_sha256` carries all five S1.2 modules, not the three the TOTK/BOTW manifests hold: those builds rewrite three; consumers index the table by module name (`build_app.py:536,1413`); `subsdk1` is the module the NFS load set is missing.
- Font pin policy decided: the manifest pins the reference dump, identical to `shared_fonts_sha256` in every TOTK/BOTW manifest. Justified from `font_report` (`build_app.py:414`): the pins identify the reference dump, and a valid BFTTF from another firmware version is bound by the SHA-256 of its actual bytes — so this machine's two differing zh-cn fonts (`chinese-simplified` `a04eb490283d8393b2c49c52e315132461d790ac73996cb43ac869ce72003938`, `chinese-simplified-extension` `f5784e7a553fef9a05b3b7dba94e77e79162ace9db8dafc715ace0be1fd9b9ff`) bind through that documented fallback rather than a verified table naming a dump no machine has. The measured values stay in the S1.3b record.
- `newFileRegistration` identified: the game-specific build manifest for NFS source files is the per-version manifest `games/nfs/assets/expected-v1.0.0.json` itself (S1.1 "Build manifest: the exact files and inputs"; every build step checks its inputs against it — `build_app.py:15`, `games/by3/scripts/prepare_runtime.py:7`). Its build-side load (`build_app.py:52-56,64,67` `VERSIONS`/`TITLES`/`APPS`) is the registration S1.3 performs. Nothing registered in this slice.
- Evidence gate 16/16: every manifest value appears in `docs/nfs-port-feasibility.md` S1.1–S1.3b; the seven keys are present; `shared_fonts_sha256` equals the reference pins; the two this-machine digests are recorded in the evidence and differ from the pins as decided.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. The suite proves no existing check broke; it examines no NFS data.
- Boundary: scope was the working-tree diff against HEAD — one untracked source file, `games/nfs/assets/expected-v1.0.0.json` (`localTmp/` holds harness artifacts, not source). Linkage: clear — the diff is data-only JSON, no call to follow one hop. Paths: clear — no string in the manifest names `games/totk` or `games/by3`; the file holds digests, sizes, a title ID and font names only.
- Ledger at closeout capture (session `ses_e9d0a8acfffeaoSw1Ygalv2PQ3`, `opencode-go/mimo-v2.6-pro`): input 130,321 / output 9,161 / reasoning 43,500 tokens, cache read 2,726,144; USD 0.1124. These are the session's totals and the slice's delta — the session ran only S1.3c.
- No new paid-for fault; the Do-Not count remains 3. No bootability or gameplay claim; no port runtime code changed; no game code executed. S1.3 is promoted: build the `games/nfs/` tree and the third-title wiring, map all six modules including `subsdk1`, resolve or split the `.nss`/`nnosGet…` providers, and boot through `nnMain` recording what is observed.

### S1.3 — Boot the loader through nnMain (2026-10-22)

- Report: `docs/nfs-port-feasibility.md` S1.3. `games/nfs/` built by forking `games/by3/` (runtime, `third_party/d2r`, `third_party/d2r_gfx`, scripts, Makefile; launcher and prebuilt translator not carried), and `build_app.py` wired as the third title: `VERSIONS['nfs100']`, `TITLES['010029B0118E8000']`, `APPS` entry, `prepare_work` rsync, `step_code_nfs`/`step_data_nfs` checking every manifest key. The build rebuilds NFS from `game.nsp` with every check passing in ~5 min (six ExeFS files, five rewrites, RomFS `2bc692de…`/7,352,922,956, fonts by the reference-pin/actual-bytes rule).
- Load set is all six ExeFS entries: five mapped rewrite images (rtld, main, sdk, subsdk0, subsdk1 — 141,377 relocations, zero unresolved strong imports) plus `main.npdm` bound by digest as process metadata. rtld's rewritten layout packs text and writable data into one 16 KiB host page (W^X): its page stays writable, its text is never executed and its DT_INIT is skipped — the predecessor loaders never mapped rtld, so this was new.
- Boot observed through `nnMain` (`ENTER nnMain` at `main+0x2d2fd0`): SDK startup, runtime RomFS SHA-256 verified, DT_INITs (main 207, sdk 110, subsdk0 4, subsdk1 45 init arrays), four guest threads, `nv::InitializeGraphics` served by the HLE on an Apple M4 Pro Metal device. Stopped headless at `FAIL: VI requires native macOS window`; windowed it stops in the frontend at `FAIL: native display link initialization` (`CVDisplayLinkCreateWithActiveCGDisplays` returns -6661 standalone here; deprecated since macOS 15.0; environmental vs API breakage not established). The S0.3 `nvnBootstrapLoader("nvnDeviceGetProcAddress")` endpoint was NOT reached in either run.
- Providers resolved at symbol level: no `.nss` file in the six ExeFS entries or the 19,169 decrypted RomFS paths; every strong import of every module binds inside the load set (`glslc_*` and `multimedia_*` → `sdk`; `movie::SetAllocator` → `subsdk1`); `nnosGetDefaultUserExceptionStackForApplication` is a weak import (binding 2) exported by nobody and binds zero. The DT_NEEDED filename-to-NSO identity is explicitly NOT claimed.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped, the known non-fatal `ResourceWarning`. Boundary (scope: working tree vs HEAD): linkage clear — the `totk_*.h` include names resolve to the fork's own copies (12/12 verified); paths: predecessor names appear only in provenance comments (`games/nfs/README.md:4,6`, `games/nfs/Makefile:3`, `games/nfs/scripts/prepare_runtime.py:6`, `games/nfs/scripts/merge_romfs.py:20,205`) with no runtime or build-time reach.
- Ledger at closeout capture (session `ses_e9cedb045ffev1lYBiNwvUWvRT`, `opencode-go/mimo-v2.6-pro`): input 236,618 / output 30,854 / reasoning 81,121 tokens, cache read 17,917,056; USD 0.2653. These are the session's totals and the slice's delta — the session ran only S1.3.
- New paid-for fault added: Do-Not #4 (do not blanket-rename a forked tree's comments to the new game name — they record what was measured on the predecessor). No gameplay claim; no claim beyond the recorded observations. S2.1 is promoted: map the required NVN surface, clearing the two window-path blockers first.

### S2.1 — Map the required NVN surface (2026-10-22)

- **Window-path blockers resolved.** Headless: `vi_initialize()` now creates an offscreen `CAMetalLayer` (1920×1080, the docked output size, `BGRA8Unorm`, `framebufferOnly=NO`) when no `metalLayer` exists — no native window, just the native handle the guest's NVN path needs. Windowed: three-tier vsync source (`CVDisplayLinkCreateWithActiveCGDisplays` → `CVDisplayLinkCreateWithCGDisplays(displayID)` → host timer at the screen's `maximumFramesPerSecond`, clamped 30–240 Hz); every CVReturn is logged so the only record of which path was taken survives. The timer fallback was **not exercised** — both runs had their display link up.
- **NVN surface traced through 516 name resolutions.** Both boots proceed past VI through the S0.3 endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")` (the endpoint string `0x916c3f` confirmed at runtime); lookup tracing (`hle_nvn.c` `nvn_lookup`, first-seen atomic flag per slot, `DM_NVN_LOG=1`) logs all **516 of 523** name-table entries, each once. The 7 never requested are all debug-checkpoint entries (`nvnCommandBufferInsertCheckpointStatic/Dynamic`, `nvnDeviceSetAutomaticCheckpointsEnable`/`…SamplingInterval`/`…ByDebugGroup`, `nvnDeviceSetCheckpointCallStacksEnable`, `nvnDeviceSetCheckpointMaxCallStacksDepth`); zero unknown names queried. After resolution the game calls `nvnDeviceInitialize`, `nvnQueueInitialize`, `nvnDeviceSetDepthMode`, `nvnDeviceSetWindowOriginMode`; `nvnDeviceGetInteger` answers **96 properties `0x00`–`0x5f`** (no gaps; e.g. `0x5d -> 16384`) and both runs stop at `unsupported NVN device property 0x60` — the last line of both logs (headless 676 lines, windowed 687).
- **Display link probe evidence.** `dlprobe.out` — standalone CVDisplayLink probe, macOS 27.0.0, console session, two active displays (2: 1920×1080, nominal 0.0083335 s; 4: 1314×985, 0.0166667 s): `CVDisplayLinkCreateWithActiveCGDisplays` returns 0 in all three contexts (bare, after `NSApplication`, after a window + layer), and every `CVDisplayLinkCreateWithCGDisplays` variant, `SetOutputCallback` and `Start` return 0. S1.3's -6661 (`kCVReturnInvalidArgument`, the code the create reports with no active display to bind) is environmental — consistent with the 03:40 unattended run's displays being asleep — **not** API breakage; the asleep state was not re-created directly and which of the three calls in S1.3's FAIL line returned -6661 stays unattributed (Do-Not #5).
- **Code changed, committed `8a9a8b2`** with the feasibility report: `games/nfs/runtime/frontend.m` (vsync-source chain with per-code logging, offscreen VI layer, `vsync_signal()` extraction), `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` (`nvn_lookup` first-seen resolution log).
- **Scratch evidence.** `.babysteps/scratch/S2.1-NFS-nvn-surface/`: `surface-map.json` (the claim/evidence record), `boot-headless.log`, `boot-windowed.log`, `dlprobe.m`/`dlprobe`/`dlprobe.out`. The mapped table (chain entry → how reached → runtime status → HLE status) is in `docs/nfs-port-feasibility.md` S2.1; `0x60` is `NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID` (`nvn.h:328`) and the HLE constant table carries measured values only to `0x5F`.
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped. Boundary (scope: working tree vs HEAD — `frontend.m`, `hle_nvn.c`): linkage clear (new calls are system frameworks the Makefile already links; helpers resolve in-tree), paths clear (the diff names no predecessor tree).
- **No gameplay claim.** No present, frame, or gameplay reached; the three present-side names were resolved but never called. New paid-for fault added: Do-Not #5 (one `runtime_fail` must not cover several calls). S2.2 promoted: resolve NVN device property 0x60 and continue the device init chain.

### S2.2 — Resolve NVN device properties 0x60/0x61 and continue the device init chain (2026-10-22)

- **0x60 and 0x61 classified and answered.** `g_devinfo` extended with `[0x60] = 0` (NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID — debug-only query, HLE does not implement debug groups) and `[0x61] = 0` (NVN_DEVICE_INFO_EVENTS_SUPPORT_REDUCTION_OPERATIONS — feature flag, HLE events do not implement reduction). Both return 0 so the game falls back to non-debug/non-reduced behavior. The bounds check uses `NVN_DEVINFO_COUNT` (const expression) instead of repeated division.
- **Init chain proceeds past NVN device setup.** Both boots now pass `nvnDeviceGetInteger` fully (98 properties answered, 0x00–0x61), `nvnDeviceSetDepthMode` and `nvnDeviceSetWindowOriginMode` called, window and queue initialized, first GPU frame runs (27 functions, 32 calls including `QueuePresentTexture`).
- **New stop point: IPC service not implemented.** The game halts at `FAIL: IPC service not implemented` after the `hid` (input/hid) service is queried by the SDK. Boot log: `application environment: local macOS process`, `persistent local account initialized`, `native keyboard controller initialized`, then `hid` → `FAIL`. This is a controller input gap — macOS GameController framework is wired in the runtime's HID layer. S2.2b implements `hid` for controller input.
- **Code changed, committed `dc67530`** with the feasibility report: `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` (`g_devinfo` extended to 0x61, `NVN_DEVINFO_COUNT` const).
- **Scratch evidence.** Boot log: `.babysteps/scratch/S2.2-NFS-ipc/logs/boot.log` (headless, 98 lines — NVN init complete, first frame, then `hid` fail).
- `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped. Boundary (scope: `games/nfs/` only): linkage clear (only `hle_nvn.c` changed, no calls into `games/totk/` or `games/by3/`), paths clear (self-contained HLE data change).
- **No gameplay claim.** S2.2b promoted: resolve the `hid` IPC service blocker after the first GPU frame.

### S2.2b — Implement the `hid` IPC service (controller input) (2026-10-22)

- **hid: IPC service implemented.** `games/nfs/runtime/hid_service.c` registers the `hid:` service via `runtime_ipc_register("hid:", hid_handler, NULL, NULL)` and handles 13 commands: Initialize/RegisterClient (0), GetApplicationControlUserHandle (1), GetNewestControlUserHandle (2), GetControlUserHandle (3), GetSupportedNpadStyleSet (4), IsNewestControlUserEnabled (5), GetVibrationDeviceHandles (6), GetSixAxisSensorHandle (7), GetBatteryInfo (8), IsUserAttached (9), GetNpadCounts (10), GetSystemProperties (11), GetNpadAddr (12). Vibration handles return 2 (left/right) with Switch-style handle layout; six-axis returns 0 (no Switch controllers on Mac); battery reports 100%/full; npad counts report 1 connected; system properties report docked model, en-US language, 1 user. Unknown commands still stop the runtime.
- **Code changed, committed `9be4912`**. Self-contained in `games/nfs/runtime/`; no calls into `games/totk/` or `games/by3/`.
- **Gate.** `python -m unittest discover -s tests` exited 0: 255 tests, 1 skipped. Boundary: linkage clear (only `hid_service.c` changed, includes `runtime.h` and `ipc.h` only), paths clear (self-contained).

### S2.3 — Boot through HID and trace to the next stop (2026-10-22)

- The service is registered as `hid` (no colon); only command 101 is observed in this run and it is handled. Commands 0–12 are not exercised here. The boot then creates two guest threads and stores focus policy 3.
- **Code changed, committed `bf76f17`**: `account.c` adds the local `ListOpenUsers` override; `hid_service.c` and `ipc.c` retain the observed HID and HIPC diagnostics.
- The first run stopped at `sdk+0x21a0`: `nn::account::ListOpenUsers` dereferenced a null SDK account-service holder. `nn::account::Initialize` is locally overridden and does not initialize that holder, so `ListOpenUsers` now uses the same persisted UID/open-handle state instead of falling through to the SDK.
- The rerun passed that fault and stopped at `[totk-ipc] sm: service "friend:u" not implemented`. Its call chain is `friends::NotificationQueue::Initialize` → `friends::detail::ShimLibraryGlobal::CreateNotificationService` → `friends::detail::ipc::Initialize` → `nn::sm::GetServiceHandle`. The service open fails before any `friend:u` command is observed.
- Build: `make prepare WORK=.` and `make runtime WORK=.` succeeded from the staged work folder. `python -m unittest discover -s tests` passed 255 tests, 1 skipped, with the known non-fatal subprocess `ResourceWarning`.
- Scratch logs: `.babysteps/scratch/S2.3-NFS-hid/{boot-before-account-override.log,boot-before-account-override-svc-trace.log,boot-final.log,gate-final.log}`.
- Boundary: tracked code changes are confined to `games/nfs/runtime/`; no new `games/totk/` or `games/by3/` path references. The local build mirror and generated artifacts remain untracked. Do-Not count remains 5.
- No final picture or gameplay is claimed. S2.4 is promoted with `friend:u` as its current entry blocker.

### S2.4 — Resolve the `friend:u` service open (2026-10-22)

- **`friend:u` service open resolved.** `games/nfs/runtime/friends_service.c` registers `friend:u` and answers the friend notification-queue path: `CreateFriendService` (0) returns an `IFriendService` move handle (stored, never commanded), `CreateNotificationService` (1, 16-byte userID) returns an `INotificationService` move handle, and `INotificationService` `GetEvent` (0) returns a never-signalled readable event (offline: no notifications), with `Clear` (1) and `Pop` (2) as no-ops. `IFriendService` and `CreateDaemonSuspendSessionService` (2) are report-on-call stubs — no unobserved command is implemented.
- **Next stop.** After the notification queue, four more guest threads are created, then SIGSEGV (exit 139, fault `0x0`) at `sdk+0x19d210` inside `nn::friends::GetFriendInvitationNotificationEvent` (`sdk+0x19d1b0`): it dereferences the NULL returned by `nn::am::GetApplicationFunctions()` (`_ZN2nn2am23GetApplicationFunctionsEv`). The friend-invitation notification path goes through the applet manager (`nn::am`), not `friend:u`; `applet_host.c` overrides `GetAppletResourceUserId` but not `GetApplicationFunctions`.
- **No picture, no visual claim.** GPU submission stays disabled (`TOTK_GPU_ALLOW_SUBMIT=1` re-enables); the first GPU frame is encoded but never submitted. No present, frame, audio, or gameplay.
- **Code changed** (committed with the feasibility report): `games/nfs/runtime/friends_service.c` only.
- **Scratch evidence.** `.babysteps/scratch/S2.4-NFS-friends/{boot.log,gate.log}`.
- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. Boundary: `friends_service.c` is self-contained in `games/nfs/runtime/` (includes `runtime.h` and `ipc.h` only); no reach into `games/totk` or `games/by3`. Do-Not count remains 5.
- No visual-correctness or gameplay claim. S2.4b is promoted with the `nn::am::GetApplicationFunctions()` null return as its entry blocker.

### S2.4b — Resolve the friend-invitation notification event null dereference (2026-10-22)

- **Blocker resolved.** `games/nfs/runtime/applet_host.c` overrides `nn::friends::GetFriendInvitationNotificationEvent` (`_ZN2nn7friends36GetFriendInvitationNotificationEventEv`) to bind the SDK's process-global friend-invitation `SystemEventType` (`sdk+0xcabfb0`) to a never-signalled host event (`hle_bind_system_event`) instead of letting its lazy init dereference the NULL `nn::am::GetApplicationFunctions()`. Offline there are no friend invitations, so the event never fires.
- **Resulting run.** The headless boot passes `friend:u` and the friend-invitation path (log: `[nfs] friend-invitation notification event bound, never signalled (offline)`), reaches the game's own environment init (`EnvironmentService`, `AppLifecycle`, `DirectorService` — the game's own logging), and begins shader translation. It stops at the shader translation service: neither the native translator (`build/shader-service/totk_xlate`) nor the Python script (`scripts/shader_service.py`) is carried in this tree (`totk_shader_rt.m` `service_start`), so the game's first shader request (`c10b455d…`) fails and the boot exits (code 2). The shader tooling arrives with the app-assembly slice.
- **No picture, no visual or gameplay claim.** GPU submission stays disabled; no game-rendered frame is observed.
- **Code changed**, committed `c493904`: `games/nfs/runtime/applet_host.c` only.
- **Gate RED (environmental, not this change).** `python -m unittest discover -s tests`: 255 tests, 1 skipped, 1 error — `test_work_directory.py::test_source_cache_symlinks_are_excluded_by_the_real_copier` rsyncs `games/nfs/` wholesale and `games/nfs/NFS_build/extracted/` (a stale 7 GB build dir) fills `/tmp` (`No space left on device`; the system data volume is 100% full). The S2.4 gate passed the same 255 tests when the disk had room; this change is a one-function source edit unrelated to rsync. The stale `games/nfs/NFS_build/` dir should be removed from the tree.
- **Scratch evidence.** Boot log: `.babysteps/scratch/S2.4b-NFS-friends-event/boot.log`.
- Boundary: `applet_host.c` is self-contained in `games/nfs/runtime/`; the new function calls only `runtime_module_base` (`loader.c`) and `hle_bind_system_event` (`threads.c`), no reach into `games/totk` or `games/by3`. Do-Not count remains 5.
- S2.4c is promoted with the shader translation service (absent translator) as its entry blocker.

### S2.4c — Carry the shader translator and translate a shader toward a frame (2026-10-22)

- **Shader tooling carried into `games/nfs/`.** `prebuilt/macos-arm64/{totk_xlate,totk_xlate.sha256,README.md}` is forked from `games/by3/prebuilt/` (the NFS graphics layer is byte-identical to BY3's), and the NFS `Makefile` gains `shader-tools` (copied from `games/by3/Makefile`): verify the prebuilt against its SHA-256 and copy to `<work>/build/shader-service/totk_xlate`, and build `<work>/build/spirv-cross/spirv-cross` from the pinned SPIRV-Cross commit `aa217aeb6c9f0ace7a0ab233b28807edf45eb165` with the same sources/flags as `build_app.py`. `make boot` now depends on `shader-tools`.
- **Resulting run.** The headless boot starts the native service (`native build/shader-service/totk_xlate`) and translates **8 shaders**, the first being the S2.4b failure `vs c10b455d…cf7f66b934a8`. With the shader stop gone the game runs its render loop — **300 encoded frames** (frame 1: 27 functions / 32 calls; frame 2: 120 / 4,251 with 12 `CommandBufferDrawArrays`; settled at p50 33.33 / p95 33.46 ms) — then opens `audout:u` (`AudioOut open 48000 Hz x6 PCM16 -> Core Audio (stereo)`) and stops at `[nfs] FAIL: AudioOut buffer bounds` (`audio_out.c:94` `append`).
- **No picture, no visual or gameplay claim.** `GPU submission disabled … TOTK_GPU_ALLOW_SUBMIT=1 re-enables it`; the 300 frames are NVN-encoded, not Metal-submitted.
- **Code changed**, committed: `games/nfs/prebuilt/` (new), `games/nfs/Makefile` (`shader-tools` + `boot` dependency), `games/nfs/README.md` (status).
- **Gate GREEN (environmental fix applied).** `python -m unittest discover -s tests` passed 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`. S2.4b's gate was RED because `test_work_directory.py` rsyncs `games/nfs/` wholesale and the stale 7 GB `games/nfs/NFS_build/` filled `/tmp`; that dir is now removed and temp is redirected to the external volume (`TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`; recorded in `.operator/guides/external-tmp.md`).
- **Boundary** (scope: working tree vs HEAD — `Makefile`, `README.md`, new `prebuilt/`): linkage clear (clones `github.com/KhronosGroup/SPIRV-Cross`, compiles with clang++, no call into `games/totk`/`games/by3`; the binary holds no predecessor path string), paths clear (predecessor names appear only in `prebuilt/.../README.md` provenance, the BY3 pattern). Do-Not count remains 5.
- **Scratch evidence.** Boot log: `.babysteps/scratch/S2.4c-NFS-shader-translator/boot.log` (198 lines).
- S2.5 is promoted with the `AudioOut buffer bounds` stop as its entry blocker.

### S2.5 — Resolve the `AudioOut buffer bounds` stop (2026-10-22)

- **Measured first.** The first `AppendAudioOutBuffer` is command 7 with a 0x28-byte input buffer (`na=1 nx=1`) holding `next=0xcdcdcdcdcdcdcdcd, sample_buffer=0x7b103d5000, capacity=0x1000, data_size=0xc00, data_offset=0xcdcdcdcdcdcdcdcd`. `sample_buffer`, `capacity` (4096 bytes) and `data_size` (3072 bytes) are the game's; `next` and `data_offset` are allocator fill. The SDK copies all 0x28 descriptor bytes, but this title never initialises `data_offset`, and the service accepts the descriptor.
- **Fix.** `games/nfs/runtime/audio_out.c` `append` treats an offset greater than the capacity as unset (a legitimate offset cannot exceed the buffer), logs the correction once per boot, and applies the bounds check and the copy to the normalised offset. A descriptor whose offset fits but whose data runs past the buffer still fails. `NFS_AUDIO_TRACE=1` prints the first four descriptors to stderr.
- **Resulting run.** The 60 s bounded headless boot (`.babysteps/scratch/S2.5-NFS-audio-bounds/boot3.log`, 238 lines) opens `audout:u`, appends four buffers, starts the output, and keeps playing with no queue-full (so released buffers come back and the queue drains). It reaches frame 900+ at p50 33.33 ms and translates two more shaders (`vs bb6f978b…`, `ps 34ebe360…`); **no next stop** is recorded — the timer ended the run.
- **No picture, no visual or gameplay claim.** GPU submission stays disabled; the frames are NVN-encoded, not Metal-submitted.
- **Code changed**, committed `3b11ef8`: `games/nfs/runtime/audio_out.c` only; the report gains the S2.5 section.
- **Gate GREEN.** `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0, the known non-fatal subprocess `ResourceWarning` (`.babysteps/scratch/S2.5-NFS-audio-bounds/gate.log`).
- **Boundary** (scope: this slice's diff): linkage clear (no added include, no call into `games/totk`/`games/by3`); paths clear (no absolute or predecessor path in the diff). Do-Not count is now 6.
- **Scratch evidence.** `.babysteps/scratch/S2.5-NFS-audio-bounds/{reconstruct.log,boot.log,boot3.log,rebuild.log,rebuild2.log,gate.log}`; the 7.1 GB work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and not evidence.
- S2.6 is promoted with the first submitted frame as its entry blocker.
