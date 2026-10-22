# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, this is the initial BabySteps interview stage for a new NFS Hot Pursuit Remastered Switch port; no NFS implementation exists yet. The target is the user-owned base NSP v0 staged at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are intended local build-time inputs, not boundary paths, and must not be committed. The approved target is Apple-silicon macOS first; success is a packaged native app completing a representative race with correct picture, controls, audio, and saves. `games/totk/` and `games/by3/` are approved reference predecessor paths and rebuild boundary paths. The NFS target must not retain runtime/build-time path dependencies on either; adapting or copying code into the NFS target is not retaining a path dependency.

**The next action** is S1.3b — Extract the NFS RomFS with the title-parameterised path and record `romfs{size,sha256}`. `build_app.py` compares `romfs_sha256` against the digest of `extracted/<version>/romfs/romfs.bin` and `romfs.size` against that file's size, and `games/by3/scripts/extract_romfs.py` writes `romfs.bin` as the last IVFC level with `romfs_size=levels[-1][1]`; that copy hardcodes the TOTK title, so drive the extraction with `--title 010029B0118E8000` through `program_header`/`open_program` and verify every IVFC level. The Program NCA's RomFS is nearly the whole 7.39 GB package, so the digest needs its own explicitly-sized run (Do-Not #2 is about the whole NSP, not this image, but the same 120 s window applies). Record the six `shared_fonts_sha256` inputs in the same pass — a package with no system-update partition has nowhere else to take them. Evidence only: no `expected-*.json`, no loader, no game code, no bootability claim. Carry the S1.2 and S1.3a splits: `subsdk1` in the runtime load set, the `.nss` and `nnosGet…` providers, and the indirect first-present targets.

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The 350,000 work-unit ceiling is planned, not a hard live cutoff. The verified active model for S1.3a was `opencode-go/deepseek-v4.1-flash#high`, with 1,000,000 context and 384,000 output; the configured `modelContextLimit` of 1,050,000 is above the host's 1,000,000, so the host limit is the real cap. Re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

S1.3b — Extract the NFS RomFS with the title-parameterised path and record `romfs{size,sha256}`. Read what `build_app.py` hashes before extracting, then extract the Program NCA's RomFS with `--title 010029B0118E8000` through `program_header`/`open_program`, verify every IVFC level, and record the digest and byte count over exactly the image the check hashes. The image is nearly the whole 7.39 GB package: give the digest its own explicitly-sized run and record the wall time. Record the six `shared_fonts_sha256` inputs from the same package. Write nothing outside scratch and commit no game data: no `expected-*.json`, no loader, no game code, no bootability claim.

Gate: `python -m unittest discover -s tests` passes; `romfs.size` and `romfs.sha256` are recorded from an extracted image with every IVFC level verified, and six font hashes are recorded; no port runtime code changes.

Close when both manifest inputs are measured with recorded bytes and offsets, or a precise blocker names the reason and a split; the configured baseline passes; and no port runtime code is changed. Make no bootability or gameplay claim.

**The Do-Not list is `docs/donot.md`** — 3 entries, append-only. Never dropped, never trimmed.

## Completed

S0.1, S0.2, S0.3, S1.1, S1.2 and S1.3a evidence tasks are complete; no NFS implementation is complete. The test fixture repair is complete, but it is not an NFS port task.

## In progress

No NFS port task is in progress.

## Queued

The roadmap below contains approved candidate work units. S1.3b is promoted as the next task; the remaining S1.1, S1.2 and S1.3a evidence splits are carried into S1.3b/S1.3/S2.1. Preflight later work against the 350,000 planned work-unit ceiling and split further before work if needed. This is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **DONE**
- S0.2 — Use existing package/module analysis tools to map loading, rewrite/SDK, NVN, and service requirements; decide go/no-go for defining S1 implementation work. **DONE**
- S0.3 — Trace function-level startup and package providers on the existing NFS analysis carriers; record a precise blocker and split if no safe supported analyzer can answer. **DONE**
- S1.1 — Identify target extraction and build manifest. **DONE**

### S1 — Game-specific build and runtime bootstrap (approved)

- S1.2 — Produce a verified ARM64 rewrite. **DONE**
- S1.3a — Re-extract and decode the CNMT version. **DONE**
- S1.3b — Extract the RomFS and the six font hashes for the manifest. **NEXT**
- S1.3 — Boot the loader through `nnMain`.

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
