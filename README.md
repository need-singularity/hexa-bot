# 🤖 hexa-bot — Robot substrate (HEXA family)

> 4-verb robot substrate organized around the **n=6 invariant lattice**:
> ROBOTICS / TRANSPORT / AUTOMATION / DOG_ROBOT. Spec-first extraction
> from `n6-architecture@c0f1f570` (2026-05-06). Working `.hexa` CLI
> dispatcher is a placeholder; all 4 verbs ship as spec corpus at v1.0.0.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-informational.svg)](CHANGELOG.md)
[![Verbs: 0/4 wired](https://img.shields.io/badge/verbs-0%2F4_wired_(spec--only)-orange.svg)](#verbs)
[![n=6 lattice](https://img.shields.io/badge/n%3D6-σ%3D12_τ%3D4_φ%3D2_J₂%3D24-purple.svg)](#why-n6-robot-substrate)
[![Substrate](https://img.shields.io/badge/substrate-peripheral_reality-blue.svg)](#cross-link)

---

## Why (n=6 robot substrate)

`hexa-bot` is the **robot-axis sister** in the n=6 substrate family. The
n6-architecture canon decomposes peripheral reality across six invariant
substrates; `hexa-bot` carves out the embodiment-and-actuation slice:

```
  n=6 substrate lattice
    ├── hexa-bio        molecular   (write-side cage assembly)
    ├── hexa-codex      AI/cognition (planned cousin)
    ├── anima           consciousness (planned cousin)
    └── hexa-bot   ←    robotics    (THIS REPO — embodiment / actuation)
                          + 2 more axes TBD
```

The 4-verb decomposition mirrors the hexa-bio pattern (4 sister verbs
per substrate, σ(6)=12 → 4 verb pairs):

```
                      ┌──────────────┐
                      │  manipulation│
                      │   (ROBOTICS) │
                      └───────┬──────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
   ┌──────────▼──────┐  ┌─────▼──────┐  ┌────▼────────┐
   │   transport     │  │ automation │  │  dog_robot  │
   │  (mobility)     │  │ (control)  │  │ (quadruped) │
   └─────────────────┘  └────────────┘  └─────────────┘
       [SPEC]              [SPEC]            [SPEC]
```

---

## Verbs

| Verb         | Status | Substrate role                           | Seed source                                            |
|--------------|--------|------------------------------------------|--------------------------------------------------------|
| `robotics`   | SPEC   | manipulation / arm / actuator primitives | `n6-architecture/domains/infra/robotics/`              |
| `transport`  | SPEC   | ground+aerial transport + logistics      | `n6-architecture/domains/infra/robotics-transport/`    |
| `automation` | SPEC   | control-automation + factory-line        | `n6-architecture/domains/infra/control-automation/`    |
| `dog_robot`  | SPEC   | quadruped test rig + locomotion          | `n6-architecture/domains/life/dog-robot-test/`         |

All four are extracted directly from `n6-architecture@c0f1f570` via `cp -R`;
the original n6-architecture trees remain canonical and read-only.

---

## Status

**4-verb robot substrate. spec-first (작동 .hexa CLI TBD).** All 4 verbs ship
as `.md` spec corpus at v1.0.0. The bundled `cli/hexa-bot.hexa` is a
**placeholder dispatcher** — it routes the 4 sub-commands and prints spec
heads, but does not execute any working numerical sandbox. Working
empirical paths are deferred to post-v1.0 cycles.

**motorcycle**은 의도적으로 빠져 있습니다 — 후속 `hexa-mobility` standalone
후보 (자율 이동체 axis: motorcycle + drone + autonomous-vehicle 묶음).

---

## Install

### Via git clone (works today)

```bash
git clone https://github.com/need-singularity/hexa-bot.git ~/.hexa-bot
export HEXA_BOT_ROOT=~/.hexa-bot
export PATH="$HEXA_BOT_ROOT/cli:$PATH"

# Run any subcommand (placeholder dispatcher):
hexa run $HEXA_BOT_ROOT/cli/hexa-bot.hexa selftest
hexa run $HEXA_BOT_ROOT/cli/hexa-bot.hexa status
hexa run $HEXA_BOT_ROOT/cli/hexa-bot.hexa robotics
```

### Via `hx` (when registry entry lands)

```bash
hx install hexa-bot          # global, pulls latest from registry
hx install hexa-bot@1.0.0    # pin specific version
hexa-bot --version           # → 1.0.0
```

### Read the spec corpus directly

```bash
$EDITOR ~/.hexa-bot/robotics/robotics.md
$EDITOR ~/.hexa-bot/transport/robotics-transport.md
$EDITOR ~/.hexa-bot/automation/control-automation.md
$EDITOR ~/.hexa-bot/dog_robot/dog-robot-test.md
```

---

## Cross-link

`hexa-bot` is a sibling substrate in the `need-singularity` n=6 family:

- [`need-singularity/hexa-codex`](https://github.com/need-singularity/hexa-codex)
  — AI substrate cousin (cognition / language)
- [`need-singularity/anima`](https://github.com/need-singularity/anima)
  — consciousness substrate cousin (subjective-experience axis)
- [`need-singularity/hexa-bio`](https://github.com/need-singularity/hexa-bio)
  — molecular substrate cousin (4-verb molecular toolkit; sister precedent)
- [`need-singularity/lumiere`](https://github.com/need-singularity/lumiere)
  — apps-axis spike (camera-filter + hexa-main-character; iOS standalone)

Upstream canon: [`need-singularity/n6-architecture`](https://github.com/need-singularity/n6-architecture)
@ commit `c0f1f570` (extraction reference SHA).

Planned post-v1.0 cousin: `need-singularity/hexa-mobility` — autonomous
mobility (motorcycle + drone + autonomous-vehicle 자율 이동체 묶음).

---

## License

MIT. See [LICENSE](LICENSE).

Copyright (c) 2026 박민우 / need-singularity.
