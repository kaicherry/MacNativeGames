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
