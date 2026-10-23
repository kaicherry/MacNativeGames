---
slice: S2.4b
stateFile: docs/STATE.md
donotCount: 5
seededFrom: f3afd9b
---

# Next slice — S2.4b: continue toward an observed game frame

## The next action — verbatim

S2.4b — Continue toward an observed game frame. S2.4 resolved the `friend:u` service open (notification queue answered: `CreateFriendService` → `CreateNotificationService` → `GetEvent`). The boot now stops at a null dereference in `nn::friends::GetFriendInvitationNotificationEvent` (`sdk+0x19d210`), which dereferences the NULL returned by `nn::am::GetApplicationFunctions()` — the applet-manager application-functions interface is neither initialized nor overridden (`applet_host.c` overrides `GetAppletResourceUserId` only). Resolve that blocker before any visual-correctness claim; record only behavior demonstrated by the resulting run. No gameplay claim.

## What is settled — do not re-derive

- `friend:u` open resolved (S2.4): the boot opens the service and answers `CreateFriendService` (0) → `IFriendService`, `CreateNotificationService` (1) → `INotificationService`, `INotificationService` `GetEvent` (0) → a never-signalled event; `Clear`/`Pop` are offline no-ops. `IFriendService` and `CreateDaemonSuspendSessionService` (2) are report-on-call stubs.
- The friend-invitation notification path does **not** go through `friend:u`: `nn::friends::GetFriendInvitationNotificationEvent` calls `nn::am::GetApplicationFunctions()` (`_ZN2nn2am23GetApplicationFunctionsEv`), which returns NULL (the applet-manager application-functions interface was never initialized). The faulting instruction is `ldr x8, [x0]` at `sdk+0x19d210`.
- Earlier (S2.1–S2.3): both window-path blockers cleared; 516/523 NVN names resolved; device properties 0x00–0x61 answered; first GPU frame runs (27 functions / 32 calls) but GPU submission is disabled (`TOTK_GPU_ALLOW_SUBMIT=1` re-enables), so no picture is rendered; hid command 101 handled; local account `ListOpenUsers` override in place.

## Do-Not list that applies

- Do-Not #2, #3, #4, #5 (from `docs/donot.md`): do not hash the whole NSP; do not conclude absent from raw bytes; do not blanket-rename fork comments; do not let one `runtime_fail` cover several calls.
- Do not claim visual correctness without an observed game frame; do not claim gameplay.
- Do not implement unobserved `nn::am` / applet-manager commands — classify from observed calls.

## Code to touch

| Work | Touch |
| --- | --- |
| Applet-manager blocker | `games/nfs/runtime/applet_host.c` (or a new override) — provide or stub `nn::am::GetApplicationFunctions`; alternatively stub `nn::friends::GetFriendInvitationNotificationEvent` as an offline never-signalled event. Implement only what is observed. |
| Visual verification | NVN/Metal path — only after a game-rendered picture is observed; re-enable GPU submission only to observe a frame. |

## Gate commands

- `python -m unittest discover -s tests` passes; every device-init or visual-correctness claim tied to a recorded observation; no gameplay claim.

## Withdrawn numbers

**2, 3, 4, 5** — unchanged from the previous seed.

## Specs to read

- `docs/nfs-port-feasibility.md` — "S2.4 — Resolve the `friend:u` service open".
- `CONTRIBUTING.md` — "How the port works, and what it must not become", "Repository layout".
