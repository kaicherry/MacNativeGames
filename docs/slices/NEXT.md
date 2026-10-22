---
slice: S1.3
stateFile: docs/STATE.md
donotCount: 3
seededFrom: 54f0289512b29cb3b1db033095eda884c9e3bf2e
---

# Next slice — S1.3: boot the loader through nnMain

## The next action — verbatim

S1.3 — Boot the loader through `nnMain`. Build the `games/nfs/` tree and wire the third title so NFS rebuilds from its original files like the other games: `build_app.py` `GAME`/`VERSIONS`/`TITLES`/`APPS` (`:48-70`) entries for title `010029B0118E8000` v0 consuming `games/nfs/assets/expected-v1.0.0.json`, the way the TOTK/BOTW steps consume theirs. The runtime load set maps all six ExeFS modules including `subsdk1` (`main` imports `movie::SetAllocator` from it), and the five recorded rewrite images. Resolve the boot-blocking providers first, or split them with evidence: the `DT_NEEDED` names `glslc.nss`, `multimedia.nss`, `nnSdk.nss`, and `nnosGetDefaultUserExceptionStackForApplication` (imported by `sdk`, provider unknown). Boot through `nnMain` and record exactly what is observed — module mapping, relocations, the S0.3 `nvnBootstrapLoader` chain — and what is not. Adapt or copy predecessor code into `games/nfs/`; no runtime/build-time path dependency on `games/totk/` or `games/by3/` may survive. Preflight the unit against the 350,000 work-unit ceiling and split further before work if it does not fit.

## What is settled — do not re-derive

- `games/nfs/assets/expected-v1.0.0.json` exists (commit `54f0289`), all seven keys verified 16/16 against the S1.1–S1.3b recorded evidence. The manifest file is the authority for every value; do not re-measure and do not re-derive its contents.
- `exefs_sha256` holds six files: `main`, `main.npdm`, `rtld`, `sdk`, `subsdk0`, `subsdk1`. `rewritten_sha256` holds five: `main`, `rtld`, `sdk`, `subsdk0`, `subsdk1` as `<module>.patched.bin` — deliberately more than the three TOTK/BOTW manifests hold (those builds rewrite three; consumers index by module name, `build_app.py:536,1413`).
- Font pin policy is decided: the manifest pins the **reference dump**, identical to `shared_fonts_sha256` in every TOTK/BOTW manifest. `font_report` (`build_app.py:414`) says the pins identify the reference dump and a valid BFTTF from another firmware version is bound by the SHA-256 of its actual bytes, so this machine's two differing zh-cn fonts bind through that fallback at build time. This machine's measured values, recorded in S1.3b and kept there: `chinese-simplified` `a04eb490283d8393b2c49c52e315132461d790ac73996cb43ac869ce72003938`, `chinese-simplified-extension` `f5784e7a553fef9a05b3b7dba94e77e79162ace9db8dafc715ace0be1fd9b9ff`. Do not "fix" the pins to those values.
- `newFileRegistration` is identified: the game-specific build manifest for NFS source files is the per-version manifest `games/nfs/assets/expected-v1.0.0.json` itself (every build step checks its inputs against it — `build_app.py:15`, `games/by3/scripts/prepare_runtime.py:7`). Its build-side load (`build_app.py:52-56,64,67`) is S1.3's registration to perform.
- Identity facts: `title_id` `010029B0118E8000`, CNMT `version` `0`, `display_version` `1.0.0`, Program NCA `a040a32c4a4389eb6ad4baeecb97740c.nca` (offset 736, 7,387,971,584 bytes). The five NSO build IDs (S1.2) are rtld `c1ed34fd…`, main `3515e683…`, sdk `d9add76f…`, subsdk0 `ebedb264…`, subsdk1 `84e50460…`; none equals the registered D2R build ID, so every rewrite is generic x18 / thread-pointer / SVC / counter substitution (`cuts 0`, `obs 0`).
- Static boot chain already established (S0.3): `nnMain` `0x2d2fd0` → `sub_2a8120` → `sub_3d4440` → `sub_3d4620` → `sub_7683b0` → `sub_768ab0` → `sub_76e690` → `nvnBootstrapLoader("nvnDeviceGetProcAddress")` at `0x76e6f0`; `sub_76a220` resolves `nvnDeviceInitialize`, `nvnQueuePresentTexture`, `nvnWindowAcquireTexture`, `nvnWindowSetPresentInterval`. Static reachability only — no runtime behavior was observed.
- The rewrite tools run with `DM_RULESET=native DM_PRIVATE_GATES=0` (`build_app.py:540-541`); `prepare_modules.py`'s module list must include `subsdk1`; Python `lz4` and `capstone` are needed (S1.2 used a scratch venv; no dependency entered the repository).
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No NFS implementation exists. No bootability or gameplay claim has been made.

## Withdrawn — proven wrong, do not use

- "The six shared-font inputs come from the game package." They cannot: zero font files among the 19,169 RomFS paths and no font content in the five NCAs. They are firmware-dump inputs (titles `0100000000000810`–`0814`).
- "The CNMT version integer is unknown." It is `0` (CNMT `+0x08`, container 7,388,514,656).
- `display_version` as a CNMT field: it is a NACP field (`control.nacp` `+0x3060`), value `1.0.0`.
- A whole-package SHA-256 of `game.nsp` (Do-Not #2). Identity stays per-file.
- "No `NCA0` at `0x200` means the artifact is wrong." Pre-decryption bytes (Do-Not #3).

## Code to touch

| Work | Touch |
| --- | --- |
| NFS tree | New `games/nfs/` tree (runtime, graphics layer, scripts) — copy/adapt the predecessor's; adapting is fine, a surviving path dependency is not. |
| Build wiring | `build_app.py:48-70` — `GAME`/`VERSIONS`/`TITLES`/`APPS` entries for `010029B0118E8000` v0 → `games/nfs/assets/expected-v1.0.0.json`; `step_code`/`step_data` must check the manifest's keys as they do for TOTK/BOTW. |
| Loader | The module load set: all six ExeFS modules with `subsdk1` included, binding the five recorded `rewritten_sha256` images. |
| Providers | `DT_NEEDED` `glslc.nss`, `multimedia.nss`, `nnSdk.nss`; `nnosGetDefaultUserExceptionStackForApplication` (imported by `sdk`). Resolve or split with evidence — the S0.3 candidate mapping is a candidate only. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S0.3 — Function-level startup and provider evidence", sections "Startup chain (static, function-level)" and "Providers".
- `docs/nfs-port-feasibility.md` — "S1.2 — Verified ARM64 module rewrites", section "Five modules, five recorded hashes".
- `CONTRIBUTING.md` — "How the port works, and what it must not become" and "Repository layout".
- `docs/how-it-works.md` — "The code".

## Carried evidence splits — owners

- `.nss` provider identity/absence and `nnosGetDefaultUserExceptionStackForApplication` provider → S1.3 (the slice itself).
- `subsdk1` in the runtime load set → S1.3 (the slice itself).
- Indirect first-present target and runtime trace → S2.1.
- `sub_63fea0` indirect/thread entry → S1.3/S3.2.

## Do-Not entries that apply here

- #1 — build the fixture's `.venv/bin/python` from `sys.executable`; never symlink to `sys.prefix`.
- #2 — do not hash the whole multi-GB NSP; identity stays per-file.
- #3 — do not conclude a structure is absent from raw container bytes; decrypt the layer first.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- The build's own manifest checks (`exefs_matches`, `rewritten_sha256`, `step_data` RomFS/fonts) accept `games/nfs/assets/expected-v1.0.0.json` against real inputs.
- Evidence gate: every claim is backed by a recorded observation and none extends past it.

## When this slice closes

Close when a `games/nfs/` tree wired through `build_app.py` rebuilds NFS from its original files with every manifest check passing, the loader maps all six modules (including `subsdk1`) and boots through `nnMain` with the observation recorded, the `.nss`/`nnosGet…` provider questions are resolved or split with evidence, the configured baseline passes, and no claim is made beyond what was observed. Make no gameplay claim.
