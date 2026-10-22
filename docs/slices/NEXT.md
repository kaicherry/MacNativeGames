---
slice: S1.2
stateFile: docs/STATE.md
donotCount: 2
seededFrom: 18db78f4147d6e4af6d6f048702b0fc421521b92
---

# Next slice — S1.2: verified ARM64 rewrite of the NFS modules

## The next action — verbatim

S1.2 — Produce a verified ARM64 rewrite of the NFS modules. Convert all five extracted NSOs to analysis carriers with `games/by3/scripts/prepare_modules.py`, adding `subsdk1` to its default module list in the scratch copy only, then run `third_party/d2r/patch_nso.py` with `DM_RULESET=native` over `rtld`, `main`, `sdk`, `subsdk0`, `subsdk1`. Record each module's NSO SHA-256 and rewritten SHA-256. Do not implement or load a loader, do not run any game code, and make no bootability or gameplay claim.

## What is settled — do not re-derive

- S1.1 is recorded in `docs/nfs-port-feasibility.md` S1.1 and closed in `docs/STATE.md`; commit `18db78f`.
- Extraction is solved and proven: `python3 games/by3/scripts/extract_code.py <nsp> --keys <keys> --title 010029B0118E8000 --output <dir>` reproduces the S0.1 ExeFS byte for byte in about 1 s. Do not re-derive it; reuse `.babysteps/scratch/S0.1-NFS-feasibility/exefs/` or the S1.1 copy.
- The package is base-only: one Program NCA, no BKTR. `prepare_update_container` and `identify_update` do not apply.
- The ExeFS is six files: `main`, `main.npdm`, `rtld`, `sdk`, `subsdk0`, `subsdk1`; sizes and SHA-256 values are in the S1.1 report. `main.npdm` is not an NSO and is not rewritten.
- `games/totk/scripts/extract_exefs.py` and both `extract_romfs.py` copies are unusable for this title (TOTK program ID hardcoded). Only the BY3 path is title-parameterized.
- The rewriter has never been run against NFS code. `build_app.step_code` rewrites only `main`, `sdk`, `subsdk0`, so `subsdk1` and `rtld` are outside every existing rewrite list.
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No NFS implementation exists. No bootability or gameplay claim has been made.

## Withdrawn — proven wrong, do not use

- A whole-package SHA-256 of `game.nsp`: not obtainable under the shell limit (Do-Not #2). Identity is the Program NCA name/size and the per-file ExeFS hashes.
- TOTK/BY3 script paths as-is for a first extraction: they reject `010029B0118E8000`. Use the BY3 `--title` path.
- The CNMT version integer is **unknown**, not zero. Do not name a version or a `version` field without reading it.

## Code to touch

| Work | Touch |
| --- | --- |
| S1.2 rewrite | `games/by3/scripts/prepare_modules.py` behaviour, invoked from scratch; `third_party/d2r/patch_nso.py` with `DM_RULESET=native`. Scratch copy of the module list to add `subsdk1`; no production script change is required to run it. |
| Port runtime | None — no loader, no module loading, no game code. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — “S1.1 — Extraction and build manifest”, “Reusable helpers, and the exact form”, “Inputs with hashes”.
- `docs/nfs-port-feasibility.md` — “S0.1 — Staged NFS port feasibility”, “Module and platform evidence” (module list and import counts).
- `docs/how-it-works.md` — “The code” (what a rewritten module is for, and what the loader binds).
- `CONTRIBUTING.md` — “How the port works, and what it must not become”.
- `.operator/index/games/by3.md` — “Architecture”.

## Carried evidence splits — owners

- CNMT version integer and `display_version` (needed before any `expected-*.json` can exist) → the manifest slice, after S1.2.
- RomFS extraction with a title-parameterized extractor, for `romfs{size,sha256}` → the manifest slice.
- Six `shared_fonts_sha256` inputs for a package with no system update partition → the manifest slice.
- `subsdk1` in the runtime load set → S1.3 (and this slice's rewrite result).
- Indirect first-present target and runtime trace → S2.1.
- `sub_63fea0` indirect/thread entry → S1.3/S3.2.
- `.nss` provider identity/absence → S1.3.
- `nnosGetDefaultUserExceptionStackForApplication` provider → S1.3.

## Do-Not entries that apply here

- #2 — do not hash the whole multi-GB NSP.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Evidence gate: a per-module rewrite result, each either rewritten with source and output SHA-256 or refused with the exact reason and a split; no port runtime code changes.

## When this slice closes

Close when every module either rewrites with recorded hashes or has a precise blocker and split; the configured baseline passes; and no port runtime code is changed. Make no bootability or gameplay claim.
