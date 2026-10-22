---
slice: S1.3c
stateFile: docs/STATE.md
donotCount: 3
seededFrom: 1a752e0e0fffed7ca7f435d92d4972270d80c82f
---

# Next slice — S1.3c: the per-version manifest from the measured inputs

## The next action — verbatim

S1.3c — Write the per-version NFS manifest `games/nfs/assets/expected-v1.0.0.json` from the measured inputs. Read the exact key shapes from `games/totk/assets/expected-v1.4.3.json` and from the checks that consume them in `build_app.py` before writing: `title_id`, `version`, `display_version`, `exefs_sha256`, `rewritten_sha256`, `romfs{size,sha256}`, `shared_fonts_sha256`. Every value is already measured — take them from `docs/nfs-port-feasibility.md` S1.1/S1.2/S1.3a/S1.3b and do not re-measure. Decide and record the font pin policy: this machine's firmware dump (two zh-cn fonts differ from the reference values every TOTK/BOTW manifest pins) or the reference values, justified from `font_report` semantics. Identify the exact manifest `newFileRegistration` means for NFS source files and record it without registering anything. Data only: no loader, no game code, no `build_app.py` wiring beyond what the manifest check needs, no bootability claim.

## What is settled — do not re-derive

- S1.3b is recorded in `docs/nfs-port-feasibility.md` S1.3b and closed in `docs/STATE.md`; commit `1a752e0`. `romfs{size,sha256}` = 7,352,922,956 / `2bc692dec1bd6df0b13050474c8d7aa84450d34e2a7b4e3f52ad9f4fe827a788`, measured over exactly `romfs.bin` (the last IVFC level; `index.json['romfs_sha256']` covers those bytes, the file's st_size equals `romfs_size`). All six IVFC levels verified. The report table is the authority for every value below.
- The six `shared_fonts_sha256` values as measured from this machine's firmware dump (bytes and offsets in the S1.3b report): `standard` `823913c9d80807979381e327846466f561f31ba1842cea8f7fedc5ad7d45d2ad`, `chinese-simplified` `a04eb490283d8393b2c49c52e315132461d790ac73996cb43ac869ce72003938`, `chinese-simplified-extension` `f5784e7a553fef9a05b3b7dba94e77e79162ace9db8dafc715ace0be1fd9b9ff`, `chinese-traditional` `9d59b74e59a2dc92296d6bebad7cce791ba94688bd436304aaae174a58bd7457`, `korean` `5a4e694782ead5ddcbc0a226aab24362e2d641ca33bac7bdf8cda36ff157c9e8`, `nintendo-extension` `f9f62e025c9bbe0e9ae25a54ba5011202b1e3a3f3c33b2d876c40aff6174af68`.
- Four of those match the reference pins in `expected-v1.4.3.json` (standard, chinese-traditional, korean, nintendo-extension). The two zh-cn fonts (both title `0100000000000814`) differ — this dump is another firmware version for that title, and both are valid BFTTFs. `build_app.py:font_report` binds a valid font from another firmware version by the SHA-256 of its actual bytes, so either pin is workable; the choice must be recorded and justified.
- The game package cannot supply fonts: zero `.bfttf`/`.ttf`/`.otf` among the 19,169 RomFS paths and no font content among the five NCAs. `build_app.py:step_data` requires `--firmware` or `--shared-fonts` for NSP builds.
- The manifest filename follows the display string: `expected-v1.0.0.json`. `version` is the CNMT integer `0`; `display_version` is `1.0.0`; `title_id` is `010029B0118E8000`.
- The other two value sets already exist: ExeFS per-file hashes (six files) in `docs/nfs-port-feasibility.md` S1.1; module NSO and `.patched.bin` hashes in S1.2 and `.babysteps/scratch/S1.2-NFS-rewrite/rewrite-results.json`. Program NCA identity: `a040a32c4a4389eb6ad4baeecb97740c.nca`, offset 736, 7,387,971,584 bytes.
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No NFS implementation exists. No bootability or gameplay claim has been made.

## Withdrawn — proven wrong, do not use

- "The six shared-font inputs come from the game package." They cannot: the game RomFS holds zero font files and the NSP carries no font content. They are firmware-dump inputs (`registered/<id>.nca/00`), as `step_data`'s `--firmware`/`--shared-fonts` requirement implies.
- "The CNMT version integer is unknown." It is `0` (CNMT `+0x08`, container 7,388,514,656).
- `display_version` as a CNMT field: it is a NACP field (`control.nacp` `+0x3060`), value `1.0.0`.
- A whole-package SHA-256 of `game.nsp` (Do-Not #2). Identity stays per-file.
- "No `NCA0` at `0x200` means the artifact is wrong." Pre-decryption bytes (Do-Not #3).

## Code to touch

| Work | Touch |
| --- | --- |
| Manifest | New `games/nfs/assets/expected-v1.0.0.json` (the `games/nfs/` tree does not exist yet). Key shapes copy `games/totk/assets/expected-v1.4.3.json`; values come from the S1.1–S1.3b reports, not from re-measurement. |
| Font pin | A recorded decision with its justification: the measured dump values or the reference values. `build_app.py:font_report` (`:414`) is the semantics to justify against. |
| Registration | Identify the manifest `newFileRegistration` means for NFS source files and record it; register nothing yet. |
| Port runtime | None — no loader, no game code. `build_app.py` third-title wiring (`:48-70` `GAME`/`VERSIONS`/`TITLES`/`APPS`) only if the manifest check cannot load the file without it; otherwise it stays S1.3's. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S1.1 — Extraction and build manifest", section "Build manifest: the exact files and inputs".
- `docs/nfs-port-feasibility.md` — "S1.3b — The RomFS digest and the six shared-font inputs", sections "The six shared-font inputs" and "Closeout".
- `CONTRIBUTING.md` — "How the port works, and what it must not become".

## Carried evidence splits — owners

- `subsdk1` in the runtime load set → S1.3.
- `.nss` provider identity/absence and `nnosGetDefaultUserExceptionStackForApplication` provider → S1.3.
- Indirect first-present target and runtime trace → S2.1.
- `sub_63fea0` indirect/thread entry → S1.3/S3.2.
- `build_app.py` third-title wiring beyond manifest loading → S1.3.

## Do-Not entries that apply here

- #1 — build the fixture's `.venv/bin/python` from `sys.executable`; never symlink to `sys.prefix`.
- #2 — do not hash the whole multi-GB NSP; identity stays per-file.
- #3 — do not conclude a structure is absent from raw container bytes; decrypt the layer first.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Evidence gate: every manifest value matches the S1.1–S1.3b recorded evidence; the font pin decision is recorded with its justification; no port runtime code changes.

## When this slice closes

Close when `games/nfs/assets/expected-v1.0.0.json` exists with all seven required keys whose values match the recorded evidence, the font pin policy is decided and recorded, the configured baseline passes, and no port runtime code is changed. Make no bootability or gameplay claim.
