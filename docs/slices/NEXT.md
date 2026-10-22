---
slice: S2.2b
stateFile: docs/STATE.md
donotCount: 5
seededFrom: dc67530
---

# Next slice — S2.2b: resolve the `hid` IPC service

## The next action — verbatim

S2.2b — Implement the `hid` IPC service (controller input). S2.2 classified 0x60 (`NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID`) and 0x61 (`NVN_DEVICE_INFO_EVENTS_SUPPORT_REDUCTION_OPERATIONS`) as debug/feature flags returning 0. The init chain now proceeds past NVN device init, the first GPU frame runs (27 functions, 32 calls including `QueuePresentTexture`), then the game stops at `FAIL: IPC service not implemented` with the missing service being `hid` (input/hid). The runtime already has GameController framework wiring in the HID layer; implement `hid` service so controller input works end-to-end instead of leaving the boot blocked on an unimplemented IPC. The indirect first-present target and the runtime first-present trace carry into S2.2b; the `sub_63fea0` entry path (S3.2) also carries.

## What is settled — do not re-derive

- Both window-path blockers resolved (S2.1): headless offscreen `CAMetalLayer` in `vi_initialize()`; windowed three-tier CVDisplayLink fallback chain (`CVDisplayLinkCreateWithActiveCGDisplays` → `CVDisplayLinkCreateWithCGDisplays(displayID)` → host timer, every CVReturn logged). `dlprobe.out` (macOS 27.0.0, two active displays) shows every create variant returning 0 in this session, so S1.3's -6661 was environmental (consistent with the 03:40 unattended run's displays being asleep), not API breakage; which of S1.3's three bundled calls failed stays unattributed (Do-Not #5). The timer fallback is unexercised.
- NVN surface traced through 523 name resolutions (S2.1, S2.2): the endpoint `nvnBootstrapLoader("nvnDeviceGetProcAddress")` is reached and the first-seen tracer logs each name once. The 7 never requested are debug-checkpoint entries only; zero unknown names queried. After resolution the game calls `nvnDeviceInitialize`, `nvnQueueInitialize`, `nvnDeviceSetDepthMode`, `nvnDeviceSetWindowOriginMode`. Full map (chain entry → how reached → runtime status → HLE status): `docs/nfs-port-feasibility.md` S2.1.
- Device properties 0x60 and 0x61 classified (S2.2): `g_devinfo` extended with `[0x60] = 0` (NVN_DEVICE_INFO_DEBUG_GROUPS_MAX_DOMAIN_ID, debug-only) and `[0x61] = 0` (NVN_DEVICE_INFO_EVENTS_SUPPORT_REDUCTION_OPERATIONS, feature flag); both return 0 so the game falls back to non-debug/non-reduced behavior. Bounds check uses `NVN_DEVINFO_COUNT` const expression.
- First GPU frame observed (S2.2): `DeviceGetInteger=98`, full NVN init chain runs — window queue initialized, texture/pool/buffer created, command buffer recorded (clear color, scissor, render targets, barriers), `QueuePresentTexture` called, 27 functions / 32 calls in frame 1.
- New stop point (S2.2): application environment detected as local macOS process, persistent local account initialized, native keyboard controller initialized, then `FAIL: IPC service not implemented` for `"hid"`.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): Do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not invent an IPC service implementation — classify from observed calls.
- Do not claim gameplay beyond the service stub.

## Code to touch

| Work | Touch |
| --- | --- |
| `hid` service (controller input) | `games/nfs/runtime/` — the IPC service dispatch layer; implement `hid` to expose connected GameController gamepads (already registered with macOS) so the boot continues past `hid` initialization into gameplay-relevant initialization |
| Boot trace continuation | `games/nfs/runtime/` — any service initialization paths that call `hid` |

## Gate commands

- `python -m unittest discover -s tests` passes; every device init claim tied to a recorded observation; no gameplay claim.

## Controller input

## Withdrawn numbers

**2, 3, 4, 5** — all four entries from the previous NEXT.md "Withdrawn" section:
- #2: "The six shared-font inputs come from the game package" (proven wrong — zero fonts in RomFS)
- #3: "The CNMT version integer is unknown" (proven wrong — it is `0`)
- #4: A whole-package SHA-256 of `game.nsp` (Do-Not #2 applies)
- #5: "No `NCA0` at `0x200` means the artifact is wrong" (Do-Not #3 applies)

## Specs to read

- `docs/nfs-port-feasibility.md` — "Boot observation", "Not observed", "The rtld W^X finding", "Provider evidence"
- `CONTRIBUTING.md` — "How the port works, and what it must not become", "Repository layout"
