---
slice: S2.4
stateFile: docs/STATE.md
donotCount: 5
seededFrom: bf76f17
---

# Next slice — S2.4: verify shader and visual correctness

## The next action — verbatim

S2.4 — Verify shader and visual correctness. The latest headless boot reaches `nn::friends::NotificationQueue::Initialize` and stops when the SDK requests the unimplemented `friend:u` service, before a game picture is observed. Resolve or split that service-open blocker before making a visual-correctness claim; record only behavior demonstrated by the resulting run. No gameplay claim.

## What is settled — do not re-derive

- Both window-path blockers resolved (S2.1): headless offscreen `CAMetalLayer` in `vi_initialize()`; windowed three-tier CVDisplayLink fallback chain. `dlprobe.out` (macOS 27.0.0, two active displays) shows every create variant returning 0.
- NVN surface traced through 523 name resolutions (S2.1, S2.2): `nvnBootstrapLoader("nvnDeviceGetProcAddress")` reached. 7 never requested are debug-checkpoint entries only; zero unknown names queried. Device properties 0x00–0x61 answered.
- First GPU frame observed (S2.2): 27 functions / 32 calls including `QueuePresentTexture`.
- Device properties 0x60 and 0x61 classified (S2.2): debug-only and feature flag, both return 0.
- HID/account boot trace completed (S2.3): service opens as `hid`; command 101 was observed and handled (commands 0–12 were not observed in this run). The local `ListOpenUsers` override passes the previous null dereference; the next stop is the unimplemented `friend:u` service open, with no friend command observed.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): Do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not claim gameplay — trace to the next stop only.
- Do not invent hid service behavior — classify from observed calls.

## Code to touch

| Work | Touch |
| --- | --- |
| Service blocker | `games/nfs/runtime/` — only if the friend-service request is in scope; do not implement commands that have not been observed |
| Visual verification | NVN/Metal path and captured output — only after a game-rendered picture is observed |

## Gate commands

- `python -m unittest discover -s tests` passes; shader/visual claims tied to an observed game-rendered image; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — all four entries from the previous NEXT.md "Withdrawn" section (same as S2.2b's seed).

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.3 — Boot through HID and stop at `friend:u`", "Not observed", "Provider evidence"
- `CONTRIBUTING.md` — "How the port works, and what it must not become", "Repository layout"
