---
slice: S1.3a
stateFile: docs/STATE.md
donotCount: 2
seededFrom: 3037f65ac209a1ccd8ed03c99a2e971302cc6276
---

# Next slice — S1.3a: re-extract and decode the NFS CNMT

## The next action — verbatim

S1.3a — Re-extract and decode the NFS CNMT. Find the meta NCA in the package, re-extract its section 0 at the right offset and size (decompressing when the NCA header says the section is compressed), and read `version` (integer) and `display_version` from the CNMT header. Do not infer either value from SDK strings, title IDs, or the ExeFS. Record the container offset, the section offset/size, and the exact bytes carrying each field. Write nothing outside scratch: no `expected-*.json`, no RomFS extraction, no loader, no game code, no bootability claim.

## What is settled — do not re-derive

- S1.2 is recorded in `docs/nfs-port-feasibility.md` S1.2 and closed in `docs/STATE.md`; commit `3037f65`.
- All five modules are rewritten and hashed: carriers in `.babysteps/scratch/S1.2-NFS-rewrite/elf/`, rewrites in `.../rewritten/`, hashes and per-module counts in `.../rewrite-results.json`. Do not re-run the rewrite unless `patch_nso.py` changes.
- `cuts 0` and `obs 0` on all five modules is measured, because the D2R tables are keyed by build ID `db0507f36b84200ac05f75d76ed5026f` and none of the five NFS modules carries it.
- `structck.py` passed all five (`STRUCT <module> OK … 失败项=0`). It needs `LLVM_BIN=/opt/homebrew/opt/llvm@22/bin`; `llvm@23`, the path it defaults to, is not installed.
- A working lz4/capstone environment already exists: `.babysteps/scratch/S1.2-NFS-rewrite/venv` (Python 3.12, capstone 5.0.9, lz4 4.4.5). Reuse it; the system interpreter has neither package.
- Extraction of the ExeFS is solved: `python3 games/by3/scripts/extract_code.py <nsp> --keys <keys> --title 010029B0118E8000 --output <dir>`, about 1 s. The ExeFS copy is `.babysteps/scratch/S1.1-NFS-manifest/exefs/`.
- The package is base-only (one Program NCA, no BKTR). S1.1 recorded the meta NCA at offset 7,388,511,456, size 3,584 — treat that location as unconfirmed; see Withdrawn.
- Module NSO build IDs from S1.2: rtld `c1ed34fd…`, main `3515e683…`, sdk `d9add76f…`, subsdk0 `ebedb264…`, subsdk1 `84e50460…`.
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No NFS implementation exists. No bootability or gameplay claim has been made.

## Withdrawn — proven wrong, do not use

- A whole-package SHA-256 of `game.nsp`: not obtainable under the shell limit (Do-Not #2). Identity is the Program NCA name/size plus per-file ExeFS hashes.
- The CNMT version integer: **unknown, not zero**. Do not name a version or a `version` field without reading it.
- `.babysteps/scratch/S1.1-NFS-manifest/meta/pfs0/Application_010029b0118e8000.cnmt` as a CNMT. Measured: no `CNMT` magic (it begins with a title-ID-like `00 80 8e 11 b0 29 00 01`), and those 304 bytes occur nowhere inside `meta/cnmt.nca` (`find == -1`).
- `.babysteps/scratch/S1.1-NFS-manifest/meta/cnmt.nca` as a usable meta NCA. Measured: 3,584 bytes, no `NCA0` magic at `0x200`. Re-locate before decoding.
- TOTK/BY3 script paths as-is for extraction: they reject `010029B0118E8000`. Only the BY3 `--title` path works.

## Code to touch

| Work | Touch |
| --- | --- |
| CNMT decode | Scratch only. Reuse the BY3 title-parameterized path (`games/by3/scripts/extract_code.py`, `merge_romfs.py` `program_header`) to locate the meta NCA; the pinned hactool at `.babysteps/scratch/S0.2-NFS-feasibility/work/src/hactool/hactool` is available for header/section dumps. |
| Port runtime | None — no loader, no manifest file, no `expected-*.json`, no game code. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — “S1.2 — Verified ARM64 module rewrites”, section “The S1.1 meta artifact holds no CNMT”.
- `docs/nfs-port-feasibility.md` — “S1.1 — Extraction and build manifest”, sections “Inputs with hashes” and “Staged package contents”.
- `docs/nfs-port-feasibility.md` — “S1.1 — Extraction and build manifest”, section “Build manifest: the exact files and inputs” (why `version` must be an integer, not a display string).
- `CONTRIBUTING.md` — “How the port works, and what it must not become”.

## Carried evidence splits — owners

- RomFS extraction with a title-parameterized extractor, for `romfs{size,sha256}` → the manifest slice, after the version is read.
- Six `shared_fonts_sha256` inputs for a package with no system update partition → the manifest slice.
- `subsdk1` in the runtime load set → S1.3.
- `.nss` provider identity/absence and `nnosGetDefaultUserExceptionStackForApplication` provider → S1.3.
- Indirect first-present target and runtime trace → S2.1.
- `sub_63fea0` indirect/thread entry → S1.3/S3.2.

## Do-Not entries that apply here

- #2 — do not hash the whole multi-GB NSP.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Evidence gate: `version` and `display_version` read from CNMT bytes with the container offset, section offset/size, and the carrying bytes recorded; no port runtime code changes.

## When this slice closes

Close when both values are read from the container with recorded bytes and offsets, or a precise blocker names the reason and a split; the configured baseline passes; and no port runtime code is changed. Make no bootability or gameplay claim.
