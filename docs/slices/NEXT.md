---
slice: S2.3
stateFile: docs/STATE.md
donotCount: 5
seededFrom: 9be4912
---

# Next slice — S2.3: boot through hid and trace to next stop

## The next action — verbatim

S2.3 — Boot the NFS runtime with the `hid` service implemented and trace the boot to the next stop point. The hid service is registered and answers commands 0–12 (Initialize, user handles, style set, vibration/six-axis handles, battery, npad counts, system properties). Run `make boot` headless and capture the full boot log. Trace what happens after the hid service is opened: controller initialization, account flow, and the next IPC or runtime blocker. Report the next stop with the same rigor as S2.2 — every IPC command observed, every fail path recorded, no gameplay claim.

## What is settled — do not re-derive

- Both window-path blockers resolved (S2.1): headless offscreen `CAMetalLayer` in `vi_initialize()`; windowed three-tier CVDisplayLink fallback chain. `dlprobe.out` (macOS 27.0.0, two active displays) shows every create variant returning 0.
- NVN surface traced through 523 name resolutions (S2.1, S2.2): `nvnBootstrapLoader("nvnDeviceGetProcAddress")` reached. 7 never requested are debug-checkpoint entries only; zero unknown names queried. Device properties 0x00–0x61 answered.
- First GPU frame observed (S2.2): 27 functions / 32 calls including `QueuePresentTexture`.
- Device properties 0x60 and 0x61 classified (S2.2): debug-only and feature flag, both return 0.
- hid IPC service implemented (S2.2b): registered in `system_services.c` via `runtime_ipc_register("hid:", hid_handler, NULL, NULL)`. Handles 13 commands following switchbrew "HID services" layout. Unknown commands still stop the runtime.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): Do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not claim gameplay — trace to the next stop only.
- Do not invent hid service behavior — classify from observed calls.

## Code to touch

| Work | Touch |
| --- | --- |
| Boot trace | `games/nfs/runtime/` — capture and analyze the boot log; any new stop may require additional IPC services or HLE functions |
| Next stop handler | `games/nfs/runtime/` — only if the boot reaches a new, specific IPC command that needs implementation |

## Gate commands

- `python -m unittest discover -s tests` passes; every device init claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — all four entries from the previous NEXT.md "Withdrawn" section (same as S2.2b's seed).

## Specs to read

- `docs/nfs-port-feasibility.md` — "Boot observation", "Not observed", "Provider evidence"
- `CONTRIBUTING.md` — "How the port works, and what it must not become", "Repository layout"
