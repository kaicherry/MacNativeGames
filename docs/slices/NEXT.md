---
slice: S1.3b
stateFile: docs/STATE.md
donotCount: 3
seededFrom: fe640fa5ca56c41785a772da9555de62726d9526
---

# Next slice — S1.3b: the RomFS and font inputs for the NFS manifest

## The next action — verbatim

S1.3b — Extract the NFS RomFS with the title-parameterised path and record `romfs{size,sha256}`. Read what `build_app.py` hashes before extracting, then extract the Program NCA's RomFS with `--title 010029B0118E8000` through `program_header`/`open_program`, verify every IVFC level, and record the digest and byte count over exactly the image the check hashes. The image is nearly the whole 7.39 GB package: give the digest its own explicitly-sized run and record the wall time. Record the six `shared_fonts_sha256` inputs from the same package. Write nothing outside scratch and commit no game data: no `expected-*.json`, no loader, no game code, no bootability claim.

## What is settled — do not re-derive

- S1.3a is recorded in `docs/nfs-port-feasibility.md` S1.3a and closed in `docs/STATE.md`; commit `fe640fa`. `version` is `0`; `display_version` is `1.0.0`; the per-version manifest will be `games/nfs/assets/expected-v1.0.0.json`.
- The meta NCA is at container 7,388,511,456 (PFS0 entry at 7,388,510,720 plus the 736-byte data start), 3,584 bytes. Its section 0 is NCA bytes `0xC00..0xE00`; the PFS0 inside starts at section `+0x20` (past a 32-byte hash table) and is 400 bytes.
- `build_app.py` compares `romfs_sha256` to the SHA-256 of `extracted/<version>/romfs/romfs.bin` and `romfs.size` to that file's size (`build_app.py:692,700,845`). `games/by3/scripts/extract_romfs.py:59` writes `romfs.bin` as the last IVFC level and reports `romfs_size=levels[-1][1]`, `romfs_sha256` over those bytes, plus `ivfc_blocks` counts from its per-level verify. That is the exact object to reproduce.
- NCA decryption, already solved and reusable: header AES-XTS(`header_key`) over six 512-byte sectors; body AES-CTR with the key from the NCA key area and IV = 8-byte section CTR (`fs_header+0x140`) big-endian ‖ block offset >> 4. `games/by3/scripts/extract_exefs.py` (`read_at`, `decrypt`, `container_entries`, `common_tickets`, `nca_ctr_key`) and `games/by3/scripts/merge_romfs.py` (`Ctr`, `open_program`, `program_header`, `index_romfs`) already do all of it.
- Program NCA `a040a32c4a4389eb6ad4baeecb97740c.nca` at 736, 7,387,971,584 bytes. Control NCA at 7,387,972,320, 287,744. ExeFS extraction command works: `python3 games/by3/scripts/extract_code.py game.nsp --keys prod.keys --title 010029B0118E8000 --output <dir>`, ~1.2 s.
- All five modules are rewritten, hashed and structurally checked; do not re-run the rewrite. Evidence in `.babysteps/scratch/S1.2-NFS-rewrite/rewrite-results.json`.
- System `python3` (3.15) has `cryptography` 50.0.2, which is all the container helpers need. The S1.2 venv (Python 3.12, capstone, lz4) exists if a step needs `lz4`; RomFS extraction should not.
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No NFS implementation exists. No bootability or gameplay claim has been made.

## Withdrawn — proven wrong, do not use

- "The CNMT version integer is unknown." It is `0`, read at CNMT `+0x08`, container 7,388,514,656.
- "The meta NCA location is unconfirmed." It is confirmed: 7,388,511,456, size 3,584, SHA-256 `8938618b…1abf5f630`.
- "The 304-byte `Application_…cnmt` is not a CNMT." It is, byte-identical to what S1.1 kept (`25cba081…0d1e1f4`); it simply carries no `CNMT` magic.
- "No `NCA0` at `0x200` means the artifact is wrong." It means the bytes were read before decryption — see Do-Not #3.
- `display_version` as a CNMT field: the CNMT has no display string at all; read it from `control.nacp` `+0x3060`.
- The container's `.cnmt.xml`/`.nacp.xml` as a hash source: their Meta `<Hash>` is not the meta NCA's SHA-256. They agree on the two version values and can corroborate, nothing more.
- A whole-package SHA-256 of `game.nsp` (Do-Not #2). Identity stays per-file.

## Code to touch

| Work | Touch |
| --- | --- |
| RomFS extraction | Scratch only. `games/by3/scripts/extract_romfs.py` hardcodes the TOTK title (`program_nca(nca_inventory(...))`), so use a scratch copy that takes `--title 010029B0118E8000`, or drive `open_program` + `Ctr` + `index_romfs` directly. Output `romfs.bin` and `index.json` under scratch. |
| Font hashes | `games/by3/scripts/extract_shared_fonts.py` already skips `.cnmt.nca`; check whether it too needs a title parameter before using it. Six fonts, hashed. |
| Port runtime | None — no loader, no manifest file, no `expected-*.json`, no game code. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S1.1 — Extraction and build manifest", sections "Build manifest: the exact files and inputs" and "Reusable helpers, and the exact form".
- `docs/nfs-port-feasibility.md` — "S1.3a — The CNMT version and the NACP display version", sections "The meta NCA, decoded" and "Closeout".
- `CONTRIBUTING.md` — "How the port works, and what it must not become".

## Carried evidence splits — owners

- The per-version manifest `expected-v1.0.0.json` itself, once RomFS and font hashes exist → the manifest slice after this one.
- `subsdk1` in the runtime load set → S1.3.
- `.nss` provider identity/absence and `nnosGetDefaultUserExceptionStackForApplication` provider → S1.3.
- Indirect first-present target and runtime trace → S2.1.
- `sub_63fea0` indirect/thread entry → S1.3/S3.2.

## Do-Not entries that apply here

- #2 — do not hash the whole multi-GB NSP.
- #3 — do not conclude a structure is absent from raw container bytes; decrypt the layer first.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Evidence gate: `romfs.size` and `romfs.sha256` recorded from an extracted image with every IVFC level verified, the digest's wall time recorded, and six font hashes recorded; no port runtime code changes.

## When this slice closes

Close when both manifest inputs are measured with recorded bytes and offsets, or a precise blocker names the reason and a split; the configured baseline passes; and no port runtime code is changed. Make no bootability or gameplay claim.
