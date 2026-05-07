# Changelog — hexa-bot

All notable changes to `hexa-bot` (need-singularity Robot substrate) are
documented in this file.

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
Versioning: [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html)

---

## [1.1.0] — 2026-05-07

### Added — runnable surface (pattern source: `hexa-sscb` reference)

- `verify/` — atlas-anchored invariant audit (Python stdlib, 5 scripts):
  - `n6_arithmetic.py`  — σ(6)·φ(6) = n·τ(6) = J₂ = 24 identity
  - `verb_presence.py`  — 4-verb spec corpus presence sweep
  - `cross_doc_audit.py`— n=6 lattice + 4-verb table consistency across
                          README / CHANGELOG / RELEASE_NOTES / .roadmap / hexa.toml
  - `roadmap_check.py`  — `.roadmap.hexa_bot` sections + monotonic versioning
  - `firmware_sync.py`  — firmware header ↔ engineering doc sync
  - `cli.py` unified dispatcher (subcommands: n6 / verbs / audit / roadmap /
                                 firmware / all + --json + --quiet + --list)
- `build/` — pandoc PDF generator for the 4 verb specs (Makefile + CJK header).
  - 4 PDFs: `hexa-bot_{robotics,transport,automation,dog_robot}.pdf`
  - `make -C build all` / `check` / `check-soft` / `clean` / `verify` / `firmware`
    / `sim` / `all-targets`
- `tests/` — pytest acceptance scaffold (`test_acceptance.py`, A-1 .. A-10) +
  `conftest.py` markers (auto / bench / hardware / cert).
  - 13 auto checks (12 PASS, 1 SKIP for missing v1.x git tag).
  - `test_selftest.hexa` (the v1.0.0 sentinel) preserved untouched.
- `module/firmware/` — STM32F429ZIT6 reference firmware (CMSIS-only):
  - `include/dog_robot.h` — n=6 lattice projection (τ(6)=4 gait phases ·
    σ(6)=12 joints · 4 legs · 3 DOF/leg) with `_Static_assert` guards.
  - `include/stm32f4xx_min.h` — minimal CMSIS register stub (3 build modes:
    bare ARM / `USE_VENDOR_CMSIS` / `SIM_BUILD`).
  - `src/main.c`, `gait_sm.c`, `servo_pwm.c`, `leg_kinematics.c`, `system_init.c`
  - `startup/startup_stm32f429.s` + `linker/stm32f429zi.ld`
  - ARM cross-compile: 824 B text + 2608 B BSS via `arm-none-eabi-gcc 16.1.0`.
- `module/firmware/sim/` — host-side simulator (POSIX C, no ARM):
  - `dog_sim` binary with `--gait=stand|trot|bound|gallop` flag, JSON / CSV
    / verbose output modes.
  - `--selftest`: 4/4 gait pattern coherence sweep.
  - `mock_arch.h` + `mock_hardware.c` collapse STM32 MMIO onto plain memory.
- `module/firmware/test/` — xunit-style unit tests (8 tests):
  - τ(6)=4, σ(6)=12 lattice macro consistency
  - STAND holds STANCE; TROT diagonal-pair alignment + visits all 4 phases
  - BOUND front/rear pair alignment; servo bounds; state machine BOOT→LOCOMOTE
  - dog_joint_index bijection across 12-element joint surface
- `.gitignore` — build artifacts (PDFs, .elf, .bin, __pycache__, etc.).

### Changed

- `cli/hexa-bot.hexa` extended with `verify` / `build` / `sim` / `firmware` /
  `n6` subcommands (existing 4 verb subcmds + status + selftest preserved).
- `README.md` — Module inventory table, Build & verify section, validation
  sweep, runnable artifact diagram updated.

### Status (raw#10 honest C3)

- 1/4 verbs (`dog_robot`) now ships a register-level firmware reference + host
  sim + unit tests — n=6 lattice is enforced at compile time via _Static_assert.
- 3/4 verbs (`robotics`, `transport`, `automation`) still spec-only; their
  closed-form .hexa numerical paths are v1.2.0 / v2.0.0 work per `.roadmap`.
- ROS2 bridge + UL/KC certification still OUT-OF-SCOPE.

---

## [1.0.0] — 2026-05-06

### Added

- Initial extraction from `n6-architecture@c0f1f570`.
- 4-verb spec corpus (HEXA family pattern):
  - `robotics/robotics.md`        ← `domains/infra/robotics/`
  - `transport/robotics-transport.md` ← `domains/infra/robotics-transport/`
  - `automation/control-automation.md` ← `domains/infra/control-automation/`
  - `dog_robot/dog-robot-test.md` ← `domains/life/dog-robot-test/`
- `cli/hexa-bot.hexa` placeholder dispatcher (4 sub-command + status +
  selftest + help + version).
- `hexa.toml` package manifest (MIT, hexa-lang ≥1.0.0).
- `install.hexa` hx install hook (no-op pre, selftest post).
- `tests/test_selftest.hexa` — 4-verb spec presence sentinel test.
- `LICENSE` (MIT).

### Status (raw#10 honest C3)

- 0/4 verbs empirically wired. All 4 ship as `.md` spec corpus.
- Working `.hexa` numerical sandbox deferred to post-v1.0 cycles.
- motorcycle / drone / autonomous-vehicle stack OUT-OF-SCOPE — planned
  for `need-singularity/hexa-mobility` standalone.

### Cross-link

- Sister: `need-singularity/hexa-bio` (molecular, 1/4 wired)
- Cousin: `need-singularity/hexa-codex` (AI substrate)
- Cousin: `need-singularity/anima` (consciousness substrate)
- Cousin: `need-singularity/lumiere` (apps-axis spike)

### Provenance

- Extracted-from: `n6-architecture@c0f1f570`
- Extraction date: 2026-05-06 (UTC)
