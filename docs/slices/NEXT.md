---
slice: S0.3
stateFile: docs/STATE.md
donotCount: 1
seededFrom: f117adb2299e61ce1d99a21b31a2f3c121482bf5
---

# Next slice — S0.3: NFS startup call and provider evidence

## The next action — verbatim

S0.3 — Use an available function-level static-analysis path on the existing hash-verified NFS analysis carriers to trace startup from `nnMain` to the first `nvnBootstrapLoader`/NVN operation and presentation, and to the earliest `nifm`/socket/friends calls; identify whether the three `DT_NEEDED` `.nss` names and `nnosGetDefaultUserExceptionStackForApplication` are provided by package modules or remain external. Distinguish linked, statically reachable, and runtime-observed facts. If no safe supported analyzer can establish a path or provider, record that exact blocker and split it; do not install tools or edit port code.

## What is settled — do not re-derive

- S0.2 evidence and blockers are recorded in `docs/nfs-port-feasibility.md`; no implementation GO was given, and there is no bootability or gameplay claim.
- ExeFS includes `subsdk1`. The report lists `main`'s three `.nss` `DT_NEEDED` strings and one unresolved SDK import, `nnosGetDefaultUserExceptionStackForApplication`; their providers remain unresolved.
- NPDM was decoded. Raw SVC candidates are not proof of execution; import and bounded call-site references do not prove reachability. First-call ordering remains unresolved.
- Existing carriers are hash-verified. Preserve the ignored evidence at `.babysteps/scratch/S0.1-NFS-feasibility/` and `.babysteps/scratch/S0.2-NFS-feasibility/`; do not rerun title-bound extraction.
- The S0.2 hactool source pin is `1d64a83450e025622f3468c28fc4164dad2c5ef6`.
- The configured full baseline passed 255 tests, 1 skipped, with a non-fatal subprocess `ResourceWarning`. No new Do-Not fault was found.

## Withdrawn — proven wrong, do not use

- No new withdrawn numbers.

## Code to touch

| Work | Touch |
| --- | --- |
| S0.3 evidence report | `docs/nfs-port-feasibility.md` only. |
| Port source/runtime/build | None — no port code edits. |

## Read these first, and only these

- `docs/nfs-port-feasibility.md` — “S0.2 — Static linkage, permissions, and bounded call-site evidence”, “Closeout”.
- `docs/how-it-works.md` — “The code”, “The operating system”, “The graphics”.
- `.operator/guides/hopper-demo.md` — “Known limits”, “Manual read path”.
- `.opencode/commands/babysteps-exploration.md` — “Call discipline”.
- `.operator/index/games/totk.md` — “Architecture”.
- `.operator/index/games/by3.md` — “Architecture”.

## Do-Not entries that apply here

- None. Current Do-Not count is 1; #1 is unrelated.

## The gate

- `python -m unittest discover -s tests` (configured baseline).
- Evidence gate: function-level startup call order or explicit tool/provider blockers and splits; `.nss` and `nnosGetDefaultUserExceptionStackForApplication` provider status; linked, permitted, statically reachable, and runtime-observed claims kept distinct; no code edits.

## When this slice closes

Close when the report traces those startup calls with function-level evidence or records a precise tool/provider blocker and splits unresolved questions; `.nss` and `nnosGetDefaultUserExceptionStackForApplication` providers are identified or explicitly unresolved; linked, permitted, statically reachable, and runtime-observed claims are distinct; the configured baseline passes; and no port code is changed. Make no bootability or gameplay claim.
