# S0.1 — Staged NFS port feasibility

## Scope and verdict

This is a read-only feasibility inspection of the locally staged package at
`target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, using its adjacent local `prod.keys`.
No key values or game payloads are included here; no source or runtime code was changed.

**Preliminary verdict: conditional GO for a bounded Stage 1 investigation, not a claim of bootability or playability.** The package exposes a native ARM64 program and an NVN bootstrap symbol, which fit the project's native-code/NVN-to-Metal approach. The existing build path does not recognize this title, and the module and service dependencies need further work before a boot can be predicted.

## Package and key evidence

- The 7,388,586,538-byte input parsed as a PFS0/NSP with 11 entries, five NCA files, and one common ticket.
- The existing NCA-header parser accepted all five headers as NCA3, checked declared NCA sizes and section bounds, and derived a content key for each using the local keys and ticket. All five NCAs report title ID `010029B0118E8000`.
- The Program NCA is 7,387,971,584 bytes. Its `main.npdm` program ID is also `010029B0118E8000`.
- A Meta NCA is present, but CNMT/NACP version fields were not decoded. The internal release/build version is unknown; the staging path and title ID are not treated as version evidence.
- The ExeFS contains `main` (6,906,316 bytes), `rtld` (7,223), `sdk` (5,619,621), `subsdk0` (4,501,008), `subsdk1` (3,397,429), and `main.npdm` (1,540).
- The ExeFS FS header, hash table, and every data block verified. The extractor explicitly does **not** check NCA RSA signatures. RomFS/IVFC content was not extracted or verified.
- A full NSP SHA-256 was not obtained: the 7.39 GB hash command exceeded its 120-second limit. No whole-package digest is claimed.

## Module and platform evidence

The existing analysis-only NSO converter produced ARM64 ELF carriers for all five NSOs. Its segment checks passed; the Python LZ4 module was absent, so this one-off run used the already-installed system `liblz4` through an in-memory wrapper. No dependency or source file was added.

- `main` has 461 dynamic imports and 23 exports. Its `DT_NEEDED` names include `glslc.nss`, `multimedia.nss`, and `nnSdk.nss`; its imports include `nvnBootstrapLoader` and APIs from `nn::nifm`, `nn::socket`, and `nn::friends`.
- The `sdk` module is present and exposes 26,221 symbols in the analysis carrier, but its Nintendo SDK revision/version is unknown.
- Name matching between module import/export tables found 456 `main` imports provided by `sdk` and one `main` import, `movie::SetAllocator`, provided by `subsdk1`. This is static symbol evidence, not a runtime call trace.
- `sdk` contains 61 SVC instruction-pattern candidates. The converter warns that byte-pattern matches are candidates until control flow is analyzed; they are not evidence that those SVCs execute.

The repository's documented runtime maps `main`, `sdk`, and `subsdk0`; this package also has `subsdk1`, and `main` imports a symbol found in that module. The extra module therefore needs explicit loader/linkage investigation. The network/friends imports establish linked dependencies, not that offline startup calls them. The NFS-specific `.nss` dependencies likewise remain unresolved.

## Fit against the existing port

- The project runs native ARM64 game code and rebuilds NVN graphics on Metal (`CONTRIBUTING.md`, “How the port works, and what it must not become”; `docs/how-it-works.md`, “The code” and “The graphics”). The observed ARM64 modules and `nvnBootstrapLoader` are positive compatibility signals, not proof of rendering.
- The root builder recognizes only the TOTK and BOTW title IDs (`build_app.py:64,1377-1388`); the existing TOTK NSP extractor also hard-codes the TOTK program ID (`games/totk/scripts/extract_exefs.py:160-178`). Neither is a drop-in NFS build path.
- The project keeps other game ports in separate `games/<name>/` trees (`CONTRIBUTING.md`, “Repository layout”). BY3's documented service gaps are precedent only, not proof of which services NFS needs during startup (`games/by3/CHANGELOG.md:42-44`).

## Next evidence needed

A Stage 1 investigation should establish NFS module-load requirements (especially `subsdk1` and the `.nss` dependencies), decode the relevant service permissions, and trace which network/friends and NVN APIs are reached before the first presentation. Until then, offline boot, service sufficiency, full RomFS integrity, and gameplay remain unknown. This report does not authorize or implement port code.

## Source references

- `games/totk/scripts/extract_exefs.py:69-157,160-178,186-246` — PFS0, ticket/key, NCA inventory, title selection, and ExeFS verification.
- `games/by3/scripts/prepare_modules.py:25-212,215-230` — NSO analysis-carrier conversion and its default module list.
- `build_app.py:64,1377-1388` — supported title selection.
- `CONTRIBUTING.md:15-26`; `docs/how-it-works.md:27-37,91-123` — port architecture and module/build contract.
- `games/by3/CHANGELOG.md:42-44` — BY3 service findings, used only as precedent.

## S0.2 — Static linkage, permissions, and bounded call-site evidence

### Inputs and analysis tools

This pass reused `.babysteps/scratch/S0.1-NFS-feasibility`; it did not re-hash the NSP or extract RomFS. Rechecked hashes for the extracted NSOs and synthetic ELF carriers (`rtld`, `main`, `sdk`, `subsdk0`, `subsdk1`) match the existing local inventories/manifest. The analysis ELFs are carriers, not runnable binaries.

`build_app.build_hactool` built pinned hactool revision `1d64a83450e025622f3468c28fc4164dad2c5ef6` under `.babysteps/scratch/S0.2-NFS-feasibility`; `hactool -t npdm --json` decoded `main.npdm` to `.babysteps/scratch/S0.2-NFS-feasibility/npdm-permissions.json`. Disassembly and count scans used Apple LLVM `llvm-objdump` 21.0.0. `/opt/homebrew/bin/cstool` reports Capstone 5.0.9; Python is 3.15.0.

### Static dependencies and symbol-name matches

- `main` has `DT_NEEDED` strings for `C:/Nintendo/NFS_10_4_1/NintendoSDK/Libraries/NX-NXFP2-a64/Release/glslc.nss`, the same path ending in `multimedia.nss`, and `nnSdk.nss`. These names do not map to files among the extracted ExeFS NSOs. RomFS was not inspected, so package-wide absence and an SDK revision inferred from the path are not established.
- Exact-symbol matching across the extracted module inventories found 461 `main` imports: 456 match `sdk` exports, one matches `subsdk1` (`movie::SetAllocator`), and four are linker-boundary names. `sdk` has eight imports matched by `main` and one non-linker unmatched import, `nnosGetDefaultUserExceptionStackForApplication`, whose provider is unknown. `subsdk0` has 81 matches in `sdk` plus one in `main`; `subsdk1` has 646 in `sdk` plus three in `main`. These are name matches, not runtime-resolution proof. The existing port loader maps `main`, `sdk`, and `subsdk0`; NFS also contains `subsdk1`.

### NPDM permissions and raw SVC candidates

The decoded NPDM identifies title ID `010029b0118e8000`, main-thread priority 44, stack `0x100000`, and default CPU 0. Kernel access permits priorities 28–59 and CPU IDs 0–2. It lists 50 allowed SVCs and 46 service entries, including `friend:u`, `nifm:u`, `bsd:u`, `nvdrv`, `vi:u`, `nsd:u`, `sfdnsres`, `ssl`, and `time:u`; the full decoded set is in the scratch JSON. The hactool `is_retail` metadata bit is not signature verification and does not establish signature validity.

The raw instruction-pattern scan found 61 SVC candidates in `sdk`, four in `rtld` (`0x06`, `0x26`, `0x27`, `0x28`), and none in `main`, `subsdk0`, or `subsdk1`. Of the `sdk` candidate immediates, 46 are NPDM-allowed and 15 are not: `0x2a`, `0x40`, `0x41`, `0x43`, `0x44`, `0x45`, `0x48`, `0x49`, `0x4b`, `0x50`, `0x51`, `0x52`, `0x53`, `0x70`, `0x72`. Four NPDM-allowed values have no raw `sdk` candidate: `0x1e`, `0x20`, `0x24`, `0x5f`. These are raw scan candidates, not proof of reachable or executed SVCs.

### Register/counter observations and bounded call-site scan

LLVM disassembly contains raw `x18` operand mentions in 3,565 `main`, 5,108 `sdk`, 1,342 `subsdk0`, and 3,574 `subsdk1` instructions, and none in `rtld`. These counts are not distinct validated rewrite sites. In `sdk`, the scan found 60 `TPIDRRO_EL0`, five `TPIDR_EL0`, and 33 `CNTPCT_EL0` MRS/MSR sites; none were found in the other four carriers. Project documentation describes existing x18, thread-pointer, SVC, and counter-register rewrites with version-specific validation; no NFS rewrite map or sites were established, so these observations do not establish compatibility.

A bounded static call-site scan of the `main` ELF found a direct call to imported `nvnBootstrapLoader` at `0x76e6f0` (stub at `0x7f3c00`), `nn::nifm::Initialize` at `0x6411b0`, `nn::nifm::SubmitNetworkRequest` at `0x6411c0`, and `nn::socket::Initialize` at `0x64121c`. The stripped analysis carrier does not provide reliable local function/caller boundaries or a complete interprocedural path. These offsets do not establish ordering, startup reachability, first presentation, or runtime behavior. The bounded scanner found no direct call edge to friends imports; that is not proof they are unused. Ghidra was not configured. The local Hopper guide documents a REA bridge first-read timeout and says not to retry it.

### Closeout

No implementation GO. Conditional GO is limited to defining a focused follow-up evidence slice to resolve module/`.nss` provider semantics and trace startup from `nnMain` to the first NVN/presentation and network/friends calls using a functioning function-level analysis path. No bootability or gameplay claim is made.

## S0.2 source references

- `docs/how-it-works.md:9-13,27`, “The code”; `games/totk/runtime/loader.c:156-233,260-273,495-544` — existing module mapping/relocation, permissions, and `boot`.
- `build_app.py:68,737-834` — pinned hactool build path.
- `games/by3/scripts/prepare_modules.py:25-212` — analysis-carrier conversion.
- `.operator/guides/hopper-demo.md:3-7`, “Known limits” — local REA/Hopper limitation.

## S0.3 — Function-level startup and provider evidence

### Inputs and method

This pass reused `.babysteps/scratch/S0.1-NFS-feasibility`; it did not re-hash the NSP or re-extract any container. SHA-256 of `main.elf`, `sdk.elf`, `subsdk0.elf`, and `subsdk1.elf` matched the `elf_sha256` recorded in their `*-inventory.json` files (all four matched).

Hopper Demo was used for manual function inspection. Ghidra was unavailable: `GHIDRA_INSTALL_DIR` was unset, no headless analyzer was configured, and no matching native decompiler was available. The documented Hopper/REA bridge first-read timeout was not retried.

The function-level path was established with Apple LLVM `llvm-objdump` 21.0.0 disassembly of the verified `main.elf`, then cross-checked against Hopper decompilation for `nnMain`, `sub_768ab0`, and `sub_76e690`. Only direct `bl`/`b` edges are followed; indirect `blr` targets and function-pointer tables are not resolved. No NFS code was executed.

### Startup chain (static, function-level)

- `nnMain` is at `0x2d2fd0` and calls `sub_2a8120` at `0x2d30b8`.
- `sub_2a8120` calls `sub_3d4440` at `0x2a8164`.
- `sub_3d4440` calls `sub_3d4620` at `0x3d4500`.
- `sub_3d4620` tail-branches to `sub_7683b0` at `0x3d4730` (`b`).
- `sub_7683b0` calls `sub_768ab0` at `0x7684f0`.
- `sub_768ab0` calls `sub_76e690` at `0x768c14`.
- `sub_76e690` loads the name pointer at `0x916c3f`, whose string is `nvnDeviceGetProcAddress`, and calls the `nvnBootstrapLoader` PLT stub at `0x76e6f0` (`bl 0x7f3c00`). The import slot is `0x2087d38`.
- The return value is moved to `x21`; at `0x76e700` `sub_76e690` calls `sub_76a220`, and at `0x76e714` it performs `blr x8` through `[0xd37000+0x3d0]`.
- `sub_76a220` starts at `0x76a220` and has no `ret` before `0x76aaf0`. It references `nvnDeviceInitialize` at `0x76a284`, `nvnQueuePresentTexture` at `0x76a8c0`, `nvnWindowAcquireTexture` at `0x76aa9c`, and `nvnWindowSetPresentInterval` at `0x76aaf0`. These are name resolutions, not direct calls.

This is static function-level reachability only. It establishes no ordering against other startup work and no runtime behavior.

### Presentation

- `nnMain`'s loop calls `sub_2d30e0` at `0x2d3048` and `sub_2d4ce0` at `0x2d3088`.
- The actual `nvnQueuePresentTexture` call is not a direct call site; the name is resolved and NVN dispatch is indirect. The first present call is not established at the call-site level, and no runtime trace was captured.
- `nvnWindowPresent` is not a string in the binary; `nvnQueuePresentTexture` is the present name that is resolved.
- Split: resolve the indirect present target and capture a runtime first-present trace in a later slice. Do not infer the first present from the loop call sites.

### Network and friends

- `main` imports 7 `nn::nifm`, 22 `nn::socket`, and 17 `nn::friends` symbols, plus one `nvnBootstrapLoader`.
- The bounded call sites are `nn::nifm::Initialize` at `0x6411b0`, `nn::nifm::SubmitNetworkRequest` at `0x6411c0`, and `nn::socket::Initialize` at `0x64121c`; all three are in `sub_63fea0`.
- A direct caller chain exists: `sub_63fea0` ← `sub_643920` at `0x643a58` ← `sub_40c160` at `0x40c1d0` ← `sub_302040` at `0x302ae0` ← `sub_2f49e0` at `0x2f4a8c` ← `sub_2f4948` at `0x2f49b8`. `sub_2f4948` has no direct caller in the disassembly, so this is consistent with an indirect or thread entry; a direct path from `nnMain` was not found. The sites are linked and present, but startup reachability is not established, and no runtime observation was made.
- The S0.2 bounded scan found no direct `friends` call edge; that is not proof the imports are unused.
- The NPDM permits `friend:u`, `nifm:u`, and `bsd:u` (S0.2); permission is not a call.
- Split: determine whether `sub_63fea0` is reached through an indirect target or a thread entry. Do not treat these call sites as startup calls.

### Providers

- `main` has three `DT_NEEDED` strings: `glslc.nss`, `multimedia.nss`, and `nnSdk.nss` (full NintendoSDK paths). No extracted ExeFS module has those filenames.
- `sdk` exports 26,221 symbols and provides 456 of `main`'s 461 imports. `subsdk1` provides one (`movie::SetAllocator`). `subsdk0` provides none of `main`'s imports.
- `sdk` exports `glslc_Alloc`, `glslc_Free`, `glslc_AllocAlign`, `multimedia_AllocAlign`, and many movie/audio symbols. `subsdk0` exports 14 `glslc*` symbols. `subsdk1` exports 894 candidate movie/media/audio symbols.
- `main` imports zero `glslc` and zero `multimedia` symbols by name. It imports one `movie` symbol, `movie::SetAllocator`, provided by `subsdk1`.
- Candidate mapping: `nnSdk.nss` ↔ `sdk`; `glslc.nss` ↔ `subsdk0`/`sdk`; `multimedia.nss` ↔ `subsdk1`/`sdk`. These are candidates, not identified providers. The `DT_NEEDED` filenames and symbol content do not prove identity, and the loader's name-based resolution is not established. RomFS was not inspected, so package-wide absence is not established.
- `nnosGetDefaultUserExceptionStackForApplication` is imported by `sdk` (index 11, undefined, binding 2). It is not exported by `main`, `sdk`, `subsdk0`, `subsdk1`, or `rtld`. Its provider remains unresolved.
- Split: inspect RomFS and package metadata for `.nss` providers, and locate the `nnosGet...` provider. Do not assume the extracted modules satisfy the `DT_NEEDED` names.

### Claim separation

- Linked: the imports and `DT_NEEDED` entries above.
- Permitted: the NPDM service entries from S0.2.
- Statically reachable: the NVN chain from `nnMain` to `nvnBootstrapLoader` and the `sub_76a220` resolver cluster. The nifm/socket sites are present but not statically reachable from `nnMain` through direct calls.
- Runtime-observed: none; no NFS code was executed.
- Provider status: candidate or unresolved, not a runtime resolution.

### Closeout

No implementation GO; no bootability or gameplay claim; no port code changed. The report now traces the first NVN bootstrap chain with function-level static evidence, and explicitly splits the first present call, `sub_63fea0` startup reachability, `.nss` provider identity/absence, and the `nnosGet...` provider.

Next evidence: (a) resolve the indirect NVN present target and capture a runtime first-present trace; (b) determine the `sub_63fea0` entry path; (c) inspect RomFS and package metadata for `.nss` providers; (d) find the `nnosGet...` provider.

## S0.3 source references

- `.babysteps/scratch/S0.1-NFS-feasibility/elf/main-inventory.json`, `sdk-inventory.json`, `subsdk0-inventory.json`, `subsdk1-inventory.json`, `modules-summary.json` — hashes, imports/exports, `DT_NEEDED`, and symbol counts.
- `.babysteps/scratch/S0.3-NFS-startup/hopper-nnmain-listing.txt` — Hopper decompilation of `nnMain`.
- `.babysteps/scratch/S0.3-NFS-startup/main-disasm.txt` — full LLVM disassembly used for the direct-call graph.
- `.operator/guides/hopper-demo.md`, “Known limits”, “Manual read path” — Hopper access path and its documented limits.
- Addresses: `0x2d2fd0`, `0x2d30b8`, `0x2a8164`, `0x3d4500`, `0x3d4730`, `0x7684f0`, `0x768c14`, `0x76e6f0`, `0x76e700`, `0x76e714`, `0x76a220`, `0x76a284`, `0x76a8c0`, `0x76aa9c`, `0x76aaf0`, `0x6411b0`, `0x6411c0`, `0x64121c`, `0x2087d38`, `0x7f3c00`.

## S1.1 — Extraction and build manifest

Read-only scoping. No port code, runtime code, or manifest file was added; no bootability or gameplay claim. Scratch: `.babysteps/scratch/S1.1-NFS-manifest/`.

### Reusable helpers, and the exact form

- **ExeFS extraction is reusable as-is through the BY3 title-parameterized path.** `games/by3/scripts/extract_code.py` takes `--title` and resolves the Program NCA through `program_header(f, entries, keys, title)` (`games/by3/scripts/merge_romfs.py`). This command was run and completed in 1.2 s:
  `python3 games/by3/scripts/extract_code.py game.nsp --keys prod.keys --title 010029B0118E8000 --output <scratch>/exefs`
  Its manifest (`.babysteps/scratch/S1.1-NFS-manifest/extraction-manifest.json`) reports the same six files and byte-identical SHA-256 values as the S0.1 extraction. That is the reuse proof.
- **`games/totk/scripts/extract_exefs.py` is not reusable.** `program_nca` (`:160-178`) requires title `0100F2C0115B6000` and rejects any other package.
- **Both `extract_romfs.py` copies are not reusable.** They call `program_nca(nca_inventory(...))` with the TOTK hardcode, so RomFS extraction needs the title parameter that `program_header` already accepts.
- **`games/by3/scripts/prepare_modules.py` is reusable for analysis carriers only, with one change.** Its default module list is `['rtld', 'main', 'sdk', 'subsdk0']` (`:215-230`); NFS has a sixth module, `subsdk1`, so the list must include it. Its output is an ELF carrier, not a runnable program. It needs Python `lz4`, which the system interpreter lacks (S0.1 wrapped system `liblz4`; no dependency was added then or now).
- **The rewriter is untested on these modules.** `third_party/d2r/patch_nso.py` with `DM_RULESET=native` is what `step_code` invokes; nothing in S0.x ran it against NFS code.

### Build manifest: the exact files and inputs

`build_app.py` is TOTK/BOTW-only at module scope: `GAME`/`BOTW` (`:49,60`), the manifest loads (`:52-55,61`), `VERSIONS` (`:56`), `TITLES` (`:64`), `APPS` (`:67`). A third title is not a drop-in; it needs a `games/nfs/` tree plus entries in each.

- Per-version manifest: `games/<game>/assets/expected-v<display>.json`, listed in `VERSIONS`. Required keys, from `games/totk/assets/expected-v1.4.3.json`: `title_id`, `version` (integer), `display_version`, `exefs_sha256`, `rewritten_sha256`, `romfs{size,sha256}`, `shared_fonts_sha256`.
- `version` is the CNMT version, not a function of the display string: TOTK 1.4.3 is `720896` (`0xB0000`) and BOTW 1.6.0 is `786432` (`0xC0000`). It cannot be inferred, so it gates the manifest filename as well as its contents.
- Output paths `step_code` writes into the work tree (`prepare_work`: `work/totk` ← rsync of `GAME`, excluding `extracted/ rewritten/ build/ analysis/ .venv/ tooling/ user-data/`): `extracted/<version>/exefs/` and `extraction-manifest.json`; `extracted/<version>/elf/`; `rewritten/<version>/{main,sdk,subsdk0}.patched.bin`; `analysis/phase12-validation.json` (v0) or `analysis/<version>-port/phase12-validation.json`; logs `extract_exefs-<version>.log`, `prepare_modules.log`, `rewrite_<version>_<module>.log`.

### Inputs with hashes

- Source: `target-port/need-for-speed-hot-pursuit-remastered/game.nsp`, 7,388,586,538 bytes, 11 entries, 5 NCAs, 1 ticket. No whole-package SHA-256; S0.1 recorded that limit and a retry here did not change it. Identity is carried by the Program NCA and the per-file hashes below.
- Program NCA `a040a32c4a4389eb6ad4baeecb97740c.nca`, offset 736, size 7,387,971,584, `content_type` 0, title `010029B0118E8000`.
- ExeFS, all hashes matched between the S0.1 extraction and the S1.1 re-extraction: `main` 6,906,316 / `721afa1df58eefe388b0630267f9f7ae09d29a95c54136ff770fd6981a76ae5e`; `main.npdm` 1,540 / `c0b0163c578584ea4beb1165705b91bd55de21ae3f648763387797e84f7a48a3`; `rtld` 7,223 / `e1b018d97186c07e69d1203326183dddcd575af8fe192433b848c30ca650eb82`; `sdk` 5,619,621 / `c54bf6080c82b1946574714e7bac2a1d2b15cc7e1198785bde0b95b393ead722`; `subsdk0` 4,501,008 / `6afb3c974222876eae903a645b1da2abcd896c4fd5ff77e26cb2d6f6e77eb185`; `subsdk1` 3,397,429 / `efc5c92a4b55ff050d040f1236f02663ccfdfab14a80e59f312c38e23449860f`.
- Meta NCA `8938618b1d4bb12519520df058f8ab10.cnmt.nca`, offset 7,388,511,456, size 3,584, `content_type` 1; its NCA header reports SDK version **10.4.0.0**, which matches the `NFS_10_4_1` path embedded in `main`'s `DT_NEEDED` strings.
- That meta NCA's section 0 is a 400-byte PFS0 holding one 304-byte `Application_010029b0118e8000.cnmt`. Its payload begins `00 80 8e 11 b0 29 00 01`, which is the CNMT's title-id field: the file carries no `CNMT` magic. The decode is S1.3a; the version reads `0`.

### Staged package contents

One Program NCA and no BKTR: this is a base package, so extraction does not need an update container, and `prepare_update_container` / `identify_update` do not apply. The other four NCAs are `content_type` 2 (control) at offset 7,387,972,320 size 287,744; two `content_type` 3 at 7,388,260,064 size 136,704 and 7,388,396,768 size 114,688; and the meta at 7,388,511,456.

### Closeout

Manifest identified; extraction reuse proven by identical hashes; nothing built and nothing run. Splits: (a) decode the CNMT so `version` and `display_version` are read rather than inferred, before any `expected-*.json` exists; (b) run the three-or-four module rewrites to obtain `rewritten_sha256`; (c) extract RomFS with a title-parameterized extractor for `romfs{size,sha256}`; (d) establish the `shared_fonts_sha256` six-font input for a package with no system update partition; (e) add `subsdk1` to whatever module list the runtime load set uses.

## S1.1 source references

- `.babysteps/scratch/S1.1-NFS-manifest/extraction-manifest.json`, `exefs/`, `meta/cnmt.nca`, `meta/section0.bin`, `meta/pfs0/` — re-extraction, per-file hashes, and the CNMT decode attempt.
- `games/by3/scripts/extract_code.py:11-14,17-60`; `games/by3/scripts/merge_romfs.py` (`open_program`, `program_header`, `BASE_TITLE`); `games/by3/scripts/prepare_modules.py:25,215-230`.
- `games/totk/scripts/extract_exefs.py:160-178`; `games/totk/scripts/extract_romfs.py:main`; `games/by3/scripts/extract_romfs.py:main`.
- `build_app.py:48-70,307-336,340-357,499-566`; `games/totk/assets/expected-v1.4.3.json`; `games/botw/assets/expected-v1.6.0.json`; `games/by3/assets/expected-v1.2.0.json`.
- `.babysteps/scratch/S0.2-NFS-feasibility/work/src/hactool/hactool` — pinned hactool used for the meta NCA header and section dump.

## S1.2 — Verified ARM64 module rewrites

Read-only evidence pass. No port runtime code, loader, or manifest file was added or changed; no game code was executed; no bootability or gameplay claim. Scratch: `.babysteps/scratch/S1.2-NFS-rewrite/`.

### Carriers, including `subsdk1`

- `prepare_modules.py` was copied into scratch and its module list changed to `['rtld', 'main', 'sdk', 'subsdk0', 'subsdk1']` (`:222`). That one line is the whole difference from `games/by3/scripts/prepare_modules.py`, which is unchanged in the tree.
- `prepare_modules.py` and `patch_nso.py` both need Python `lz4`; the rewriter also needs `capstone`; the system interpreter (3.15) has neither. Work ran in a scratch venv and no dependency entered the repository: `uv venv --python 3.12 <scratch>/venv && uv pip install --python <scratch>/venv/bin/python capstone lz4` (capstone 5.0.9, lz4 4.4.5).
- All five carriers were built and each reports `segment_hashes_verified: true`. NSO build IDs: rtld `c1ed34fd…`, main `3515e683…`, sdk `d9add76f…`, subsdk0 `ebedb264…`, subsdk1 `84e50460…`. None equals the registered D2R build ID `db0507f36b84200ac05f75d76ed5026f`.

### Five modules, five recorded hashes

Command per module, with the same environment pair `step_code` uses (`build_app.py:540-541`):

`DM_RULESET=native DM_PRIVATE_GATES=0 <scratch>/venv/bin/python games/by3/third_party/d2r/patch_nso.py <exefs>/<module> rewritten <module>`

| module | NSO bytes | NSO SHA-256 | rewrite points | gates | `.patched.bin` bytes | rewritten SHA-256 |
| --- | --- | --- | --- | --- | --- | --- |
| rtld | 7,223 | `e1b018d97186c07e69d1203326183dddcd575af8fe192433b848c30ca650eb82` | 4 | 4 | 65,696 | `1b269a41782151ab7d69703baa1f227b6018b126b63890d0b5103a6744a9354d` |
| main | 6,906,316 | `721afa1df58eefe388b0630267f9f7ae09d29a95c54136ff770fd6981a76ae5e` | 7,506 | 0 | 34,447,824 | `02a24b38914bf6b9e0102b38a61b7f63a07d93bf1032c272d4e4f943aa5bf260` |
| sdk | 5,619,621 | `c54bf6080c82b1946574714e7bac2a1d2b15cc7e1198785bde0b95b393ead722` | 11,812 | 94 | 14,256,876 | `a42b758bc5f2160aefd685339fe110660dd18acb3bbc9feae79162bcc00c27d5` |
| subsdk0 | 4,501,008 | `6afb3c974222876eae903a645b1da2abcd896c4fd5ff77e26cb2d6f6e77eb185` | 2,963 | 0 | 10,417,284 | `386e15370672cfe7b5b640250308ebad28f6218435fe5934e7cfca5ee5a3bf4c` |
| subsdk1 | 3,397,429 | `efc5c92a4b55ff050d040f1236f02663ccfdfab14a80e59f312c38e23449860f` | 10,203 | 0 | 8,282,384 | `3dbc80c43067e693d1eac16ef7ae797d927d1d8dcf1d69a1a10d9ed40f7b4260` |

- Registered D2R cuts and observation gates applied to none of them: `cuts 0`, `obs 0` on every module, because both tables are keyed by the D2R build ID. The rewrites are the generic x18 / thread-pointer / SVC / counter substitutions only.
- Every module exited 0, and every rewrite is reproducible: a second run into `rewritten-verify/` is byte-identical for all five (`logs/verify_*.log`).
- Rewriter digest: `patch_nso.py` `f2ac79896e7f7d1f3575910e64233e23375da03bbcf40e19acfa747277980be3`.

### Independent structural check

`games/by3/third_party/d2r/structck.py rewritten <module> <exefs>/<module>` with `LLVM_BIN=/opt/homebrew/opt/llvm@22/bin` — `llvm@23`, the path the check defaults to, is not installed here. All five end `STRUCT <module> OK … 失败项=0`:

- `STATIC` recounts with llvm-objdump report `violations=0` on all five, and changed text words equal the rewrite heads on all five.
- sdk's gate and system-register counts here (94 gates; 60 `tpidrro`, 33 `cntpct`, 5 `tpidr`) match the S0.3 raw-scan counts, recounted independently of the rewriter's own capstone pass.

### What the S1.1 reading could not settle

S1.1 read `meta/cnmt.nca` as raw container bytes: 3,584 bytes with no `NCA0` at `0x200`, and the 304-byte payload not found inside them. Neither observation was evidence: the NCA3 header is AES-XTS(`header_key`) and the section body is AES-CTR, so no structure appears before decryption. S1.3a decrypted both layers — the section holds exactly the 304-byte file S1.1 kept, and the version reads `0`.

### Closeout

Closes-when met: each module rewrote with a recorded source and output SHA-256, an independent structural check passed, the baseline is green, and no port runtime code changed. Nothing was executed. Splits carried: (a) re-extract and decode the CNMT for `version`/`display_version`; (b) title-parameterized RomFS extraction for `romfs{size,sha256}`; (c) the six-font `shared_fonts_sha256` input; (d) the `subsdk1` runtime load set; (e) the rewrite has no registered cuts, so nothing here decides the memory-budget or offline-window behaviour those D2R cuts carry.

## S1.2 source references

- `.babysteps/scratch/S1.2-NFS-rewrite/rewrite-results.json` — per-module source and output hashes, sizes, site/gate/cut counts, `structck` lines, rewriter digest.
- `.babysteps/scratch/S1.2-NFS-rewrite/{elf,rewritten,rewritten-verify,logs}/` — carriers, rewrites, reproducibility run, `rewrite_<module>.log`, `verify_<module>.log`, `structck_<module>.log`, `gate.log`.
- `.babysteps/scratch/S1.2-NFS-rewrite/prepare_modules_scratch.py` — the one-line-different copy; `diff` against `games/by3/scripts/prepare_modules.py` is `:222` only.
- `.babysteps/scratch/S1.1-NFS-manifest/meta/` — the artifacts the CNMT note above measured.
- `games/by3/third_party/d2r/patch_nso.py:30,171-183,1068-1119`; `games/by3/third_party/d2r/structck.py:1-60`; `games/by3/scripts/prepare_modules.py:25-212,215-230`; `build_app.py:540-541`.

## S1.3a — The CNMT version and the NACP display version

Read-only decode. No port code, loader, `expected-*.json`, RomFS extraction or game code was added, and nothing was executed. Scratch: `.babysteps/scratch/S1.3a-NFS-cnmt/` (`EVIDENCE.md` carries the whole chain).

### Both values, and the bytes that carry them

| Field | Value | Container byte offset | Plaintext position | Bytes |
| --- | --- | --- | --- | --- |
| `version` (integer) | `0` (`0x0`) | 7,388,514,656 | CNMT `+0x08` | `00 00 00 00` |
| `display_version` | `1.0.0` | 7,388,070,208 | `control.nacp` `+0x3060` | `31 2e 30 2e 30 00 …` |

- **`display_version` is not a CNMT field.** A printable-run scan of all 304 CNMT bytes is empty; the string lives in the control NCA's NACP. Read it there or not at all.
- `version` `0` is a content version, not a display-derived number — the same reason TOTK 1.4.3 is `0xB0000` and BOTW 1.6.0 is `0xC0000` (S1.1). The manifest filename follows the display string: `expected-v1.0.0.json`.

### The meta NCA, decoded

- Container entry `8938618b1d4bb12519520df058f8ab10.cnmt.nca`: PFS0 entry at 7,388,510,720, first byte at 7,388,511,456 (data start 736), 3,584 bytes, SHA-256 `8938618b…1abf5f630`.
- Its header is **AES-XTS(`header_key`)**, six 512-byte sectors tweaked by sector index; only then does `0x200` read `NCA3`. `content_type` 1 (meta), `title_id` `010029B0118E8000`, SDK 10.4.0.0, rights id zero.
- Section 0: media blocks 6..7 = NCA `0xC00..0xE00`, `crypt_type` 3 (CTR). Body key = key area slot 2 of `key_area_key_application_0a` = `68a273da…019c3af0`, the value hactool prints as "Key 2 (Decrypted)"; the IV is the 8-byte section CTR at `fs_header+0x140` (zero here) as a big-endian high half, with the block's absolute NCA offset >> 4 as the low half.
- The section does not begin with its partition. Its first 32 bytes are the hash table (`hash_table_offset` 0, size 32; SHA-256 of them equals the superblock `master_hash` `e22ad794…3e0990`), and the PFS0 starts at `pfs0_offset` 32 with `pfs0_size` 400. The header's own section hash at `0x280` is SHA-256 of the 0x200-byte fs header and matches (`42aefcf6…f54b99a`).

### The CNMT, decoded from the bytes

`Application_010029b0118e8000.cnmt`, 304 bytes, SHA-256 `25cba081…0d1e1f4` — byte-identical to the copy S1.1 kept, so S1.1 had the file and lacked only the decode.

- `0x00` title id `010029B0118E8000`; `0x08` **version `0`**; `0x0C` type `0x80` (Application); `0x0E` extended-header size 16; `0x10` content count 4; `0x12` content-meta count 0; `0x20` extended header: original id `010029B0118E8800`, required system version `0x28200000` (673,185,792).
- No `CNMT` magic precedes the title-id field in these bytes. What confirms the structure is the content table, not a magic: four records from `0x30`, `0x38` bytes each (`hash` 0x20, `id` 0x10, `size` 6 bytes LE, `type` 1, reserved 1), then the 0x20-byte digest at `0x110`. Each record's id is the first 16 bytes of its hash, and its size and type match the NCA in the container.
  - Program `a040a32c…`, 7,387,971,584, type 1; Control `cf889dea…`, 287,744, type 3; LegalInformation `8a334a23…`, 136,704, type 5; HtmlDocument `e908c0d7…`, 114,688, type 4. Digest `83eb59b4…b8b96d46`, which is not SHA-256 of the preceding bytes.
- Three of the four hashes were re-derived from the container by hashing the NCA entries (control, legal, html) and match; the Program NCA was not hashed (Do-Not #2).

### The NACP, and what the package's sidecars are worth

- Control NCA at 7,387,972,320 (287,744), `content_type` 2. Section 0: NCA `+0xC00`, 284,672 bytes, CTR as above, fs header an `IVFC` RomFS superblock whose last level starts at section `+0x14000`.
- `control.nacp` is 16,384 bytes at section `+0x14200`; the 16 ASCII bytes at `+0x3060` read `1.0.0`. The file starts with the 16 language entries (0x3000: name 0x200 + publisher 0x100) and carries no `NACP` magic either.
- The package also holds two sidecars inside its PFS0, `8938618b….cnmt.xml` and `cf889dea….nacp.xml`. Both agree with the decoded bytes (`<Version>0</Version>`, `<DisplayVersion>1.0.0</DisplayVersion>`) and neither is needed. Their Meta `<Hash>` `f7ca7ebb…` is not the meta NCA's SHA-256 `8938618b…f630`, so they are not a hash source.

### Closeout

Closes-when met: both values are read from container bytes with recorded offsets, the configured baseline passes, and no port runtime code changed. Splits carried: (a) title-parameterized RomFS extraction for `romfs{size,sha256}` → S1.3b; (b) the six `shared_fonts_sha256` inputs → S1.3b; (c) `subsdk1` in the runtime load set → S1.3; (d) `.nss` provider identity/absence and the `nnosGet…` provider → S1.3; (e) indirect first-present target and runtime trace → S2.1.

## S1.3a source references

- `.babysteps/scratch/S1.3a-NFS-cnmt/EVIDENCE.md` — the offset chain from PFS0 table to either field, plus the keys and IV rule.
- `.babysteps/scratch/S1.3a-NFS-cnmt/{decode_cnmt.py,decode_nacp.py}` — the two decoders, reusing `games/by3/scripts/extract_exefs.py` (`container_entries`, `read_at`, `decrypt`, `nca_ctr_key`, `common_tickets`) and `games/by3/scripts/merge_romfs.py` (`Ctr`).
- `.babysteps/scratch/S1.3a-NFS-cnmt/{cnmt-report.json,nacp-report.json,content-hash-check.json}` — decoded fields, offsets, and the three re-derived content hashes.
- `.babysteps/scratch/S1.3a-NFS-cnmt/{meta.cnmt.nca,section0.bin,Application_010029b0118e8000.cnmt,control-section0.bin,control.nacp,cnmt.xml,section0-hactool.bin}` — the bytes each claim above was measured on.
