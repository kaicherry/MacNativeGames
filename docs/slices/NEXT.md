---
slice: S2.2
stateFile: docs/STATE.md
donotCount: 5
seededFrom: 8a9a8b2
---

# Next slice — S2.2: resolve NVN device property 0x60

## The next action — verbatim

S2.2 — Resolve NVN device property 0x60 and continue the device init chain. Both S2.1 boots cleared the window-path blockers, reached the S0.3 endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")`, resolved 516 of 523 NVN names, and both stopped at `unsupported NVN device property 0x60` (the last line of both kept logs) — `nvnDeviceGetInteger` had answered 96 properties `0x00`–`0x5f` first. The header already names the constant (`NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID`, `nvn.h:328`); what is open is the **value** to answer (the HLE table carries Tegra X1 measured values only to `0x5F`, and `0x61` `NVN_DEVICE_INFO_EVENTS_SUPPORT_REDUCTION_OPERATIONS` is next in the enum) and what the game does once its queries are answered. Answer `0x60`+ from a sourced value or classify the query with evidence — never invent a number — then continue tracing to the next stop point. The indirect first-present target and the runtime first-present trace carry into S2.2; the `sub_63fea0` entry path → S3.2.

Gate: `python -m unittest discover -s tests` passes; every device init claim is tied to a recorded observation; no gameplay claim.

Close when NVN device property `0x60` is answered from a sourced value or its query is classified with evidence (what the game does with the answer), the device init chain is traced past it to the next stop point, the S2.1 findings (516 resolved names, both blockers resolved, stop at `0x60`) are incorporated, the configured baseline passes, and no claim extends past what was observed. Make no gameplay claim.

## What is settled — do not re-derive

- `games/nfs/` exists and is wired through `build_app.py` as the third title (`VERSIONS['nfs100']`, `TITLES`, `APPS`, `step_code_nfs`/`step_data_nfs`); the build rebuilds NFS from `game.nsp` with every `expected-v1.0.0.json` check passing (~5 min). Full record: `docs/nfs-port-feasibility.md` "S1.3 — The games/nfs tree and the boot through nnMain".
- The load set is the whole ExeFS: five digest-bound rewrite images (`rtld`, `main`, `sdk`, `subsdk0`, `subsdk1`) plus `main.npdm` as process metadata. 141,377 relocations; zero unresolved strong imports. rtld's text is never executed (W^X shared host page) and its DT_INIT is skipped — by policy, see `.operator/specs/nfs-port.md`.
- The boot reaches `nnMain` at `main+0x2d2fd0` (S0.3's static address) and runs SDK startup, all runnable DT_INITs, guest threads, and `nv::InitializeGraphics` served by the HLE (Metal device observed).
- Both window-path blockers resolved (S2.1): headless offscreen `CAMetalLayer` in `vi_initialize()`; windowed vsync source tries `CVDisplayLinkCreateWithActiveCGDisplays` → `CVDisplayLinkCreateWithCGDisplays(displayID)` → host timer, every CVReturn logged. `dlprobe.out` (macOS 27.0.0, two active displays) shows every create variant returning 0 in this session, so S1.3's -6661 was environmental (consistent with the 03:40 unattended run's displays being asleep), not API breakage; which of S1.3's three bundled calls failed stays unattributed (Do-Not #5). The timer fallback is unexercised.
- NVN surface traced through 516 of 523 name resolutions (S2.1): the endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")` is reached and the first-seen tracer logs each name once. The 7 never requested are debug-checkpoint entries only; zero unknown names queried. After resolution the game calls `nvnDeviceInitialize`, `nvnQueueInitialize`, `nvnDeviceSetDepthMode`, `nvnDeviceSetWindowOriginMode`. Full map (chain entry → how reached → runtime status → HLE status): `docs/nfs-port-feasibility.md` S2.1.
- Both S2.1 boots stop at the same point: `nvnDeviceGetInteger` answers `0x00`–`0x5f` (96 properties, no gaps) then dies at `unsupported NVN device property 0x60` = `NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID` (`nvn.h:328`); the HLE constant table ends at `0x5F`. This is the next blocker. Keep logs: `.babysteps/scratch/S2.1-NFS-nvn-surface/`.
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
| Device property 0x60+ | `games/nfs/third_party/d2r_gfx/gfx/nvn/hle_nvn.c` — the `nvnDeviceGetInteger` property table (measured values end at `0x5F`); answer `0x60` (`NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID`) and `0x61` from a sourced value or classify them with evidence |
| Chain continuation | `hle_nvn.c` — the same first-seen tracer from S2.1; continue logging as the device init chain proceeds past `0x60` |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — "S1.3 — The games/nfs tree and the boot through nnMain": sections "Boot observation", "Not observed", "The rtld W^X finding".
- `docs/nfs-port-feasibility.md` — "S0.3 — Function-level startup and provider evidence", section "Startup chain (static, function-level)" — quote the numbers: `nnMain 0x2d2fd0`; call sites `0x2d30b8`, `0x2a8164`, `0x3d4500`, `0x3d4730` (`b`), `0x7684f0`, `0x768c14`; PLT stub `0x76e6f0` (`bl 0x7f3c00`); `sub_76e690` `blr x8` at `0x76e714` through `[0xd37000+0x3d0]`; `sub_76a220` name resolutions `0x76a284` `nvnDeviceInitialize`, `0x76a8c0` `nvnQueuePresentTexture`, `0x76aa9c` `nvnWindowAcquireTexture`, `0x76aaf0` `nvnWindowSetPresentInterval`.
- `docs/nfs-port-feasibility.md` — "S1.3" section "Provider evidence" and its claim boundary.
- `CONTRIBUTING.md` — "How the port works, and what it must not become" and "Repository layout".

## Carried evidence splits — owners

- S0.3 `nvnBootstrapLoader` chain endpoint and the runtime trace → resolved in S2.1 (endpoint reached, 516 resolutions, stop at `0x60`). The **indirect first-present target and the runtime first-present trace** carry into S2.2.
- `sub_63fea0` indirect/thread entry → S3.2.

## Do-Not entries that apply here

- #2 — size every long evidence run explicitly (the S2.1 display-link probe pattern).
- #3 — do not conclude absence from bytes that are not the decoded layer.
- #4 — do not blanket-rename a forked tree's comments; keep the predecessor as attribution until re-observed.
- #5 — one `runtime_fail` must not cover several calls: log each call's code at the site so a failure is attributable.
- Do not invent a device-property value: `0x60`/`0x61` must be answered from a sourced measurement or classified with evidence, never from what a number "should" be.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Every device init claim tied to a recorded observation; none extends past it.

## When this slice closes

Close when NVN device property `0x60` is answered from a sourced value or its query is classified with evidence, the device init chain is traced past it to the next stop point, the S2.1 findings (516 resolved names, both blockers resolved, stop at `0x60`) are incorporated, the configured baseline passes, and no claim extends past what was observed. Make no gameplay claim.
