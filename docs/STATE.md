# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, this is the initial BabySteps interview stage for a new NFS Hot Pursuit Remastered Switch port; no NFS implementation exists yet. The target is the user-owned base NSP v0 staged at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are intended local build-time inputs, not boundary paths, and must not be committed. The approved target is Apple-silicon macOS first; success is a packaged native app completing a representative race with correct picture, controls, audio, and saves. `games/totk/` and `games/by3/` are approved reference predecessor paths and rebuild boundary paths. The NFS target must not retain runtime/build-time path dependencies on either; adapting or copying code into the NFS target is not retaining a path dependency.

**The next action** is S0.3 — Use an available function-level static-analysis path on the existing hash-verified NFS analysis carriers to trace startup from `nnMain` to the first `nvnBootstrapLoader`/NVN operation and presentation, and to the earliest `nifm`/socket/friends calls; identify whether the three `DT_NEEDED` `.nss` names and `nnosGetDefaultUserExceptionStackForApplication` are provided by package modules or remain external. Distinguish linked, statically reachable, and runtime-observed facts. If no safe supported analyzer can establish a path or provider, record that exact blocker and split it; do not install tools or edit port code.

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The 350,000 work-unit ceiling is planned, not a hard live cutoff. The verified active model was `opencode/gpt-6-luna#high`, with 1,050,000 context and a 20,000-token compaction buffer; re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

S0.3 — Use an available function-level static-analysis path on the existing hash-verified NFS analysis carriers to trace startup from `nnMain` to the first `nvnBootstrapLoader`/NVN operation and presentation, and to the earliest `nifm`/socket/friends calls; identify whether the three `DT_NEEDED` `.nss` names and `nnosGetDefaultUserExceptionStackForApplication` are provided by package modules or remain external. Distinguish linked, statically reachable, and runtime-observed facts. If no safe supported analyzer can establish a path or provider, record that exact blocker and split it; do not install tools or edit port code.

Gate: `python -m unittest discover -s tests` passes; `docs/nfs-port-feasibility.md` records function-level startup call order or a precise analyzer/provider blocker and split, identifies or explicitly leaves unresolved the `.nss` and `nnosGetDefaultUserExceptionStackForApplication` providers, and distinguishes linked, permitted, statically reachable, and runtime-observed facts. No port code changes.

Close when the report traces those startup calls with function-level evidence or records a precise tool/provider blocker and splits unresolved questions; `.nss` and `nnosGetDefaultUserExceptionStackForApplication` providers are identified or explicitly unresolved; linked, permitted, statically reachable, and runtime-observed claims are distinct; the configured baseline passes; and no port code is changed. Make no bootability or gameplay claim.

**The Do-Not list is `docs/donot.md`** — 1 entry, append-only. Never dropped, never trimmed.

## Completed

S0.1 and S0.2 evidence tasks are complete; no NFS implementation is complete. The test fixture repair is complete, but it is not an NFS port task.

## In progress

No NFS port task is in progress.

## Queued

The roadmap below contains approved candidate work units. S0.3 is promoted as the next evidence-only task; preflight later work against the 350,000 planned work-unit ceiling and split further before work if needed. This is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **DONE**
- S0.2 — Use existing package/module analysis tools to map loading, rewrite/SDK, NVN, and service requirements; decide go/no-go for defining S1 implementation work. **DONE**
- S0.3 — Trace function-level startup and package providers on the existing NFS analysis carriers; record a precise blocker and split if no safe supported analyzer can answer. **NEXT**

### S1 — Game-specific build and runtime bootstrap (approved)

- S1.1 — Identify target extraction and build manifest.
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
