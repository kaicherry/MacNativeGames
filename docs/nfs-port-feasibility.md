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
