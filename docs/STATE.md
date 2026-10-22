# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, this is the initial BabySteps interview stage for a new NFS Hot Pursuit Remastered Switch port; no NFS implementation exists yet. The target is the user-owned base NSP v0 staged at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are intended local build-time inputs, not boundary paths, and must not be committed. The approved target is Apple-silicon macOS first; success is a packaged native app completing a representative race with correct picture, controls, audio, and saves. `games/totk/` and `games/by3/` are approved reference predecessor paths and rebuild boundary paths. The NFS target must not retain runtime/build-time path dependencies on either; adapting or copying code into the NFS target is not retaining a path dependency.

**The next action** is S1.2 — Produce a verified ARM64 rewrite of the NFS modules. Run the analysis-carrier conversion (`games/by3/scripts/prepare_modules.py`, whose default list omits `subsdk1`) and then the rewriter (`third_party/d2r/patch_nso.py`, `DM_RULESET=native`) over `rtld`, `main`, `sdk`, `subsdk0`, `subsdk1` in scratch, and record per-module NSO and rewritten SHA-256 values. This is a rewrite and static-verification step only: no loader, no module loading, no runtime code, no bootability claim. Carry the S1.1 splits: the CNMT version is still unread, and `subsdk1` is outside the current rewrite set.

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The 350,000 work-unit ceiling is planned, not a hard live cutoff. The verified active model was `opencode/gpt-6-luna#high`, with 1,050,000 context and a 20,000-token compaction buffer; re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

S1.2 — Produce a verified ARM64 rewrite of the NFS modules. Convert all five extracted NSOs to analysis carriers with `games/by3/scripts/prepare_modules.py`, adding `subsdk1` to its default module list in the scratch copy only, then run `third_party/d2r/patch_nso.py` with `DM_RULESET=native` over `rtld`, `main`, `sdk`, `subsdk0`, `subsdk1`. Record each module's NSO SHA-256 and rewritten SHA-256. Do not implement or load a loader, do not run any game code, and make no bootability or gameplay claim.

Gate: `python -m unittest discover -s tests` passes; the rewrite step records a per-module result (rewritten or a precise refusal naming the reason and rule), with source and output hashes. No port runtime code changes.

Close when every module either rewrites with recorded hashes or has a precise blocker and split; the configured baseline passes; and no port runtime code is changed. Make no bootability or gameplay claim.

**The Do-Not list is `docs/donot.md`** — 2 entries, append-only. Never dropped, never trimmed.

## Completed

S0.1, S0.2, S0.3, and S1.1 evidence tasks are complete; no NFS implementation is complete. The test fixture repair is complete, but it is not an NFS port task.

## In progress

No NFS port task is in progress.

## Queued

The roadmap below contains approved candidate work units. S1.2 is promoted as the next task; the S0.3 and S1.1 evidence splits are carried into S1.2/S1.3/S2.1/S3.2 and the manifest work. Preflight later work against the 350,000 planned work-unit ceiling and split further before work if needed. This is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **DONE**
- S0.2 — Use existing package/module analysis tools to map loading, rewrite/SDK, NVN, and service requirements; decide go/no-go for defining S1 implementation work. **DONE**
- S0.3 — Trace function-level startup and package providers on the existing NFS analysis carriers; record a precise blocker and split if no safe supported analyzer can answer. **DONE**
- S1.1 — Identify target extraction and build manifest. **DONE**

### S1 — Game-specific build and runtime bootstrap (approved)

- S1.2 — Produce a verified ARM64 rewrite. **NEXT**
- S1.2 — Produce a verified ARM64 rewrite.
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
