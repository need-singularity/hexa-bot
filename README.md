# 🤖 hexa-bot — Robot substrate (HEXA family)

> 4-verb robot substrate organized around the **n=6 invariant lattice**:
> ROBOTICS / TRANSPORT / AUTOMATION / DOG_ROBOT. Spec-first extraction
> from `canon@c0f1f570` (2026-05-06). Working `.hexa` CLI
> dispatcher is a placeholder; all 4 verbs ship as spec corpus at v1.0.0.

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.20102589.svg)](https://doi.org/10.5281/zenodo.20102589)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.1.0-informational.svg)](CHANGELOG.md)
[![Verbs: 1/4 wired](https://img.shields.io/badge/verbs-1%2F4_wired_(dog__robot_firmware)-yellow.svg)](#verbs)
[![n=6 lattice](https://img.shields.io/badge/n%3D6-σ%3D12_τ%3D4_φ%3D2_J₂%3D24-purple.svg)](#why-n6-robot-substrate)
[![Verify: 5/5](https://img.shields.io/badge/verify-5%2F5_PASS-brightgreen.svg)](verify/cli.py)
[![Sim: 4/4 gaits](https://img.shields.io/badge/sim-4%2F4_gaits_coherent-brightgreen.svg)](module/firmware/sim/)
[![Tests: 8/8 unit · 12/13 pytest](https://img.shields.io/badge/tests-8%2F8_unit_·_12%2F13_pytest-brightgreen.svg)](tests/)
[![Substrate](https://img.shields.io/badge/substrate-peripheral_reality-blue.svg)](#cross-link)

---

## Why (n=6 robot substrate)

`hexa-bot` is the **robot-axis sister** in the n=6 substrate family. The
canon canon decomposes peripheral reality across six invariant
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
| `robotics`   | SPEC   | manipulation / arm / actuator primitives | `canon/domains/infra/robotics/`              |
| `transport`  | SPEC   | ground+aerial transport + logistics      | `canon/domains/infra/robotics-transport/`    |
| `automation` | SPEC   | control-automation + factory-line        | `canon/domains/infra/control-automation/`    |
| `dog_robot`  | SPEC   | quadruped test rig + locomotion          | `canon/domains/life/dog-robot-test/`         |

All four are extracted directly from `canon@c0f1f570` via `cp -R`;
the original canon trees remain canonical and read-only.

---

## Status

**4-verb robot substrate. mk1.1 — 1/4 verbs (dog_robot) wired, 3/4 spec-only.**

v1.1.0 (2026-05-07) adds a runnable surface modeled on the `hexa-sscb`
sister-pattern: `verify/` (atlas-anchored audit), `build/` (pandoc PDF),
`tests/` (pytest A-1..A-10), `module/firmware/` (STM32F429 reference + host
sim + unit tests). The `cli/hexa-bot.hexa` dispatcher routes new
subcommands `verify` / `build` / `sim` / `firmware` / `n6` alongside the
4 verb sub-commands.

**motorcycle**은 의도적으로 빠져 있습니다 — 후속 `hexa-mobility` standalone
후보 (자율 이동체 axis: motorcycle + drone + autonomous-vehicle 묶음).

---

## Module inventory

| Module | What it is | Status |
|---|---|---|
| [`robotics/robotics.md`](robotics/robotics.md) | manipulation / arm / actuator primitives | SPEC |
| [`transport/robotics-transport.md`](transport/robotics-transport.md) | ground+aerial transport + logistics | SPEC |
| [`automation/control-automation.md`](automation/control-automation.md) | control-automation + factory-line | SPEC |
| [`dog_robot/dog-robot-test.md`](dog_robot/dog-robot-test.md) | quadruped τ(6)=4 gait test rig | SPEC |
| [`cli/hexa-bot.hexa`](cli/hexa-bot.hexa) | top-level dispatcher (.hexa) | wired |
| [`verify/`](verify/) | atlas-anchored audit (5 stdlib scripts + cli.py dispatcher) | wired |
| [`build/`](build/) | pandoc PDF rebuild (Makefile + xeCJK header) | wired |
| [`tests/`](tests/) | pytest A-1..A-10 acceptance + .hexa selftest | wired |
| [`module/firmware/`](module/firmware/) | STM32F429ZIT6 dog_robot reference (CMSIS-only) | wired |
| [`module/firmware/sim/`](module/firmware/sim/) | host POSIX simulator (no ARM toolchain needed) | wired |
| [`module/firmware/test/`](module/firmware/test/) | xunit-style unit tests (8 cases) | wired |
| `hexa.toml` / `install.hexa` / `LICENSE` / `.roadmap.hexa_bot` / `CHANGELOG.md` | metadata | wired |

The `core/<verb>/ + module/<verb>/ + cli/` triplet pattern follows hive
**raw.mk2 arch.001** — same layout the `hexa-sscb` reference repository
uses, adapted to the 4-verb robot decomposition.

---

## Build & verify

```bash
# unified verifier (n=6 + verbs + cross-doc + roadmap + firmware-sync)
python3 verify/cli.py                    # all checks, human readable
python3 verify/cli.py --json             # machine-readable
python3 verify/cli.py n6                 # one specific check

# pandoc PDFs (skip gracefully if pandoc/xelatex missing)
make -C build all                        # 4 verb PDFs in build/out/
make -C build check                      # toolchain probe

# firmware (ARM cross-compile)
make -C module/firmware check            # toolchain probe
make -C module/firmware all              # firmware.elf, .bin, .hex

# host simulator (no ARM needed — runs everywhere)
make -C module/firmware/sim selftest     # 4/4 gait pattern coherence
make -C module/firmware/sim all-gaits    # JSON summary per gait
make -C module/firmware/sim run          # default trot 1.6 s

# firmware unit tests
make -C module/firmware/test run         # 8/8 PASS

# pytest acceptance ladder (A-1 .. A-10)
pytest tests/ -v -m auto                 # 12 PASS, 1 SKIP (pre-tag)

# .hexa CLI dispatcher (existing v1.0 subcmds preserved)
hexa run cli/hexa-bot.hexa selftest      # 4-verb spec sentinel
hexa run cli/hexa-bot.hexa robotics      # spec head
```

### Last validation sweep — 2026-05-07

| Check | Result | Notes |
|---|---|---|
| `python3 verify/cli.py all` | ✅ 5/5 PASS | n6 / verbs / audit / roadmap / firmware-sync all PASS |
| `make -C build all` | ✅ 4 PDFs built | total ≈386 KB into build/out/ (gitignored) |
| `make -C module/firmware all` | ✅ firmware.elf | 824 B text + 2608 B BSS — `arm-none-eabi-gcc 16.1.0` |
| `make -C module/firmware/sim selftest` | ✅ 4/4 gaits coherent | STAND/TROT/BOUND/GALLOP all assert-clean |
| `make -C module/firmware/test run` | ✅ 8/8 unit PASS | τ(6)=4 + σ(6)=12 lattice + state machine + servo bounds |
| `pytest tests/ -v -m auto` | ✅ 12 PASS / 1 SKIP | A-9 (git tag) skips with reason |
| `hexa run cli/hexa-bot.hexa selftest` | ✅ 4/4 spec sentinels | existing v1.0 path preserved |

---

## Install

```bash
# 1. Install hexa-lang (gives you `hexa` + `hx` package manager)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/dancinlab/hexa-lang/main/install.sh)"

# 2. Install hexa-bot
hx install hexa-bot
```

## Run

```bash
hexa-bot robotics      # manipulation / arm / actuator               [SPEC]
hexa-bot transport     # ground+aerial transport + logistics         [SPEC]
hexa-bot automation    # control-automation + factory-line           [SPEC]
hexa-bot dog_robot     # quadruped test rig + locomotion             [SPEC]
hexa-bot status        # 0/4-wired verb table + verdict + caveats
hexa-bot selftest      # 4-verb spec presence sweep
hexa-bot verify        # run verify/cli.py — n=6 + verbs + cross-doc + roadmap
hexa-bot build         # pandoc PDF rebuild for 4 verb specs
hexa-bot sim           # host-side dog_robot firmware simulator
hexa-bot firmware      # STM32F429 ARM cross-compile of dog_robot reference
hexa-bot n6            # n=6 lattice arithmetic (σ·φ = n·τ = J₂ = 24)
```

---

## Cross-link

`hexa-bot` is a sibling substrate in the `dancinlab` n=6 family:

- [`dancinlab/hexa-codex`](https://github.com/dancinlab/hexa-codex)
  — AI substrate cousin (cognition / language)
- [`dancinlab/anima`](https://github.com/dancinlab/anima)
  — consciousness substrate cousin (subjective-experience axis)
- [`dancinlab/hexa-bio`](https://github.com/dancinlab/hexa-bio)
  — molecular substrate cousin (4-verb molecular toolkit; sister precedent)
- [`dancinlab/lumiere`](https://github.com/dancinlab/lumiere)
  — apps-axis spike (camera-filter + hexa-main-character; iOS standalone)

Upstream canon: [`dancinlab/canon`](https://github.com/dancinlab/canon)
@ commit `c0f1f570` (extraction reference SHA).

Planned post-v1.0 cousin: `dancinlab/hexa-mobility` — autonomous
mobility (motorcycle + drone + autonomous-vehicle 자율 이동체 묶음).

---

## License

MIT. See [LICENSE](LICENSE).

Copyright (c) 2026 박민우 / dancinlab.
