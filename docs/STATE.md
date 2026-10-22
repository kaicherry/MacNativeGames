# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, this is the initial BabySteps interview stage for a new NFS Hot Pursuit Remastered Switch port; no NFS implementation exists yet. The target is the user-owned base NSP v0 staged at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are intended local build-time inputs, not boundary paths, and must not be committed. The approved target is Apple-silicon macOS first; success is a packaged native app completing a representative race with correct picture, controls, audio, and saves. `games/totk/` and `games/by3/` are approved reference predecessor paths and rebuild boundary paths. The NFS target must not retain runtime/build-time path dependencies on either; adapting or copying code into the NFS target is not retaining a path dependency.

**The next action** is S1.3 — Boot the loader through `nnMain`: build the `games/nfs/` tree and the `build_app.py` third-title wiring so NFS rebuilds from its original files against `games/nfs/assets/expected-v1.0.0.json`, map all six ExeFS modules including `subsdk1`, resolve or split the `.nss` and `nnosGet…` providers, and record exactly what the boot observes. Preflight the unit against the 350,000 ceiling and split further before work if it does not fit. Carry the remaining splits: the indirect first-present target (S2.1) and the `sub_63fea0` entry path (S1.3/S3.2).

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The 350,000 work-unit ceiling is planned, not a hard live cutoff. The verified active model for S1.3c was `opencode-go/mimo-v2.6-pro`, with 1,048,576 context and 131,072 output; the configured `modelContextLimit` of 1,050,000 is again above the host's real limit, so the host limit is the real cap. Re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

S1.3 — Boot the loader through `nnMain`. Build the `games/nfs/` tree and wire the third title so NFS rebuilds from its original files like the other games: `build_app.py` `GAME`/`VERSIONS`/`TITLES`/`APPS` (`:48-70`) entries for title `010029B0118E8000` v0 consuming `games/nfs/assets/expected-v1.0.0.json`, the way the TOTK/BOTW steps consume theirs. The runtime load set maps all six ExeFS modules including `subsdk1` (`main` imports `movie::SetAllocator` from it), and the five recorded rewrite images. Resolve the boot-blocking providers first, or split them with evidence: the `DT_NEEDED` names `glslc.nss`, `multimedia.nss`, `nnSdk.nss`, and `nnosGetDefaultUserExceptionStackForApplication` (imported by `sdk`, provider unknown). Boot through `nnMain` and record exactly what is observed — module mapping, relocations, the S0.3 `nvnBootstrapLoader` chain — and what is not. Adapt or copy predecessor code into `games/nfs/`; no runtime/build-time path dependency on `games/totk/` or `games/by3/` may survive. Preflight the unit against the 350,000 work-unit ceiling and split further before work if it does not fit.

Gate: `python -m unittest discover -s tests` passes; the build's own manifest checks (`exefs_matches`, `rewritten_sha256`, `step_data` RomFS/fonts) accept `games/nfs/assets/expected-v1.0.0.json` against real inputs; every claim is backed by a recorded observation and none extends past it.

Close when a `games/nfs/` tree wired through `build_app.py` rebuilds NFS from its original files with every manifest check passing, the loader maps all six modules (including `subsdk1`) and boots through `nnMain` with the observation recorded, the `.nss`/`nnosGet…` provider questions are resolved or split with evidence, the configured baseline passes, and no claim is made beyond what was observed. Make no gameplay claim.

**The Do-Not list is `docs/donot.md`** — 3 entries, append-only. Never dropped, never trimmed.

## Completed

S0.1, S0.2, S0.3, S1.1, S1.2, S1.3a, S1.3b and S1.3c tasks are complete; no NFS implementation is complete. The test fixture repair is complete, but it is not an NFS port task.

## In progress

No NFS port task is in progress.

## Queued

The roadmap below contains approved candidate work units. S1.3 is promoted as the next task; the remaining S1.1–S1.3c evidence splits are carried into S1.3/S2.1/S3.2. Preflight later work against the 350,000 planned work-unit ceiling and split further before work if needed. This is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **DONE**
- S0.2 — Use existing package/module analysis tools to map loading, rewrite/SDK, NVN, and service requirements; decide go/no-go for defining S1 implementation work. **DONE**
- S0.3 — Trace function-level startup and package providers on the existing NFS analysis carriers; record a precise blocker and split if no safe supported analyzer can answer. **DONE**
- S1.1 — Identify target extraction and build manifest. **DONE**

### S1 — Game-specific build and runtime bootstrap (approved)

- S1.2 — Produce a verified ARM64 rewrite. **DONE**
- S1.3a — Re-extract and decode the CNMT version. **DONE**
- S1.3b — Extract the RomFS and the six font hashes for the manifest. **DONE**
- S1.3c — Write the per-version manifest `games/nfs/assets/expected-v1.0.0.json` from the measured inputs. **DONE**
- S1.3 — Boot the loader through `nnMain`. **NEXT**

### S2 — Graphics and first frame (approved)

- S2.1 — Map the required NVN surface.
- S2.2 — Implement missing graphics calls through the first frame.
- S2.3 — Verify shader and visual correctness.

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
