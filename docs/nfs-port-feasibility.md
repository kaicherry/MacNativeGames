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
