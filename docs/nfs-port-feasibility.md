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

## S1.3b — The RomFS digest and the six shared-font inputs

Read-only measurement. No port code, loader, `expected-*.json` or game data was added, and nothing was executed beyond the extractors. Scratch: `.babysteps/scratch/S1.3b-NFS-romfs/` (`EVIDENCE.md` carries the full record).

### The RomFS input, and the exact image it covers

`build_app.py:step_data` checks `index.json['romfs_sha256']` against `EXPECTED['romfs']['sha256']` and `romfs.bin` st_size against `EXPECTED['romfs']['size']`, so the digest must cover exactly the extracted image bytes — `games/by3/scripts/extract_romfs.py` writes `romfs.bin` as the last IVFC level and digests those bytes as they stream. The extraction was driven title-parameterised: a scratch copy of that flow resolving the Program NCA through `open_program(path, keys, title)` / `program_header` with `--title 010029B0118E8000`, replacing the TOTK-hardcoded `program_nca(nca_inventory(...))`. The source and Program NCA record were cross-checked against the S1.1 extraction manifest.

| Field | Value |
| --- | --- |
| Container entry | `a040a32c4a4389eb6ad4baeecb97740c.nca` (Program, title `010029B0118E8000`), NCA offset 736, size 7,387,971,584 |
| RomFS section | index 1, media 224..14,389,632 → NCA byte offset 114,688, container byte offset 115,424 |
| `romfs.size` | **7,352,922,956** — equals level-5 length and `romfs.bin` st_size |
| `romfs.sha256` | **`2bc692dec1bd6df0b13050474c8d7aa84450d34e2a7b4e3f52ad9f4fe827a788`** — over exactly those bytes |
| IVFC blocks per level | 1 / 1 / 1 / 2 / 877 / 448,787 — all six levels block-verified |
| Indexed files | 19,169 (`romfs/index.json`: path → image offset + size) |

Wall times, both runs explicitly sized (Do-Not #2 is about the whole NSP, but the same 120 s window applies): extraction with streamed digest 357.96 s; a standalone `shasum -a 256 romfs.bin` re-run of the image file 14.14 s, returning the same digest.

### The six shared-font inputs

The NSP has no system update partition, so `--xci` does not apply and the fonts are firmware inputs: `games/by3/scripts/extract_shared_fonts.py --firmware <Ryujinx registered>` extracted them (NCA FS header + six IVFC levels verified per font NCA; BFTTF header decodes to an sfnt font). It needs **no game title parameter** — the six fonts are located by their own title IDs `0100000000000810`–`0100000000000814`, and meta NCAs are excluded by content type. Each font was re-read at its indexed offset and cross-checked against the script's own `manifest.json`.

| # | name | title_id | image offset | bfttf bytes | font bytes | sha256 | vs reference |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | standard | `0100000000000811` | 512 | 2,053,636 | 2,053,628 | `823913c9d80807979381e327846466f561f31ba1842cea8f7fedc5ad7d45d2ad` | match |
| 1 | chinese-simplified | `0100000000000814` | 124,432 | 7,848,208 | 7,848,200 | `a04eb490283d8393b2c49c52e315132461d790ac73996cb43ac869ce72003938` | differs |
| 2 | chinese-simplified-extension | `0100000000000814` | 512 | 123,920 | 123,912 | `f5784e7a553fef9a05b3b7dba94e77e79162ace9db8dafc715ace0be1fd9b9ff` | differs |
| 3 | chinese-traditional | `0100000000000813` | 512 | 3,580,268 | 3,580,260 | `9d59b74e59a2dc92296d6bebad7cce791ba94688bd436304aaae174a58bd7457` | match |
| 4 | korean | `0100000000000812` | 512 | 3,672,876 | 3,672,868 | `5a4e694782ead5ddcbc0a226aab24362e2d641ca33bac7bdf8cda36ff157c9e8` | match |
| 5 | nintendo-extension | `0100000000000810` | 237,264 | 180,244 | 180,236 | `f9f62e025c9bbe0e9ae25a54ba5011202b1e3a3f3c33b2d876c40aff6174af68` | match |

"vs reference" compares against `games/totk/assets/expected-v1.4.3.json` `shared_fonts_sha256` (identical in every TOTK/BOTW manifest — the reference dump). Four match. The two zh-cn fonts (both from title `0100000000000814`) differ: this machine's firmware dump is another version for that title, and both are structurally valid BFTTFs. `build_app.py:font_report` binds a valid font from another firmware version by the SHA-256 of its actual bytes, so either pin is workable; which value `expected-v1.0.0.json` pins is a manifest-slice decision. Font NCA provenance in this dump: `registered/{a05c2dba…,61562912…,3684576b…,f846a1c2…,8bb427d2…}.nca/00` for titles 0810–0814 respectively.

### The package cannot supply the fonts

The game RomFS holds zero font files (no `.bfttf`/`.ttf`/`.otf` among 19,169 paths) and the container's five NCAs are Program, Control, LegalInformation, HtmlDocument and Meta, so there is no font source inside the game package. `build_app.py:step_data` accordingly requires `--firmware` or `--shared-fonts` for NSP builds.

### Closeout

Closes-when met: both manifest inputs are measured with recorded bytes and offsets, the configured baseline passes, and no port runtime code changed. Splits carried: (a) the per-version manifest `games/nfs/assets/expected-v1.0.0.json` itself, including the font pin decision → S1.3c; (b) `subsdk1` in the runtime load set → S1.3; (c) `.nss` provider identity/absence and the `nnosGet…` provider → S1.3; (d) indirect first-present target and runtime trace → S2.1.

## S1.3b source references

- `.babysteps/scratch/S1.3b-NFS-romfs/EVIDENCE.md` — the full measured record, commands, and the checked-image argument.
- `.babysteps/scratch/S1.3b-NFS-romfs/{extract_romfs_nfs.py,font_offsets.py}` — the title-parameterised extractor (reusing `games/by3/scripts/{extract_exefs,extract_romfs,merge_romfs}.py`) and the font offset probe.
- `.babysteps/scratch/S1.3b-NFS-romfs/romfs/{romfs.bin,index.json}` — the extracted image and its index; `{digest-recheck.out,digest-recheck.time}` — the standalone digest re-run and its wall time.
- `.babysteps/scratch/S1.3b-NFS-romfs/{shared-fonts/manifest.json,font-index.json}` — the six font hashes and their recorded offsets.

## S1.3 — The games/nfs tree and the boot through nnMain

Implementation slice (2026-10-22), not read-only: the `games/nfs/` tree and the third-title build wiring were built, and the loader booted through `nnMain`. No bootability or gameplay claim is made beyond the recorded observation.

### The `games/nfs/` tree

- Forked from `games/by3/` (that tree was itself forked from `games/totk/`): `runtime/`, `third_party/d2r/`, `third_party/d2r_gfx/`, `scripts/`, `Makefile`, `requirements.txt`, `README.md`. The launcher bundle and the prebuilt shader translator are NOT carried — they arrive with the app-assembly slice.
- Boundary check clean: no runtime or build-time reach into `games/totk/` or `games/by3/`; the `totk_*.h` include names inside the tree resolve to the fork's own copies (12/12 verified); predecessor names appear only in provenance comments.

### Registration

The seed's `newFileRegistration`:

- `build_app.py` module scope loads `games/nfs/assets/expected-v1.0.0.json` as `VERSIONS['nfs100']`, `TITLES['010029B0118E8000']='nfs'`, `APPS` entry `('Need for Speed Hot Pursuit','org.macnativegames.nfs','NFS')`.
- `prepare_work` rsyncs `games/nfs` to `work/nfs`; `step_code_nfs`/`step_data_nfs` check every manifest key — six `exefs_sha256` files, five `rewritten_sha256` images, `romfs` size+sha256, `shared_fonts_sha256` pins via `font_report`'s reference-pin/actual-bytes fallback.

### The build run

2026-10-22, work folder `target-port/need-for-speed-hot-pursuit-remastered/NFS_build`, ~5 min:

- ExeFS extracted by `games/nfs/scripts/extract_code.py --title 010029B0118E8000`; all six files matched `exefs_sha256`.
- Five rewrites produced with `DM_RULESET=native DM_PRIVATE_GATES=0` by `games/nfs/third_party/d2r/patch_nso.py` and matched `rewritten_sha256` exactly: rtld, sdk, subsdk0, subsdk1, main.
- RomFS extracted title-parameterised (`games/nfs/scripts/extract_romfs.py`, resolving the Program NCA through `open_program`/`program_header`), all six IVFC levels verified, `romfs.sha256` `2bc692dec1bd6df0b13050474c8d7aa84450d34e2a7b4e3f52ad9f4fe827a788` and size 7,352,922,956 matched.
- Shared fonts from the firmware dump (`Ryujinx bis/system/Contents/registered`); `chinese-simplified` `a04eb490283d8393b2c49c52e315132461d790ac73996cb43ac869ce72003938` and `chinese-simplified-extension` `f5784e7a553fef9a05b3b7dba94e77e79162ace9db8dafc715ace0be1fd9b9ff` bound by actual bytes (the manifest pins the reference dump; the valid-BFTTF fallback bound them).
- All the build's own manifest checks passed.

### The load set

Six ExeFS entries: five mapped rewrite images — rtld 81,920 bytes/20 symbols; main 34,471,936/485; sdk 14,270,464/26,235; subsdk0 10,420,224/107; subsdk1 8,290,304/11,535 — plus `main.npdm` loaded as process metadata (1,540 bytes, SHA-256 `c0b0163c578584ea4beb1165705b91bd55de21ae3f648763387797e84f7a48a3` bound, `META` header).

Relocations: 141,377 total. Per module (relative/symbolic/cross/weak-zero): rtld 18/19/3/16; main 46,256/590/586/4; sdk 9,603/22,653/10/6; subsdk0 42,966/88/83/4; subsdk1 1,676/17,508/6,547/4. No strong import failed to resolve (the loader fails the boot on any unprovided strong import; it did not).

### The rtld W^X finding

- rtld's rewritten layout packs text (TEXT_END `0x18a0`), rodata (`0x2000`) and writable data (`0x3000`–`0x4000`) into one 16 KiB host page. A host page cannot be executable and writable at once, so `seal()` keeps the page writable, rtld's text is never executed, and its DT_INIT is skipped (logged at boot).
- rtld exports no symbols, so nothing calls into it. The predecessor loaders never mapped rtld at all (TOTK and BY3 map three modules each), so this was unexercised before.

### Boot observation

Headless `make boot` (log kept): `nn::init::Start` entered; the observer logged `ENTER nnMain` with `nnMain at main+0x2d2fd0` (the S0.3 static entry address). Observed in order:

- `nnosInitialize` (guest layout alias/stack/heap), rtld callback "modules linked", `nninitInitializeSdkModule` (runtime RomFS full SHA-256 verified over 7,352,922,956 bytes; 19,169 files; no overlays), `nninitStartup`;
- DT_INITs: main state=2 init_array_count=207, sdk 2/110, subsdk0 2/4, subsdk1 2/45 (rtld skipped, W^X);
- then inside nnMain: operation mode docked, two performance requests served, four guest threads created (entry `0x10…f20`), `nv::InitializeGraphics` served by the HLE ("native Metal graphics context initialized", Metal device "Apple M4 Pro").
- Stopped at `FAIL: VI requires native macOS window`.

Windowed run (log kept): the frontend created a window drawable 2628x1478 (scale 2), then stopped at `FAIL: native display link initialization` before the guest started. A standalone probe in the same session: `CVDisplayLinkCreateWithActiveCGDisplays` returns -6661 (deprecated since macOS 15.0). Environmental vs API breakage is not established.

### Not observed

- The S0.3 chain endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")` — observed in S2.2: the HLE logs the call, resolves 523 names, answers 98 properties (0x00–0x61), runs the first GPU frame, then stops at `hid` IPC service.
- rtld's DT_INIT not run (W^X above). No present, no frame, no audio, no gameplay.
- S2.2 extended observation: first GPU frame (27 functions, 32 calls including `QueuePresentTexture`), then `FAIL: IPC service not implemented` for `"hid"` — the game halts after detecting as a local macOS process and initializing account/controller, but the `hid` service is unimplemented. The runtime already has GameController framework wiring in the HID layer; S2.2b implements `hid` for controller input.

### Provider evidence

Measured on the S1.2 scratch inventories and the S1.3b RomFS index; recorded in `.babysteps/scratch/S1.3-NFS-boot/provider-evidence.json`:

- (a) `DT_NEEDED` appears only in `main`: `glslc.nss`, `multimedia.nss`, `nnSdk.nss` (the `NFS_10_4_1` path strings); rtld/sdk/subsdk0/subsdk1 carry none.
- (b) No `.nss` file exists in the ExeFS file set (six entries) or in the decrypted RomFS (19,169 indexed paths).
- (c) main's 461 imports: 456 provided by `sdk`, 1 by `subsdk1` (`movie::SetAllocator`); the 4 unresolved are weak linker symbols (`__rel_dyn_start/end`, `__rel_plt_start/end`).
- (d) `glslc_Alloc/Free/Realloc/GetAllocator` (imported by subsdk0, binding 1) and `multimedia_Alloc/Free/Realloc/GetAllocator/AllocAlign` (imported by subsdk1, binding 1) all bind to `sdk`'s exports.
- (e) `nnosGetDefaultUserExceptionStackForApplication`: imported by `sdk` (dynsym index 11, binding 2 = weak), exported by no module; the loader binds unprovided weak imports to zero (only strong unprovided imports fail), so it is not boot-blocking — confirmed by the boot.

Claim boundary: the DT_NEEDED filename strings are NOT proven to identify particular NSOs; what is established is that every strong import of every module is satisfied inside the mapped load set and no `.nss` file exists in the package's decrypted layers.

### Gate

`python3 -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0 (the configured baseline).

### Closeout

Closes-when met: the `games/nfs/` tree wired through `build_app.py` rebuilds NFS from its original files with every manifest check passing, the loader maps/loads all six ExeFS entries (five rewrite images plus `main.npdm`) and boots through `nnMain` with the observation recorded, the provider questions are resolved at symbol level (the filename-identity question explicitly not claimed), the configured baseline passes, and no gameplay claim is made. Splits carried: (a) the S0.3 `nvnBootstrapLoader` chain endpoint and the runtime trace → S2.1; (b) the two window-path blockers (`FAIL: VI requires native macOS window` headless; `FAIL: native display link initialization` windowed) → S2.1 first business; (c) the `sub_63fea0` indirect/thread entry path → S3.2; (d) the `hid` IPC service stub → S2.2b.

## S1.3 source references

- `.babysteps/scratch/S1.3-NFS-boot/{provider-evidence.json,boot-headless.log,boot-windowed.log,build.log}` — the provider record and the kept run logs.
- `games/nfs/{README.md,Makefile}`
- `games/nfs/scripts/{extract_code.py,extract_romfs.py,prepare_modules.py,prepare_runtime.py}`
- `games/nfs/runtime/loader.c` (load-set, seal, boot logging)
- `build_app.py` (NFS registration and `step_code_nfs`/`step_data_nfs`)

## S2.1 — The window-path blockers and the required NVN surface

Implementation and observation slice (2026-10-22). The two S1.3 window-path blockers are cleared with
evidence, the NVN surface the S0.3 `nvnBootstrapLoader` chain reaches is mapped with every row tied to a
recorded observation, and the chain's runtime reach is recorded to its stop. No gameplay claim.

### The windowed blocker: display link initialization

- S1.3's `FAIL: native display link initialization` covered three calls behind one `runtime_fail`, so the
  record can never say which returned -6661 (Do-Not #5). Each call's CVReturn is now logged at the site,
  and the create path retries the chosen screen's display id before falling back to a host timer.
- Standalone probe in this session (`dlprobe.m`/`dlprobe.out`, scratch), macOS 27.0.0, console session
  (`onConsole=1`): **two active displays** (2: 1920x1080, nominal refresh 0.0083335 s; 4: 1314x985,
  0.0166667 s). `CVDisplayLinkCreateWithActiveCGDisplays` returned **0 in all three contexts** (bare,
  after `NSApplication`, after an NSWindow + CAMetalLayer); every `CVDisplayLinkCreateWithCGDisplays`
  variant, `SetOutputCallback` and `Start` also 0. An offscreen `CAMetalLayer` produced a drawable
  with a texture.
- The S1.3 record's -6661 is `kCVReturnInvalidArgument`, the code `CVDisplayLinkCreateWithActiveCGDisplays`
  reports when there is no active display to bind; the run it came from was 03:40 local, unattended. Bounded
  claim: the failure is environmental (consistent with the machine's displays being asleep), **not** API
  breakage — the identical call succeeds on the same OS build when displays are awake. The asleep state was
  not re-created directly, and which of the three calls failed then stays unattributed.
- The fallback (host timer at the screen's `maximumFramesPerSecond`, 30..240 else 60 Hz, same
  `vsync_signal()` the display-link callback uses) was **not exercised**: both runs had their display link up.
- Windowed run (log kept): `window drawable 1815x1017 (scale 1)` then **`[nfs] display link running
  (nominal refresh 0.0083335s)`** — the S1.3 FAIL line passes and the guest thread starts.

### The headless blocker: VI requires a native window

- `vi_initialize` hard-failed whenever no window had been built (`metalLayer` unset), which is every
  headless boot. The VI display now gets an **offscreen CAMetalLayer stand-in** (1920x1080, the docked
  output size the windowed path opens) so the guest's `GetNativeWindow` hands out a real layer and the boot
  stays observable without one; nothing reaches a screen through it. The probe showed an offscreen layer
  yields drawables in this session.
- Headless run (log kept): `[nfs] VI display initialized on an offscreen layer 1920x1080 (headless: no
  native window)` — the S1.3 FAIL line passes and the boot continues into the NVN chain.

### The required NVN surface

Each row is tied to the S0.3 chain evidence (addresses) or the S2.1 boot logs; nothing extends past them.

| NVN entry | How the chain reaches it | Runtime (S2.1 runs) | HLE status |
| --- | --- | --- | --- |
| `nvnBootstrapLoader` | `main` import; PLT stub `0x7f3c00`, call `0x76e6f0` (S0.3) | **called** with `"nvnDeviceGetProcAddress"` (both logs) | HLE_FORCE takeover (`hle_nvn.c`) |
| `nvnDeviceGetProcAddress` | name at `0x916c3f`, the chain's argument (S0.3) | **resolved and used**: 516 name-table entries taken through it | semantic (`hle_nvn.c:146`) |
| `nvnDeviceInitialize` | name resolution `0x76a284` in `sub_76a220` (S0.3) | **resolved, then called** (OLOG) | semantic |
| `nvnQueuePresentTexture` | name resolution `0x76a8c0` (S0.3) | resolved; **not called** (no present reached) | semantic (`hle_nvn.c:2877`) |
| `nvnWindowAcquireTexture` | name resolution `0x76aa9c` (S0.3) | resolved; not called | semantic (`hle_nvn.c:2463`) |
| `nvnWindowSetPresentInterval` | name resolution `0x76aaf0` (S0.3) | resolved; not called | semantic (`hle_nvn.c:2418`) |
| dispatch through `[0xd37000+0x3d0]` | `blr x8` at `0x76e714` (S0.3) | **not established** (indirect) | — |

Runtime-observed beyond the static set: `nvnQueueInitialize` (called), `nvnDeviceGetInteger` (called),
`nvnDeviceSetDepthMode` and `nvnDeviceSetWindowOriginMode` (called). The boot log's inventory line stands:
523 names, 424 implemented (417 semantic + 7 nop), 99 report-on-call red stubs.

### The chain at runtime: reach and stop

Both modes agree line-for-line from the bootstrap call to the stop.

1. `[nfs] nvnBootstrapLoader("nvnDeviceGetProcAddress")` — the endpoint string S0.3 found at `0x916c3f`
   is what the game passes.
2. **516 of 523** name-table entries resolved through `nvnDeviceGetProcAddress`, each once (first-seen log,
   `DM_NVN_LOG=1`, `hle_nvn.c` `nvn_lookup`). The **7 never requested** are all debug-checkpoint entries:
   `nvnCommandBufferInsertCheckpointStatic/Dynamic`, `nvnDeviceSetAutomaticCheckpointsEnable`,
   `…SamplingInterval`, `…ByDebugGroup`, `nvnDeviceSetCheckpointCallStacksEnable`,
   `nvnDeviceSetCheckpointMaxCallStacksDepth`. No unknown name was queried (zero red reports).
3. Called after resolution: `nvnDeviceInitialize`, `nvnQueueInitialize`, `nvnDeviceSetDepthMode`,
   `nvnDeviceSetWindowOriginMode`.
4. `nvnDeviceGetInteger` answered **96 properties, `0x00`–`0x5f`** (the Tegra X1 measured table), then
   **stopped at `0x60`** — `die("unsupported NVN device property 0x60")`, exit 2. The header names the
   constant: `NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID` (`nvn.h:328`; `0x61` is
   `NVN_DEVICE_INFO_EVENTS_SUPPORT_REDUCTION_OPERATIONS`). The HLE table ends at `0x5F` because the
   measured NX values end there; what the Switch answers for `0x60`+ is not established.

Not observed: any `nvnQueuePresentTexture`/`nvnWindowAcquireTexture`/`nvnWindowSetPresentInterval` call,
the indirect first-present target, any present, frame, audio, or gameplay.

### Claim separation

- Statically reachable (S0.3, unchanged): the table's "how the chain reaches it" column.
- Runtime-observed (S2.1 logs): the endpoint call and argument, 516 resolutions, the four calls in step 3,
  96 property answers, the stop at `0x60`.
- Resolved-or-split with evidence: both window-path blockers (cleared; the timer fallback and the asleep
  attribution remain the split parts, both bounded above).
- Provider/identity questions: S1.3's boundary stands; nothing here extends it.

### Gate

`python3 -m unittest discover -s tests` — 255 tests, 1 skipped, exit 0. Boundary check on this slice's diff
(`games/nfs/runtime/frontend.m`, `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c`): linkage and paths clean —
no reach into `games/totk` or `games/by3`; the new calls are system frameworks the Makefile already links.

### Closeout

Closes-when met: the required NVN surface is mapped with each mapping tied to a recorded observation, both
window-path blockers are cleared with the split parts bounded, the S0.3 chain's runtime reach is recorded to
its stop at device property `0x60`, the configured baseline passes, and no claim extends past what was run.
Splits carried: (a) the device-constant surface past `0x5F` and what the game does after `0x60` is answered
→ next slice; (b) the indirect first-present target and a runtime first-present trace; (c) the
`sub_63fea0` indirect/thread entry path → S3.2; (d) the timer-vsync fallback is unexercised; (e) the asleep
display state that produced S1.3's -6661 was not re-created.

## S2.1 source references

- `.babysteps/scratch/S2.1-NFS-nvn-surface/{surface-map.json,boot-headless.log,boot-windowed.log,dlprobe.m,dlprobe.out}` — the map record, both bounded run logs, and the display-link probe.
- `games/nfs/runtime/frontend.m` — display-link fallback and the offscreen VI layer (`vsync_signal`, `vi_initialize`).
- `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c:104` — `nvn_lookup` first-seen resolution log (DM_NVN_LOG).
- `games/nfs/third_party/d2r_gfx/gfx/nvn/include/nvn.h:321-331` — `NVNdeviceInfo` values naming the stop.

## S2.3 — Boot through HID and stop at `friend:u`

Headless boot after the HID implementation passes the first HLE GPU frame and opens `hid`. Only HID command 101 is observed in this run; it is handled, then two more guest threads are created and the focus policy is stored. Commands 0–12 are not exercised here. The run stops when the service manager is asked for `friend:u`; no friend-service command is observed.

### Account path and first stop

- The first run faulted at `sdk+0x21a0` with `x0=0`, exit 139. The instruction is in `nn::account::ListOpenUsers`, which dereferences the SDK application-account service holder at SDK `.bss` `+0xc70a30`.
- The caller is `nn::friends::detail::CheckUserStatus`, reached from `nn::friends::NotificationQueue::Initialize`. The local account override replaces `nn::account::Initialize` but does not create the SDK `acc:u0` holder; allowing the original `ListOpenUsers` to run therefore dereferenced null state.
- `games/nfs/runtime/account.c` now overrides `ListOpenUsers` against the local UID/open-handle state. The rerun passed that call and advanced to the next stop.

### Next stop

- The rerun stops at `[totk-ipc] sm: service "friend:u" not implemented`, followed by `FAIL: IPC service not implemented` (exit 1).
- The recorded callers pass through `nn::friends::NotificationQueue::Initialize`, `nn::friends::detail::ShimLibraryGlobal::CreateNotificationService`, `nn::friends::detail::ipc::Initialize`, `nn::sm::GetServiceHandle`, and the HIPC service-handle processor. The registry has no `friend:u` handler, so no friend IPC command is reached.
- No gameplay or final picture is claimed. The logs show only the previously recorded first GPU frame and the new service-open boundary.

### Evidence and gate

- `.babysteps/scratch/S2.3-NFS-hid/{boot-before-account-override.log,boot-before-account-override-svc-trace.log,boot-final.log,gate-final.log}` preserve the before/after traces and full gate.
- The nested work folder uses `WORK=.` for `make prepare` and `make runtime`; the Makefile default `WORK=NFS_build` points one level too deep in this copy.
- Gate: `make prepare WORK=.` and `make runtime WORK=.` succeeded; `python -m unittest discover -s tests` passed 255 tests, 1 skipped, with the known non-fatal subprocess `ResourceWarning`.

## S2.4 — Resolve the `friend:u` service open (2026-10-22)

Implementation and observation slice. The S2.3 `friend:u` service-open blocker is resolved: the runtime now
registers `friend:u` and answers the friend notification-queue path the game reached. The boot advances past
the notification queue to a new stop — a null dereference in the friend-invitation notification path, which
reaches `nn::am::GetApplicationFunctions()`. No game picture was observed, so no visual-correctness or
gameplay claim is made.

### The `friend:u` service, and the notification path it answers

`games/nfs/runtime/friends_service.c` registers `friend:u` (`runtime_ipc_register("friend:u", …)`), the
`nn::friends::detail::ipc::IServiceCreator` of switchbrew "Friend services". The headless boot, in order:

- `friend:u` opens; `CreateFriendService` (cmd 0) returns an `IFriendService` move handle — the shim stores it
  and sends no command on it.
- `CreateNotificationService` (cmd 1, 16-byte userID) returns an `INotificationService` move handle.
- `INotificationService` `GetEvent` (cmd 0) returns a never-signalled readable event (copy handle); `Clear`
  (1) and `Pop` (2) are no-ops because offline there is nothing queued.

`IFriendService` and the creator's `CreateDaemonSuspendSessionService` (2) are report-on-call stubs: the
notification path does not reach them, and any command they do receive still stops the runtime with its id —
no unobserved command is implemented.

### The next stop

After the notification queue initializes, the boot creates four more guest threads, then faults at
`sdk+0x19d210` (SIGSEGV, fault `0x0`) inside `nn::friends::GetFriendInvitationNotificationEvent`
(`sdk+0x19d1b0`). The faulting instruction `ldr x8, [x0]` dereferences the return of
`nn::am::GetApplicationFunctions()` (`_ZN2nn2am23GetApplicationFunctionsEv`, the `bl` at `0x19d20c`), which is
NULL: the applet-manager's application-functions interface was never initialized, and
`games/nfs/runtime/applet_host.c` overrides `GetAppletResourceUserId` but not `GetApplicationFunctions`.

The friend-invitation notification path therefore does not go through `friend:u`; it goes through the applet
manager (`nn::am`). Resolving `friend:u` unblocks the notification queue, not friend-invitation notifications.

### Not observed

No game-rendered picture: GPU submission stays disabled by default (`TOTK_GPU_ALLOW_SUBMIT=1` re-enables it;
the first GPU frame is encoded but never submitted). No present, frame, audio, or gameplay is reached.

### Gate

`python -m unittest discover -s tests` passed 255 tests, 1 skipped, with the known non-fatal subprocess
`ResourceWarning`. Boundary: `friends_service.c` is self-contained in `games/nfs/runtime/` (includes
`runtime.h` and `ipc.h` only); no reach into `games/totk` or `games/by3`.

### Source references

- `.babysteps/scratch/S2.4-NFS-friends/boot.log` — the headless boot log (friend:u opens, notification path
  answers, then the `nn::am::GetApplicationFunctions` null dereference).
- `.babysteps/scratch/S2.4-NFS-friends/gate.log` — the passing baseline.
- `games/nfs/runtime/friends_service.c` — the new service.
- switchbrew "Friend services" — the `IServiceCreator`/`INotificationService` command layouts.

### Closeout

Closes-when met: the `friend:u` service-open blocker is resolved and the resulting run is recorded to its next
stop; no visual-correctness or gameplay claim. The friend-invitation notification path
(`nn::am::GetApplicationFunctions()` returning NULL) is the next blocker, split to the next slice.

## S2.4b — Resolve the friend-invitation notification event null dereference (2026-10-22)

`games/nfs/runtime/applet_host.c` overrides `nn::friends::GetFriendInvitationNotificationEvent`
(`_ZN2nn7friends36GetFriendInvitationNotificationEventEv`) to bind the SDK's process-global friend-invitation
`SystemEventType` (`sdk+0xcabfb0`) to a never-signalled host event (`hle_bind_system_event`) instead of letting its
lazy init dereference the NULL `nn::am::GetApplicationFunctions()`. Offline there are no friend invitations, so the
event never fires.

The resulting headless boot passes `friend:u` and the friend-invitation path (log: `friend-invitation notification
event bound, never signalled (offline)`), reaches the game's own environment init (`EnvironmentService`,
`AppLifecycle`, `DirectorService`), and begins shader translation. It stops at the shader translation service: neither
the native translator (`build/shader-service/totk_xlate`) nor the Python script (`scripts/shader_service.py`) is
carried in this tree (`totk_shader_rt.m` `service_start`), so the game's first shader request (`c10b455d…`) fails and
the boot exits (code 2). The shader tooling arrives with the app-assembly slice.

No game-rendered picture is observed (GPU submission stays disabled), so no visual-correctness or gameplay claim.
Source references: `.babysteps/scratch/S2.4b-NFS-friends-event/boot.log`; commit `c493904`. The shader-translation
service is the next blocker, split to S2.4c.

## S2.4c — Carry the shader translator and translate a shader toward a frame (2026-10-22)

The S2.4b stop (the absent shader translator) is resolved by carrying the shader tooling into `games/nfs/`, exactly as
`games/by3/` does: the prebuilt NativeAOT translator is forked to `games/nfs/prebuilt/macos-arm64/{totk_xlate,
totk_xlate.sha256,README.md}` (the BY3 copy, which the NFS graphics layer is byte-identical to), and the NFS `Makefile`
gains the `shader-tools` target (copied from `games/by3/Makefile`): it verifies the prebuilt against its SHA-256 and
copies it to `<work>/build/shader-service/totk_xlate`, and builds `<work>/build/spirv-cross/spirv-cross` from the pinned
SPIRV-Cross commit `aa217aeb6c9f0ace7a0ab233b28807edf45eb165` with the same sources and flags as `build_app.py`. `make
boot` now depends on `shader-tools`, so the headless boot has a translator on first use.

### Resulting run

The headless boot (`.babysteps/scratch/S2.4c-NFS-shader-translator/boot.log`, 198 lines) now starts the native service
(`runtime translation service started (pid …, native build/shader-service/totk_xlate)`) and translates **8 shaders**, the
first being the S2.4b failure `vs c10b455d…cf7f66b934a8`. With the shader stop gone, the game runs its render loop:
**300 frames encoded** — frame 1 is 27 functions / 32 calls, frame 2 is 120 functions / 4,251 calls (13
`CommandBufferBindTexture`, 12 `CommandBufferDrawArrays`, 6 `ProgramSetShaders`, …), and the loop settles at ~33 ms a
frame (p50 33.33 / p95 33.46 ms over 299 frames). The boot then opens `audout:u` (`AudioOut open 48000 Hz x6 PCM16 ->
Core Audio (stereo)`) and stops at `[nfs] FAIL: AudioOut buffer bounds` (`audio_out.c:94` `append`:
`data_offset + data_size <= capacity` fails on the first `AppendAudioOutBuffer`).

No game-rendered picture is observed: `GPU submission disabled: work is encoded but never submitted (no picture, no GPU
results); TOTK_GPU_ALLOW_SUBMIT=1 re-enables it`. So no visual-correctness claim; the frames are NVN-encoded, not Metal
submitted. No gameplay claim.

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`.
- Boundary (scope: working tree vs HEAD — `games/nfs/Makefile`, `games/nfs/README.md`, new `games/nfs/prebuilt/`):
  linkage clear (the `shader-tools` target clones `github.com/KhronosGroup/SPIRV-Cross` and compiles with clang++; no call
  into `games/totk`/`games/by3`; the prebuilt binary holds no predecessor path string — `strings` finds none); paths clear
  (the only predecessor names are provenance text in `prebuilt/.../README.md`, the same pattern as BY3's).
- Scratch: `.babysteps/scratch/S2.4c-NFS-shader-translator/boot.log`. The stale `games/nfs/NFS_build/` (7 GB, flagged by
  S2.4b) was removed, and the gate's temp now lands on the external volume (`TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames`;
  see `.operator/guides/external-tmp.md`) — the S2.4b gate was RED only because the system data volume was full.

The `AudioOut buffer bounds` stop is the next blocker, split to S2.5.

## S2.5 — Resolve the `AudioOut buffer bounds` stop (2026-10-22)

The stop was a misread descriptor field, not an out-of-range buffer. The first `AppendAudioOutBuffer` (command 7)
carries a 0x28-byte input buffer holding `next=0xcdcdcdcdcdcdcdcd, sample_buffer=0x70d03d5000, capacity=0x1000,
data_size=0xc00, data_offset=0xcdcdcdcdcdcdcdcd`: the sample buffer, capacity (4096 bytes) and data size (3072 bytes)
are the game's, while `next` and `data_offset` hold allocator fill. The SDK copies all 0x28 bytes of
`nn::audio::AudioOutBuffer`, but this title never initialises `data_offset`, and the service accepts the descriptor —
so the host's unconditional `data_offset + data_size <= capacity` failed on a field that is not data.

`games/nfs/runtime/audio_out.c` `append` now treats an offset greater than the capacity as unset (a legitimate offset
cannot exceed the buffer), logs it once per boot, and applies the bounds check to the normalised offset. A descriptor
whose offset fits the buffer but whose data runs past it still fails. `NFS_AUDIO_TRACE=1` prints the first four
descriptors — command, tag, input buffer, all five words, and the offset actually used.

### Resulting run

After the change the 60 s bounded headless boot (`.babysteps/scratch/S2.5-NFS-audio-bounds/boot3.log`, 238 lines)
opens `audout:u`, appends four buffers, starts the output, and keeps playing: no queue-full, so
`GetReleasedAudioOutBuffers` returns buffers and the queue drains. The render loop reaches frame 900+ at p50
33.33 ms, translating two more shaders (`vs bb6f978b…`, `ps 34ebe360…`), and records **no next stop** — the run was
ended by the timer, not by a failure. GPU submission stays disabled, so no picture and no visual-correctness claim;
no gameplay claim.

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, the known non-fatal subprocess `ResourceWarning`.
- Boundary (scope: this slice's diff, `games/nfs/runtime/audio_out.c`): linkage clear (no added include, no call into
  `games/totk` or `games/by3`); paths clear (no absolute or predecessor path in the diff).
- Scratch: `.babysteps/scratch/S2.5-NFS-audio-bounds/{reconstruct.log,boot.log,boot3.log,rebuild*.log,gate.log}`. The
  work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and not evidence.

The absence of a stop makes the first submitted frame the next observation: S2.6.

## S2.6 — Submit the encoded frames and observe the first picture (2026-10-22)

The `TOTK_GPU_ALLOW_SUBMIT=1` gate (`nvn_mtl.m:147-151`) was opened for four bounded headless boots (90 s, 80 s, 75 s,
55 s; `.babysteps/scratch/S2.6-NFS-first-frame/boot1.log`–`boot4.log`). Submission is real: frame 1 carries one
`QueueSubmitCommands` and one `QueuePresentTexture`, and the timing line reports **GPU p50 0.86 ms / p95 1.21 ms over 292
frames** (boot1) and **p50 1.45 / p95 1.74 over 300 frames** (boot2), against 0.00 ms when submission was disabled. No
Metal validation failure, draw error, crash, or kernel panic was recorded in any run; every run was ended by its timer.

The picture path works. `shot` at frame 2 wrote a 1920x1080 PNG that is pure black (mean 0, max 0); the 5 s shot (frame
144) is also black. The 15 s shot (**frame 441**) is the first non-black capture: mean 0.000697, max 0.996, a small white
bar in the lower right (`f15.png`). That is what the captured frame shows; no further visual-correctness claim.

A new stop appears after the picture. Presenting stops between **frame 354 and 600 (12–20 s)** across the runs (boot1
stopped at 354, boot2 at 600): the 30 s, 50 s and 65 s shots produce no file. A 40 s thread dump (`boot3.log`) shows guest thread `0x1002` on core 0 for **16.7 s** with
`wait=0` and `nsvc=71,196,080`, while the other guest threads wait; the process stays alive until the timer. The 55 s
`hletrace` run (`boot4.log`) records **460 `QueueSubmitCommands` and 460 `QueuePresentTexture`** calls and 154,805 host
calls to 214 functions. The cause is not established; the spin, not a Metal error, is the next observation.

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0, the known non-fatal `ResourceWarning`.
- Boundary: this slice changed no code. Linkage and paths are therefore clear on the diff; the record names only
  `games/nfs/...` and the external work tree.
- Scratch: `.babysteps/scratch/S2.6-NFS-first-frame/{boot1.log,boot2.log,boot3.log,boot4.log,frame2.png,f05.png,f15.png,gate.log}`.

The present stall is the next port blocker; S2.7 is the operator's requested log-language pass.

## S2.7 — Make the host diagnostics emit English (2026-10-22)

The host surface held **721 Chinese string literals across 22 files** and 61 `TOTK_UI("中文","English")` pairs. The first
same-line inventory found 316 (266 `MLOG`, 44 `printf`, 6 `NSString`); the rest sat in multi-line `MLOG`/`snprintf` calls,
including the `[nvn]` frame summaries in `hle_nvn.c`. All 721 were translated with every format conversion and argument
preserved (the conversion sequences were compared mechanically before applying), and comments and identifiers were left
alone. `totk_ui_lang.h` now returns English unless `TOTK_UI_LANG` starts with `zh`; the game's own language stays on
`TOTK_LANG`.

Two bounded headless boots (25 s, 35 s) with `TOTK_GPU_ALLOW_SUBMIT=1` and `TOTK_UI_LANG` unset each wrote a 191-line
stderr log with **zero non-ASCII characters**; the frame-300 summary reads `[nvn] timing to frame 300: ...` in English.
No crash line was recorded and each run ended by its timer. The first stopped at **frame 399 (13.6 s)**, the second at
**frame 354 (12.0 s)** — the same present stall S2.6 saw, not a regression. Those stops also show S2.6's "frame 600" was
one run's stop, not the stall's frame; the observed range is **354–600** (Do-Not #8).

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0 (`.babysteps/scratch/S2.7-NFS-english-logs/gate.log`).
- Boundary: all 22 changed code paths are under `games/nfs`; no `games/totk`/`games/by3` reference and no build/script
  change. Outside `totk_ui_lang.h` the diff is string content only (verified by masking every string literal and comparing
  the remaining source with HEAD). The language selector no longer calls `CFLocale`; no new linkage.
- Scratch: `.babysteps/scratch/S2.7-NFS-english-logs/{boot2.log,boot3.log,build.log,build2.log,gate.log}` is evidence;
  `{mlog.tsv,printf.tsv,translate.tsv,chunk*.tsv,rem*.tsv,translated*.tsv,rem*-en.tsv,boot*.stdout}` is the removal list,
  not deleted.
- Code commit `13d2b2b`.

The present stall is the next blocker: S2.8.

## S2.8 — Locate the NFS present stop (2026-10-22)

Six bounded headless boots (`TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1`, `timeout -k 5`;
`.babysteps/scratch/S2.8-NFS-present-stop/bootA.log`–`bootF.log`) ran the new present-gap watchdog
(`TOTK_PRESENT_WATCHDOG=<ms>`: at the first gap longer than the threshold after frame 2, print one
message and call the guest-thread dump). The watchdog's first-stop message is the record; the stop
time below is its "since start" minus its "no present for", and the frame is the last presented frame.

| boot | watchdog | input | last presented / submissions | no-present gap | stop time | guest thread 0x1002 |
| --- | --- | --- | --- | --- | --- | --- |
| A | 2500 ms | `hletrace 2` | 440 / 440 | 2.6 s | 14.9 s | spin `0x1D`, key word 0 |
| B | 2500 ms | `framestep 16.7` (no-op) + `hletrace 2` | 677 / 677 | 2.6 s | 22.8 s | spin `0x1D`, key word 0 |
| C | 2500 ms | `framestep 50` (no-op) + `hletrace 2` | 676 / 676 | 2.5 s | 22.8 s | spin `0x1D`, key word 0 |
| D | 2500 ms | `hletrace 2` | 545 / 545 | 2.6 s | 18.4 s | wait `0x1C` on the same key; core 0 owner 0 |
| E | 1000 ms | `hletrace 2` | 565 / 565 | 1.0 s | 19.1 s | spin `0x1D`, key word 0 |
| F | 2500 ms | `hletrace 2`, corrected dump | 494 / 494 | 2.6 s | 16.6 s | spin `0x1D`, key word 0 |

The first-stop message for boot A, verbatim:

    [nvn] present stop: last presented frame 440, 440 submissions, no present for 2.6 s (17.5 s since start)

**The stop is not pinned to wall time or frame count.** The six runs stopped at 440, 677, 676, 545,
565 and 494 frames (14.9–22.8 s); with the S2.6 stops (354, 600) and the S2.7 stops (399, 354) the
observed range is **354–677** and 12.0–22.8 s. Identical configurations differ (A 440/14.9 s vs
D 545/18.4 s vs F 494/16.6 s). Two of the runs carried `framestep` (`runtime_frame_step_pin`) — a
no-op in NFS (`games/nfs/runtime/game_patches.c:13`) — so they are not a frame-rate separation
test. Do-Not #8: the range and the runs are the record, not one run's frame.

**A condition-variable handshake is the common state.** In every run `submissions == last presented
frame`: the last submitted frame presented, and the next frame was never submitted, so the stop is
before submission rather than in the present queue or a fence. The created guest thread `0x1002`
(prio 44, core 0) is on the condition variable at **guest heap+0xC284E8A8** (tag `0x1002`; lock word
at `heap+0xC284E8A4`). In A/B/C/E/F it spins in `SignalProcessWideKey` (`0x1D`) on that key — top
counts 11,149,511 / 11,642,620 / 11,084,815 / 4,635,340 / 11,148,046, key word 0; in D it waits in
`WaitProcessWideKeyAtomic` (`0x1C`) on the same key (`wait=2`, `addr=heap+0xC284E8A4`,
`key=heap+0xC284E8A8`) with core 0 owner 0. Per-run addresses differ only by the randomized heap base
(A `0x7c7284e8a8`, B `0x73f284e8a8`, C/E `0x70f284e8a8`, D `0x7bf284e8a8`, F `0x70f284e8a8`).

The render thread `0x1001` (the thread making the NVN calls; `hletrace` tags its calls `h=0x1001`)
stops submitting. In the spin runs it is in the guest lock path (A/F: `wait=0`, last SVC
`0x1A ArbitrateLock` at a nearby lock, A `heap+0xC284E7A4`, F `heap+0xC284E824`); in D it waits on a
different condition (`wait=2`, key `0x16ae6c8f8`, last host call
`nn::oe::TryPopNotificationMessage`). The owner of the condition key and the waiter/signaller pair
are not yet named, and the trigger event is not named; the state is consistent with a lost
wakeup/condition race. No fix is made in this slice.

The last new host call under `hletrace 2` is `nvnCommandBufferDrawElementsBaseVertex` at 13.146 s in
A and `nvnBufferFinalize` at 18.247–18.277 s in B/C/D/E; A and F stop before that finalize, so it is
not causal.

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0, the known non-fatal
  subprocess `ResourceWarning` (`.babysteps/scratch/S2.8-NFS-present-stop/gate.log`).
- Boundary (scope: this slice's diff, `games/nfs/runtime/threads.c` and
  `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c`): linkage clear — the added calls are
  libc/Mach/pthread (`memcpy`, `mach_vm_read_overwrite`, `pthread_create`, `usleep`, `getenv`) and
  the NFS runtime's own weak `runtime_dump_threads` (`games/nfs/runtime/threads.c`), with no
  `games/totk`/`games/by3` include or call; paths clear — no predecessor or absolute path in the
  added lines and no predecessor path in the link line.
- Scratch: `.babysteps/scratch/S2.8-NFS-present-stop/{bootA.log,bootB.log,bootC.log,bootD.log,bootE.log,bootF.log,bootA.stdout,bootB.stdout,bootC.stdout,bootD.stdout,bootE.stdout,bootF.stdout,build.log,build2.log,gate.log,inputA.txt,inputB.txt,inputC.txt}`.
  The work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and not evidence.
- Code commit `ff1a5f4`.

The condition-variable stop is the next blocker: S2.9.

## S2.9 — Name the condition object and fix the lost wakeup (2026-10-22)

Four bounded headless boots (`TOTK_PRESENT_WATCHDOG=2500`, `hletrace 2`;
`.babysteps/scratch/S2.9-cond-object/boot1.log`–`boot4.log`) carried two new
diagnostics: a condition ring (last 24 waits/signals/timeouts, coalesced per
thread+key per 2 ms) and a 0x100-byte memory window around each waiting lock.

**The object is named.** The windows show three 0x80-byte `nn::os::EventType`
objects at `heap+0xC284E788`, `+0xC284E808`, `+0xC284E888` (cs at +0x1c, cv at
+0x20 — the layout `SignalEvent` uses at `sdk+0x338e30`). `heap+0xC284E8A8` is the
third event's condition variable, its critical section at `heap+0xC284E8A4`; the
cs word read `0x1002` — thread `0x1002` inside that event's critical section — and
the same event appears as `wait=2 addr=+0xC284E8A4 key=+0xC284E8A8`.
`TimedWaitEvent` (`sdk+0x33906c`, from `main+0x756e60`) waits on it; `SignalEvent`
(`sdk+0x338e50`, via the refcounted closure runner `main+0x756720`/`0x7568e0`
called from `main+0x756798`) signals it. The trace records
`signal key=...E8A8 by=0x1002 woke=0` (no waiter) while `0x1005` waits and times
out on `...E858a8` every 37 ms.

**The wakeup arrived; the core handoff did not** (Do-Not #9). `0x1001` (mask 0x1,
core 0 only) was woken onto the contended event's critical section (word
`0x40001001` — owner `0x1001` with waiters) and waited in `core_take` for the whole
2.6 s gap while `0x1002` held core 0 and spun `0x1D` 11,095,006 times (boot1;
11,108,312 in boot2). `0x1003`/`0x1004` queued behind the starved main thread's
critical section; the main thread never submitted.

**Fix and verification.** `core_take` marks `yield_eq` for equal-priority co-core
owners; `runtime_core_checkpoint` (SVC entry) rotates once the holder has run
`core_slice_ns` (4 ms); higher-priority preemption is unchanged. boot3: 90 s, zero
present stops, `timing to frame 2400`, p50 33.33 ms; boot4 (30 s): frame 600, zero
stops. Prior record: stops at 354–677. Commits `0c4e0b9` (diagnostics) and
`d8a90e4` (rotation). Gate: 255 tests, 1 skipped, exit 0. Boundary (one file,
`games/nfs/runtime/threads.c`): linkage clear (libc/pthread/Mach and the tree's own
helpers), paths clear. S2.10 is promoted.

## S2.10 — Capture what the boot presents past the old stop (2026-10-22)

Two bounded headless boots carried live `shot` captures past the old stop range
(`TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1 TOTK_PRESENT_WATCHDOG=2500`, `hletrace 2`,
`timeout -k 5`; run scripts `.babysteps/scratch/S2.10-capture-past-677/runA.sh`/`runB.sh`,
both from the work tree `./build/nfs-native rewritten/nfs100 boot`). The input file starts
with `hletrace 2`; `shot <path>` lines are appended while the boot runs (the input thread
polls the file), so each shot fires about two frames after its line is read. Run A
(130 s bound) appended shots at 30/50/80/110 s; run B (185 s bound) at 60/120/170 s.
`make runtime shader-tools` from `games/nfs` with `WORK=<work>/nfs` reported nothing to
do (the work-tree binary, built after `d8a90e4`, was current).

**Captures past frame 677.** The `[shot] F<n>` log line names each capture's presented
frame; every PNG below was also opened and viewed. Run A (`bootA.log`): `F809` fA30.png —
the EA logo on its dark background (mean 0.0636 / max 0.3647); `F1406` fA50.png — the
NFS Hot Pursuit Remastered logo screen over the road perspective (0.1560 / 1.0); `F2307`
fA80.png — motion-blurred racing footage, a red car and a black-and-white police SUV
among trees (0.3120 / 0.9961); `F3206` fA110.png — footage, a desert scene with a police
helicopter and an orange car on a road (0.6889 / 1.0). Run B (`bootB.log`): `F1718`
fB60.png — the logo screen with a visible `PRESS ⊕` prompt (0.1928 / 1.0); `F3518`
fB120.png — the logo screen without a visible prompt (0.1334 / 1.0); `F5019` fB170.png —
footage, a cockpit view with a red car's mirror and a police car behind it on a forest
road (0.3328 / 1.0). Statistics are mean / max 8-bit luma over the 1920×1080 PNG
(`png-stats.py`). No gameplay and no visual-correctness claim beyond these frames.

**No new stop.** Neither run printed a `present stop` line. Run A reached frame 3600 in
130 s and run B frame 5447 in 185 s (12 and 18 timing windows, p50 33.3 ms); the worst
per-window frame-time max is 996.92 ms (run A) and 510.55 ms (run B) — hitches under the
2500 ms watchdog threshold, so the watchdog stayed silent. The prior record (stops at
354–677, 12.0–22.8 s) is superseded by these runs and S2.9's boot3/boot4; report ranges
and runs, not one run's frame (Do-Not #8).

**No code changed.** The slice used the existing `shot` mechanism
(`games/nfs/runtime/input.c` → `dm_gfx_shot_at`); no stop message existed to name a file.

### Gate, boundary, scratch

- `python -m unittest discover -s tests` passed 255 tests, 1 skipped, exit 0, the known
  non-fatal subprocess `ResourceWarning` (`.babysteps/scratch/S2.10-capture-past-677/gate.log`).
- Boundary (scope: this slice's diff — documentation only; no code, script, or build file
  changed): linkage clear — no added code, so no new call or import exists to follow into
  `games/totk` or `games/by3`; paths clear — the slice's diff is documentation; no added
  code, script, or build file exists, and no predecessor path is named for a command or
  code to resolve.
- Scratch: `.babysteps/scratch/S2.10-capture-past-677/{runA.sh,runB.sh,inputA.txt,inputB.txt,bootA.log,bootB.log,bootA.stdout,bootB.stdout,fA30.png,fA50.png,fA80.png,fA110.png,fB60.png,fB120.png,fB170.png,png-stats.py,build.log,gate.log}`.
  The work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and
  not evidence.

The title screen's `PRESS ⊕` prompt is the next probe. S2.11 is promoted.

## S2.11 — Press ⊕ at the title screen and record what changes (2026-10-22)

Two bounded headless boots, `.babysteps/scratch/S2.11-press-plus/run.sh <press|control> <tag>`, ran from
the work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs` via `./build/nfs-native rewritten/nfs100 boot`,
each `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1 TOTK_PRESENT_WATCHDOG=2500 TOTK_INPUT_LOG=1 TOTK_INPUT_FILE=<file>`
under `timeout -k 5 180`. Both modes ran the IDENTICAL wall-clock schedule and differ ONLY in four
`key 36 150` lines (press mode only): `hletrace 1` at t=0; `hletrace 0` then `hletrace 1` at t=45 s; `shot` at
t=48/52/62/72/90/110/145 s; `key 36 150` at t=55/65/75/85 s; `hletrace 0` at t=95 s and t=150 s; `close` at
t=153 s. Both runs ended on the 180 s timer (exit 124); `close` did not end either boot. `make -n runtime
shader-tools` from `games/nfs` with `WORK=<work>` reported nothing to be done, so the work-tree binary was
current. Mac key 36 is Return, which `button()` in `games/nfs/runtime/input.c` maps to button bit 1024 = Plus.

**Run P (press): the title screen gives way to the save-data notice.** The `[shot] F<n>` log line names each
capture's presented frame; every PNG was also opened directly, and the statistics are mean / max 8-bit luma
over the 1920×1080 PNG. At t=48 s `F1371` P-pre1.png — the NFS Hot Pursuit Remastered logo screen over the
road perspective, no prompt; 0.1520 / 1.0. At t=52 s `F1487` P-pre2.png — the same logo screen with a visible
`PRESS ⊕` prompt; 0.1274 / 1.0. At t=62 s `F1788` P-post1.png — a DIFFERENT screen: "THIS ICON INDICATES THE
GAME IS ACCESSING SAVE DATA ON THE SYSTEM MEMORY. DO NOT SWITCH OFF THE POWER WHEN YOU SEE THIS ICON
FLASHING." with a `Press Ⓐ to continue` prompt and the save icon; 0.0608 / 0.996078. At t=72 s `F2087`
P-post2.png — the same notice; 0.0638 / 0.996078. At t=90 s `F2629` P-post3.png — the same notice, save icon
in its bright state; 0.0556 / 0.996078. At t=110 s `F3229` P-post4.png — the same notice; 0.0633 / 0.996078.
At t=145 s `F4276` P-post5.png — the same notice, save icon in its dim state; 0.0564 / 0.996078. No gameplay
claim and no visual-correctness claim beyond these frames.

**Run P: no stop, and the guest reads the pad.** Zero `present stop` lines; frame 5100 at the 180 s timer,
p50 33.33 ms, worst per-window frame-time max 203.88 ms — under the 2500 ms watchdog. Windowed `hletrace`
counts (`runtime_hle_trace_set` in `games/nfs/runtime/hle_stack.c:80` baselines `since[]` when tracing turns
on and prints `counts[i] - since[i]` when it turns off, so a `1 … 0` bracket is exactly that window): 0–45.0 s
— 493,520 calls to 215 functions, `nn::hid::GetNpadStyleSet` 48,868, `nn::hid::GetNpadState` 5,143;
45.1–95.1 s, all four presses inside — 587,050 calls to 92 functions, `GetNpadStyleSet` 57,038,
`GetNpadState` 6,004; 95.1–150.1 s — 657,980 calls to 85 functions, `GetNpadStyleSet` 62,700, `GetNpadState`
6,600. That is about 120 `GetNpadState`/s and 1,140 `GetNpadStyleSet`/s. `TOTK_INPUT_LOG=1` records
`[totk-input] game reads full-key state for controller 0 (supported styles 0x7, ids allowed 1)`; run P logs
`[totk-input] file: key 36 for 150 ms` four times, run C none.

**Run C (control): no capture at all, and a NEW all-idle stop.** Run C wrote no capture — no `[shot]` line
appears in its log — because the stop fired before the first capture (t=48 s). Message verbatim:

    [totk-gfx] [nvn] present stop: last presented frame 1105, 1105 submissions, no present for 2.6 s (39.7 s since start)

Presentation never resumed: the last frame number anywhere in the log is 1105. State at the dump: every core
idle (`[totk-dump] core 0 owner 0 for 0.0 ms`, and cores 1 and 2 the same); every guest thread parked on a
condition whose tag is its own handle (`run wait=2`), including the main thread `tid=2382712 h=0x1001
prio=44 nsvc=65228 addr=heap+0xC284E8A4 key=heap+0xC284E8A8 tag=0x1001 word@key=0x1`, whose last host call
while tracing was `nn::oe::TryPopNotificationMessage(unsigned int*)`; `h=0x1002` nsvc=221103 and `h=0x1003`
nsvc=73212. No thread is spinning — unlike the S2.8/S2.9 stops, where `0x1002` held core 0 and spun `0x1D`
about 11M times. The `[cond-trace]` ring's last 24 events all sit inside the 13.7–13.8 ms before the dump,
several of them signals that woke nobody (`woke=0`). The stall's base rate is unknown: S2.10's two runs
(130 s, 185 s) and S2.9's boot3/boot4 saw none, while 1 of S2.11's 2 runs stopped.

**Attribution, and its limit.** The press moved the boot from the `PRESS ⊕` title screen to the save-data
notice. In the same run the prompt was up three seconds before the press (`F1487`, t=52 s), and the post-press
screen is not an attract-loop screen: S2.10's 7 captures across 2 runs show only the EA logo, the logo screen
and racing footage. The game then waits on that notice for the rest of the run (62 s to 180 s) with its icon
flashing (bright at `F2629`, dim at `F4276`) — live and animating, not a frozen frame. Limitation, stated
plainly: run C died at 39.7 s so no control reached the comparison window, and the attribution therefore rests
on the same run's own before-capture, on the attract loop's known screens and on the notice gating on a
different button (Ⓐ) — not on a control.

**Save data.** No `SaveData` function appears in any windowed count, and `games/nfs/runtime` contains no
`SaveData` HLE at all — so this records what the runtime served, not what the game attempted. No save-data
host call is claimed.

**No code changed.** `shot`, `key`, `down` and `up` are the existing `games/nfs/runtime/input.c` mechanisms;
no stop message existed to name a file, and the new stop is recorded, not fixed.

### Gate, boundary, scratch

- Verbatim: `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passed
  255 tests, 1 skipped, exit 0, the known non-fatal subprocess `ResourceWarning`
  (`.babysteps/scratch/S2.11-press-plus/gate.log`).
- Boundary (scope: this slice's diff — documentation only; no code, script, or build file changed): linkage
  clear — no added code, so no new call or import exists to follow into `games/totk` or `games/by3`; paths
  clear — the slice's diff is documentation; no added code, script, or build file exists, and no predecessor
  path is named for a command or code to resolve.
- Scratch: `.babysteps/scratch/S2.11-press-plus/{run.sh,input-P.txt,input-C.txt,emit-P.log,emit-C.log,bootP.log,bootC.log,bootP.stdout,bootC.stdout,P-pre1.png,P-pre2.png,P-post1.png,P-post2.png,P-post3.png,P-post4.png,P-post5.png,png-stats.py,png-stats.txt,gate.log}`.
  The 7.2 GB work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and not evidence.

The new all-idle present stop is the next probe. S2.12 is promoted.

## S2.12 — Diagnose the all-idle present stop (2026-10-22)

Eight bounded headless boots, `.babysteps/scratch/S2.12-all-idle-stop/probe.sh <n> <bound>` driving
`run.sh <tag> <bound>`, ran from the work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs` via
`./build/nfs-native rewritten/nfs100 boot`, each `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1
TOTK_PRESENT_WATCHDOG=2500 TOTK_INPUT_LOG=1 TOTK_INPUT_FILE=<file>` under `timeout -k 5 65`. All eight
ended on the 65 s timer (exit 124). The schedule is the control — no presses — and matches S2.11 run C up
to t=45 s: `hletrace 1` at t=0; `hletrace 0` then `hletrace 1` at t=45 s; `threads` at t=45 s and t=55 s;
`shot` at t=62 s; `hletrace 0` at t=63 s. `make -n runtime shader-tools` from `games/nfs` with `WORK=<work>`
reported nothing to be done, so the work-tree binary was current.

**Reproduced in 1 of 8.** Run r4 printed the stop, verbatim:

    [totk-gfx] [nvn] present stop: last presented frame 1218, 1218 submissions, no present for 2.6 s (43.4 s since start)

The seven runs that did not stop (r1, r2, r3, r5, r6, r7, r8) printed zero `present stop` lines, ran the
full 65 s, and their t=62 s `[shot]` lines name frames 1759, 1781, 1801, 1851, 1856, 1851, 1802 (last
completed summary block "to frame 1800"; r4's is "to frame 1200").

**r4 never resumed.** No `[shot]` line appears anywhere in its log; the last frame number anywhere in its
log is 1218; its second hletrace window (`hletrace 0` at t=63 s) printed `0 calls to 0 functions`; its load
line after the stop reads `T 0%` with the host audio thread the busiest.

**r4's dump: all idle, no signal.** Every core reads `owner 0`; every guest thread is `wait=2` on a
condition whose tag is its own handle; no thread spins. The four frame-loop threads
`0x1002`/`0x1003`/`0x1004`/`0x1005` cycle `timeout`+`wait` pairs on their own condition keys
(`0x70f284e8a8`, `0x70f284e828`, `0x70f284e7a8`, `0x70f28558a8`) with wait timeouts x3 ≈ `0x1f78a0b` ns ≈
33 ms. No `signal` event appears in any of r4's three rings (the watchdog dump at 43.4 s and the scheduled
dumps at t=45 s and t=55 s).

**The pair, named from a dump.** The main thread `0x1001` (prio 44, mask `0x1`) parks untimed
(`x3=0xffffffffffffffff`), tag `0x1001`, on `addr=0x16b3f48a4 key=0x16b3f48a8` — the third
`nn::os::EventType`'s cs/cv layout S2.9 named — last host call `nn::oe::TryPopNotificationMessage`, guest
chain `main+0x34a03c <- main+0x352214 <- main+0x366004 <- main+0x27eb3c <- main+0x27e328`. The signallers
are the frame workers `0x1003`/`0x1004` (their wait stack is `sdk+0x33906c` = `TimedWaitEvent`, entered
from the worker loop `main+0x756e64`); in every healthy run's dumps they signal the main thread's key and
wake it: r1 `signal key=0x16b7e0ae8 by=0x1003 woke=0x1001` and `by=0x1004 woke=0x1001`; r2 `0x16f10cae8`
by `0x1003`/`0x1004`; r3 `0x16b4348a8` by `0x1003`/`0x1004` and `0x16b4348b8` by `0x1002`/`0x1003`; r5
`0x16fd70ae8` by `0x1004` (twice) and `0x16fd708b8` by `0x1004`; r6 `0x16d2bcae8` by `0x1003`/`0x1004`; r7
`0x16f200ae8` by `0x1004` (twice) and `0x16f2008b8` by `0x1003`. In r4 no such signal exists, and no event
on the main thread's key appears at all.

**The condition window matches S2.11 run C's.** r4's `[cond-mem] lock=0x16b3f48a4 key=0x16b3f48a8
holder=0x1001` has the same field layout as run C's (`lock=0x16fd088a4 key=0x16fd088a8`), differing only in
the run's addresses.

**Rate.** Across the post-fix watchdog runs — S2.9 boot3/boot4, S2.10 A/B, S2.11 P/C and S2.12 r1–r8 — 2
stops in 14 bounded boots: 39.7 s (frame 1105, S2.11 run C) and 43.4 s (frame 1218, S2.12 r4); the S2.12
batch alone 1 of 8 at 65 s bounds. Report the range and the runs (Do-Not #8).

**Not the S2.9 shape.** No thread spins and every core reads `owner 0`; the S2.9 equal-priority rotation is
untouched (Do-Not #9).

**No code changed.** The slice changed no code, script, or build file; the stop is recorded, not fixed.

### Gate, boundary, scratch

- Gate GREEN: `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests` passed
  255 tests in 22.967 s, 1 skipped, exit 0, the known non-fatal subprocess `ResourceWarning`
  (`.babysteps/scratch/S2.12-all-idle-stop/gate.log`).
- Boundary (scope: this slice's diff — documentation only; no code, script, or build file changed): linkage
  clear — no added code, so no new call or import exists to follow into `games/totk` or `games/by3`; paths
  clear — the slice's diff is documentation; no added code, script, or build file exists, and no predecessor
  path is named for a command or code to resolve.
- Scratch:
  `.babysteps/scratch/S2.12-all-idle-stop/{run.sh,probe.sh,probe-summary.txt,r1..r8.input,r1..r8.emit,r1..r8.log,r1..r8.stdout,r1-end.png,r2-end.png,r3-end.png,r5-end.png,r6-end.png,r7-end.png,r8-end.png,gate.log}`
  is evidence; the 7.2 GB work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable and
  not evidence.

The missing wakeup is the next probe. S2.13 is promoted.

## S2.13 — Instrument the transition and read it on the first reproduced stop (2026-10-23)

Twenty bounded headless boots, `.babysteps/scratch/S2.13-missing-wakeup/probe.sh 8 65` (r1–r8) then
`probe-more.sh 9 12 65` (r9–r20), each driving `run.sh <tag> 65` from the work tree
`/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work/nfs` via `./build/nfs-native rewritten/nfs100 boot`,
each `TOTK_HEADLESS=1 TOTK_GPU_ALLOW_SUBMIT=1 TOTK_PRESENT_WATCHDOG=2500 TOTK_INPUT_LOG=1
TOTK_INPUT_FILE=<file>` under `timeout -k 5 65`. The schedule is S2.12's control unchanged — no presses.
All 20 ended on the 65 s timer (exit 124); every run reported `badcmd=0`.

**The instrument (no behaviour change).** `games/nfs/runtime/threads.c`: `CTRACE_MAX` 128 → 16384 and a
new `CTRACE_DUMP` 4096 entries printed per dump (was a hard-coded 24), so a dump can see ~18 s of stuck
churn or ~4 s of healthy churn instead of 0.6 s. The 128-entry ring could not see the transition: the
dump lands 2.6 s after the stop begins (Do-Not #12).

**Reproduced in 1 of 20.** Run r17 printed the stop, verbatim:

    [totk-gfx] [nvn] present stop: last presented frame 1706, 1706 submissions, no present for 2.6 s (59.7 s since start)

The other 19 printed zero `present stop` lines and ran the full 65 s; their t=62 s `[shot]` frames are r1
1768, r2 1798, r3 1800, r4 1769, r5 1721, r6 1691, r7 1651, r8 1794, r9 1691, r10 1793, r11 1773, r12
1755, r13 1709, r14 1802, r15 1853, r16 1787, r18 1769, r19 1796, r20 1748 (range 1651–1853). r17 has no
`[shot]` line anywhere.

**Rate.** Across all post-fix watchdog runs: 3 stops in 34 bounded boots — 39.7 s / frame 1105 (S2.11 run
C), 43.4 s / frame 1218 (S2.12 r4), 59.7 s / frame 1706 (S2.13 r17). Report the range and the runs
(Do-Not #8).

**The transition, captured for the first time.** r17's third dump (the watchdog at 59.7 s) prints a ring
spanning `-3938.0 ms` to `0 ms`. Its last events on the completion key are `-2579.7 ms signal
key=0x78f284e828 by=0x1001 woke=0x1003`, `-2579.7 ms signal key=0x78f284e7a8 by=0x1001 woke=0x1004`,
`-2579.7 ms signal key=0x78f284e8a8 by=0x1001 woke=0x1002` (main kicks all three frame workers), then
`-2579.7 ms wait key=0x16bc68b18 by=0x1001`, then each worker's own `wait` on its own key (`0x1004`,
`0x1003`, `0x1002`). **Nothing touches `0x16bc68b18` again**: the ring's remaining 562 entries (2.55 s)
are only the four workers' `timeout`+`wait` cycles (~33 ms each) — `0x1002`/`0x1003`/`0x1004` on their own
keys and `0x1005` on `0x78f28558a8`.

**What the workers stop doing.** They were woken by main's kick (`woke=0x1003`/`0x1004`/`0x1002`) and went
straight back to their own `TimedWaitEvent` with no intervening signal: they ran their bodies and produced
no completion anywhere in the system.

**Counts over the 3.9 s ring.** `0x1001` 1256 signal / 364 wait; `0x1002` 160 signal / 467 wait / 70
timeout; `0x1003` 228 / 493 / 69; `0x1004` 210 / 493 / 70; `0x1005` 108 wait / 108 timeout. Per
completion key: `0x16bc68b18` 240 waits, all by `0x1001`, matched by 240 signals from the workers (`0x1002`
x70, `0x1003` x89, `0x1004` x81), plus 173 signals by `0x1001` with `woke=0`; `0x16bc68b28` 83 waits by
`0x1001` / 83 worker signals; `0x16bc68d58` 41 waits / 41 worker signals. **Only `0x1001` ever waits on a
completion key; the workers only signal it.** The 173 `woke=0` signals by `0x1001` are the credit side, and a
worker must be taking them through `nn::os::TryAcquireSemaphore` (`sdk+0x33b940`), which checks and decrements the
count under the cs without ever calling the condition variable — so it leaves no entry in the ring and the
consumer is invisible in this record.

**The object is a semaphore, not a stateless cv.** r17's main line reads `h=0x1001 ... wait=2
addr=0x16bc68b14 key=0x16bc68b18 tag=0x1001 word=0x1 last_svc=0x1c nsvc=124397`, `last svc
x3=0xffffffffffffffff`. The object base `0x16bc68af8` reads `[+0x10]=1`, `[+0x12]=0`, `[+0x14]=0`,
`[+0x18]=0x7fffffff`, cs `0`, cv word `1` — an `nn::os::Semaphore` with `count=0` and `max=0x7fffffff`.
Its stack is `sdk+0xd884dc <- sdk+0x33b918 <- sdk+0x55c0fc <- main+0x743c10` =
`nn::os::AcquireSemaphore` (`sdk+0x33b8e0`) through `__nnmusl_AcquireSemaphore`. S2.12's stopped run r4
decodes the same way (`0x16b3f4888`: `[+0x10]=1`, `[+0x14]=0`, `[+0x18]=0x7fffffff`).

**Therefore not a lost wakeup.** `nn::os::AcquireSemaphore` (`sdk+0x33b8e0`) is `cs.Enter(); while (count
== 0) cv.Wait(cs); count--; cs.Leave()`; `nn::os::ReleaseSemaphore` (`sdk+0x33ba80`) is `cs.Enter();
count++; cv.Signal(); cs.Leave()`. The count is the state and both sides touch it under the same critical
section, so a release can neither precede an acquire and vanish nor arrive before a waiter registers.
**The missing event is a release that was never issued.**

**The SDK's condition protocol.** `nn::os::detail::InternalConditionVariableImplByHorizon::Signal()`
(`sdk+0x343f20`) is `orr w1, wzr, #0x1; b <svc #0x1D>` — unconditional, with no user-mode condition-key
read. `Wait`/`TimedWait` (`sdk+0x343f40`/`sdk+0x343fe0`) issue `svc #0x1C` with `(x0=cs, x1=cv, x2=tag,
x3=-1|timeout)`. `InternalCriticalSectionImplByHorizon::Enter`/`Leave` (`sdk+0x344520`/`sdk+0x344610`)
are user-mode `ldaxr/stlxr` on the cs word and reach `svc #0x1A`/`svc #0x1B` only when the `0x40000000`
waiters bit is set. So the `0x1C` case comment, which justifies the `st32(key,1)`-before-`lock_handoff`
order by "the SDK reads the condition key in user mode and does not issue svc 0x1D if it is 0", describes
a mechanism this title's SDK does not have; the order is still required, for the different reason that the
waiter links into `g_wait` under `G` and `svc 0x1D` takes `G`, so no signaller can land in between.

**The worker loop is a poll.** `main+0x756e64` is the return address after `nn::os::TimedWaitEvent`, and
the result in `w0` is overwritten there, so the loop re-runs its body every ~33 ms regardless of signal
(`x3 ≈ 0x1f78a0b` ns, from `[x23+0x1528] x 0x0f4240`). A missed kick on a worker's event therefore
self-heals inside one interval and cannot be the mechanism.

**Not the S2.9 shape.** All three cores read `owner 0`; no `core waiter` line appears in any dump
(`g_core_wait` empty); every waiting thread reads `wkind=2` with the cs word `0` and the cv word `1`;
there are no lock waiters. The S2.9 equal-priority rotation is untouched (Do-Not #9).

**`0x1019` is inert here.** It has no entry anywhere in the 3.9 s ring and reads `wait=4` on 1 handle
`0x1016` with `nsvc=29`.

**No fix is made.** The cause is named as a shape (a frame-completion credit taken and never returned) but
not at a defect site, so no behaviour change was made; the instrumenting commit is the slice's only code
change. Do-Not #12 is added from this slice: a stall's trace window must be sized from the stop's own
lateness, not from the dump.

### Gate, boundary, scratch

- Gate GREEN: `TMPDIR=/Volumes/X9BS/externalTmp/MacNativeGames python -m unittest discover -s tests`
  passed 255 tests in 28.428 s, 1 skipped, exit 0, the known non-fatal subprocess `ResourceWarning`
  (`.babysteps/scratch/S2.13-missing-wakeup/gate.log`).
- Boundary (scope: this slice's diff against HEAD — one file, `games/nfs/runtime/threads.c`, 5 insertions /
  2 deletions, a `#define` widening and one constant use): linkage clear — no added call, import or
  resolve exists in the diff to follow into `games/totk` or `games/by3`; paths clear — the diff adds no
  path literal and names neither tree.
- Scratch:
  `.babysteps/scratch/S2.13-missing-wakeup/{run.sh,probe.sh,probe-more.sh,probe-summary.txt,r1..r20.input,r1..r20.emit,r1..r20.log,r1..r20.stdout,r1..r20-end.png (all except r17, which stopped),gate.log,main-756e64.dis,sdk-icv.dis,sdk-ics.dis,sdk-sem.dis,sdk-twe.dis,sdk-event.dis,sdk-plt.dis,sdk-cvhelpers.dis}`
  is evidence; the 7.2 GB work tree `/Volumes/X9BS/externalTmp/MacNativeGames/nfs-work` is re-derivable
  and not evidence.

The missing instrument is the completion semaphores' credit accounting. S2.14 is promoted.
