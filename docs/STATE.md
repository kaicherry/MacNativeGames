# State

## Where the work is

On branch `port/need-for-speed-hot-pursuit-remastered`, this is the initial BabySteps interview stage for a new NFS Hot Pursuit Remastered Switch port; no NFS implementation exists yet. The target is the user-owned base NSP v0 staged at `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, with ignored `prod.keys` beside it. The NSP and keys are intended local build-time inputs, not boundary paths, and must not be committed. The approved target is Apple-silicon macOS first; success is a packaged native app completing a representative race with correct picture, controls, audio, and saves. `games/totk/` and `games/by3/` are approved reference predecessor paths and rebuild boundary paths. The NFS target must not retain runtime/build-time path dependencies on either; adapting or copying code into the NFS target is not retaining a path dependency.

**The next action** is Unknown — S0.1 is complete; await operator review and a newly seeded task before any port implementation.

The previous RED baseline is closed. In `tests/test_updates.py`, the two fake-tree tests now create `.venv/bin/python` and symlink it to `sys.executable`; production code is unchanged. Both targeted update-test module runs passed (14 tests under Python 3.15 and 14 under Python 3.14). Full `python -m unittest discover -s tests` and `python3.14 -m unittest discover -s tests` each passed: 255 tests, 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; this does not make the suite red.

The 350,000 work-unit ceiling is planned, not a hard live cutoff. The verified active model was `opencode/gpt-6-luna#high`, with 1,050,000 context and a 20,000-token compaction buffer; re-verify the live session model and catalog for every slice. At slice start, capture the session ID/model and cumulative usage/cost; at closeout, report actual ledger usage and USD cost, with the slice delta separate from cumulative totals. Forecasts are informational, not the spend record.

## Next action

**S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code.**

Gate: The only expected new project artifact, `docs/nfs-port-feasibility.md`, must record attributed evidence for title/build/version scope, module layout, ARM64/rewrite fit, SDK, graphics/API and service surface, unknowns, and go/no-go; tracked files must contain no game content or secrets. The exact baseline gate passed: `python -m unittest discover -s tests` under Python 3.15 and `python3.14 -m unittest discover -s tests` under Python 3.14 each passed 255 tests, with 1 skipped. Each full run emitted a non-fatal `ResourceWarning` about a subprocess still running; the suite is not red.

**The Do-Not list is `docs/donot.md`** — 1 entry, append-only. Never dropped, never trimmed.

## Completed

No NFS port task is complete. The test fixture repair is complete, but it is not an NFS port task.

## In progress

No NFS port task is in progress.

## Queued

The following are approved candidate work units, not ready-to-execute tasks. Preflight each against the 350,000 planned work-unit ceiling and split further before work if it does not fit; this is not a host-enforced live cutoff and no automatic stop is promised.

- S0.1 — Inspect the staged NSP using the local keys and produce an evidence-based feasibility report; do not implement port code. **NEXT**
- S0.2 — Map rewrite/SDK/NVN/services and decide go/no-go.

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
- Next action is unknown pending operator review and a newly seeded task.
