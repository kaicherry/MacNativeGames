---
slice: S2.1
stateFile: docs/STATE.md
donotCount: 4
seededFrom: 8e05a5e
---

# Next slice — S2.1: map the required NVN surface

## The next action — verbatim

S2.1 — Map the required NVN surface. First business is the two window-path blockers the S1.3 boot left, each to be cleared with evidence or split: headless the boot stops at `FAIL: VI requires native macOS window` (the VI service requires the native window), windowed it stops in the frontend at `FAIL: native display link initialization` before the guest starts (`CVDisplayLinkCreateWithActiveCGDisplays` returns -6661 in a standalone probe in the same session; deprecated since macOS 15.0; environmental vs API breakage is not established). Then map the NVN surface the S0.3 `nvnBootstrapLoader` chain reaches and observe that chain at runtime: its endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")` was not reached in S1.3 (both runs stopped first), and the HLE takeover (`hle_nvn.c` `HLE_FORCE`) is instrumented to log its argument when it is called. Build on the settled S1.3 base — the six-entry load set, 141,377 relocations with zero unresolved strong imports, DT_INITs and `ENTER nnMain` observed — and carry the remaining splits: the indirect first-present target and the runtime trace (S2.1), the `sub_63fea0` entry path (S3.2).

Gate: `python -m unittest discover -s tests` passes; every NVN surface mapping is tied to a recorded observation (the boot log or the S0.3 chain evidence) and no claim extends past it.

Close when the required NVN surface is mapped with each mapping tied to evidence, the two window-path blockers are resolved or split with evidence, the S0.3 chain's runtime reach is recorded as far as it goes and where it stops, the configured baseline passes, and no claim is made beyond what was observed. Make no gameplay claim.

## What is settled — do not re-derive

- `games/nfs/` exists and is wired through `build_app.py` as the third title (`VERSIONS['nfs100']`, `TITLES`, `APPS`, `step_code_nfs`/`step_data_nfs`); the build rebuilds NFS from `game.nsp` with every `expected-v1.0.0.json` check passing (~5 min). Full record: `docs/nfs-port-feasibility.md` "S1.3 — The games/nfs tree and the boot through nnMain".
- The load set is the whole ExeFS: five digest-bound rewrite images (`rtld`, `main`, `sdk`, `subsdk0`, `subsdk1`) plus `main.npdm` as process metadata. 141,377 relocations; zero unresolved strong imports. rtld's text is never executed (W^X shared host page) and its DT_INIT is skipped — by policy, see `.operator/specs/nfs-port.md`.
- The boot reaches `nnMain` at `main+0x2d2fd0` (S0.3's static address) and runs SDK startup, all runnable DT_INITs, guest threads, and `nv::InitializeGraphics` served by the HLE (Metal device observed).
- The two window-path blockers and their evidence: headless `FAIL: VI requires native macOS window`; windowed `FAIL: native display link initialization` (standalone `CVDisplayLinkCreateWithActiveCGDisplays` probe returns -6661 in this session).
- Providers are settled at symbol level: no `.nss` in ExeFS (six entries) or the decrypted RomFS (19,169 paths); `glslc_*`/`multimedia_*` → `sdk`, `movie::SetAllocator` → `subsdk1`; `nnosGetDefaultUserExceptionStackForApplication` is weak (binding 2), binds zero, not boot-blocking. The DT_NEEDED **filename** identity is not claimed and must not be asserted.
- The five NSO build IDs match none of the registered D2R builds, so all rewrites are generic (`cuts 0`, `obs 0`); rewriter runs with `DM_RULESET=native DM_PRIVATE_GATES=0`.
- Configured baseline last passed: 255 tests, 1 skipped, non-fatal subprocess `ResourceWarning`, exit 0.
- No gameplay claim exists. No present, frame, audio, or gameplay has been observed.

## Withdrawn — proven wrong, do not use

- "The six shared-font inputs come from the game package." Zero font files among the 19,169 RomFS paths; they are firmware-dump inputs (Do-Not #3 applies to absence claims).
- "The CNMT version integer is unknown." It is `0`; `display_version` `1.0.0` is a NACP field (`+0x3060`), not a CNMT field.
- A whole-package SHA-256 of `game.nsp` (Do-Not #2). Identity stays per-file.
- "No `NCA0` at `0x200` means the artifact is wrong." Pre-decryption bytes (Do-Not #3).

## Code to touch

| Work | Touch |
| --- | --- |
| Window path | `games/nfs/runtime/frontend.m` (display-link init), the VI window requirement (`runtime/services.c` / applet host) — resolve each with evidence or split it |
| NVN surface | `games/nfs/third_party/d2r_gfx/` (`nvn_*`), driven by the S0.3 chain and the device inventory (523 names / 424 implemented / 99 red stubs, logged at boot) |
| Chain observation | `hle_nvn.c` `HLE_FORCE("nvnBootstrapLoader")` already logs its argument; the S0.3 chain endpoint is what to catch |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S1.3 — The games/nfs tree and the boot through nnMain": sections "Boot observation", "Not observed", "The rtld W^X finding".
- `docs/nfs-port-feasibility.md` — "S0.3 — Function-level startup and provider evidence", section "Startup chain (static, function-level)" — quote the numbers: `nnMain 0x2d2fd0`; call sites `0x2d30b8`, `0x2a8164`, `0x3d4500`, `0x3d4730` (`b`), `0x7684f0`, `0x768c14`; PLT stub `0x76e6f0` (`bl 0x7f3c00`); `sub_76e690` `blr x8` at `0x76e714` through `[0xd37000+0x3d0]`; `sub_76a220` name resolutions `0x76a284` `nvnDeviceInitialize`, `0x76a8c0` `nvnQueuePresentTexture`, `0x76aa9c` `nvnWindowAcquireTexture`, `0x76aaf0` `nvnWindowSetPresentInterval`.
- `docs/nfs-port-feasibility.md` — "S1.3" section "Provider evidence" and its claim boundary.
- `CONTRIBUTING.md` — "How the port works, and what it must not become" and "Repository layout".

## Carried evidence splits — owners

- S0.3 `nvnBootstrapLoader` chain endpoint, runtime trace, indirect first-present target → S2.1 (this slice).
- `sub_63fea0` indirect/thread entry → S3.2.

## Do-Not entries that apply here

- #2 — size every long evidence run explicitly (the S1.3 display-link probe pattern).
- #3 — do not conclude absence from bytes that are not the decoded layer.
- #4 — do not blanket-rename a forked tree's comments; keep the predecessor as attribution until re-observed.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Every NVN surface mapping tied to a recorded observation; none extends past it.

## When this slice closes

Close when the required NVN surface is mapped with each mapping tied to evidence, the two window-path blockers are resolved or split with evidence, the S0.3 chain's runtime reach is recorded as far as it goes and where it stops, the configured baseline passes, and no claim is made beyond what was observed. Make no gameplay claim.
