---
slice: S2.4c
stateFile: docs/STATE.md
donotCount: 5
seededFrom: c493904
---

# Next slice — S2.4c: continue toward an observed game frame

## The next action — verbatim

S2.4c — Continue toward an observed game frame. S2.4b resolved the friend-invitation notification event null dereference. The boot now passes `friend:u` and the friend-invitation path, reaches the game's own environment init (`EnvironmentService`, `AppLifecycle`, `DirectorService` — the game's own logging), and begins shader translation. It stops at the shader translation service: neither the native translator (`build/shader-service/totk_xlate`) nor the Python script (`scripts/shader_service.py`) is carried in this tree (`totk_shader_rt.m` `service_start`), so the game's first shader translation request fails and the boot exits. Provide or route the shader translator so the game can translate a shader toward a frame; record only behavior demonstrated by the resulting run. No gameplay claim.

## What is settled — do not re-derive

- The friend-invitation notification event null dereference is resolved (S2.4b): `games/nfs/runtime/applet_host.c` overrides `nn::friends::GetFriendInvitationNotificationEvent` (`_ZN2nn7friends36GetFriendInvitationNotificationEventEv`) to bind the SDK's process-global `SystemEventType` (`sdk+0xcabfb0`) to a never-signalled host event (`hle_bind_system_event`) instead of dereferencing the NULL `nn::am::GetApplicationFunctions()`. Offline there are no friend invitations.
- The resulting boot (`.babysteps/scratch/S2.4b-NFS-friends-event/boot.log`) passes `friend:u` + the friend-invitation path, reaches the game's own environment init, and begins shader translation; it exits (code 2) when the game's first shader request (`c10b455d…`) finds no translator.
- The shader translator is not carried: `totk_shader_rt.m` `service_start` spawns the native `build/shader-service/totk_xlate` if present, else `python3 scripts/shader_service.py` + .NET `build/totk_xlate/totk_xlate.dll`. Neither path exists; the shader tooling arrives with the app-assembly slice.
- Earlier (S2.1–S2.4): both window-path blockers cleared; 516/523 NVN names resolved; device properties 0x00–0x61 answered; first GPU frame runs (27 functions / 32 calls) but GPU submission is disabled (`TOTK_GPU_ALLOW_SUBMIT=1` re-enables); hid command 101 handled; local account `ListOpenUsers` override in place; `friend:u` notification queue answered.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not claim visual correctness without an observed game frame; do not claim gameplay.
- Do not implement unobserved shader/service commands — classify from observed calls.

## Code to touch

| Work | Touch |
| --- | --- |
| Shader-translation stop | `games/nfs/third_party/d2r_gfx/gfx/nvn/totk_shader_rt.m` (`service_start`, ~line 100) — the translator spawn. Provide one of the two translator paths (`make shader-service` → `build/shader-service/totk_xlate`, or carry `scripts/shader_service.py` + the .NET translator), or route translation in-process. Implement only what is observed. |
| Visual verification | NVN/Metal path — only after a game-rendered picture is observed; re-enable GPU submission only to observe a frame. |

## Gate commands

- `python -m unittest discover -s tests` passes; every device-init or visual-correctness claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from the previous seed.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.4 — Resolve the `friend:u` service open" (the friend-invitation stop is recorded there).
- `CONTRIBUTING.md` — "How the port works, and what it must not become", "Repository layout".
