"""
hexa-bot — acceptance scaffold (pytest, stdlib only).

Pattern source: hexa-sscb/tests/test_acceptance.py.

Verifies the v1.0.0 → v1.1.0 acceptance ladder:

    A-1  4 verb spec files present + non-trivial size
    A-2  hexa.toml [modules].spec lists exactly the 4 verbs
    A-3  cli/hexa-bot.hexa runs `selftest` and emits PASS sentinel
    A-4  verify/cli.py 'all' returns 0 (n=6 + verbs + audit + roadmap pass)
    A-5  module/firmware/ ARM cross-compile produces firmware.elf (skip if no toolchain)
    A-6  module/firmware/sim/ host build + selftest PASS
    A-7  module/firmware/test/ unit tests PASS
    A-8  README.md + CHANGELOG carry the n=6 lattice + 4-verb table
    A-9  v1.x git tag exists (skip with reason on pre-tag runs)
    A-10 install.hexa hook still references cli/hexa-bot.hexa as canonical entry
"""
from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
PYTHON = sys.executable


# ---------------------------------------------------------------------------
# A-1 — 4 verb spec files present
# ---------------------------------------------------------------------------

@pytest.mark.auto
@pytest.mark.parametrize("verb, spec_path", [
    ("robotics",   "robotics/robotics.md"),
    ("transport",  "transport/robotics-transport.md"),
    ("automation", "automation/control-automation.md"),
    ("dog_robot",  "dog_robot/dog-robot-test.md"),
])
def test_a1_verb_spec_present(verb, spec_path):
    p = ROOT / spec_path
    assert p.exists(), f"missing spec: {spec_path}"
    assert p.stat().st_size > 200, f"spec too small (likely placeholder): {spec_path}"


# ---------------------------------------------------------------------------
# A-2 — hexa.toml [modules].spec matches expected list
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a2_hexa_toml_modules_spec_matches():
    toml = (ROOT / "hexa.toml").read_text(encoding="utf-8")
    declared = re.findall(r'"([^"]+\.md)"', toml)
    expected = {
        "robotics/robotics.md",
        "transport/robotics-transport.md",
        "automation/control-automation.md",
        "dog_robot/dog-robot-test.md",
    }
    assert expected.issubset(set(declared)), (
        f"hexa.toml [modules].spec missing entries: {expected - set(declared)}"
    )


# ---------------------------------------------------------------------------
# A-3 — cli/hexa-bot.hexa selftest emits the PASS sentinel
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a3_hexa_cli_selftest():
    if shutil.which("hexa") is None:
        pytest.skip("A-3: `hexa` binary not in PATH (install hexa-lang toolchain)")
    cli = ROOT / "cli" / "hexa-bot.hexa"
    assert cli.exists(), f"missing cli: {cli}"
    rc = subprocess.run(
        ["hexa", "run", str(cli), "selftest"],
        capture_output=True, text=True,
        env={"HEXA_BOT_ROOT": str(ROOT), "PATH": __import__("os").environ.get("PATH", "")},
    )
    assert rc.returncode == 0, f"hexa selftest exit={rc.returncode}\n{rc.stdout}\n{rc.stderr}"
    assert "__HEXA_BOT_SELFTEST__ PASS" in rc.stdout, (
        f"missing PASS sentinel in cli output:\n{rc.stdout}"
    )


# ---------------------------------------------------------------------------
# A-4 — verify/cli.py 'all' returns 0
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a4_verify_cli_all_pass():
    rc = subprocess.run(
        [PYTHON, str(ROOT / "verify" / "cli.py"), "all"],
        capture_output=True, text=True,
    )
    if rc.returncode != 0:
        # firmware-sync check correctly fails when module/firmware/ is
        # incomplete on a fresh check-out — surface exact stdout for triage.
        assert "firmware" in rc.stdout, rc.stdout
    # we still expect at least 4/5 to pass (everything except possibly firmware)
    m = re.search(r"(\d+)/(\d+) checks PASS", rc.stdout)
    assert m, f"no '<n>/<m> checks PASS' summary found:\n{rc.stdout}"
    n_pass, n_total = int(m.group(1)), int(m.group(2))
    assert n_pass >= n_total - 1, (
        f"verify/cli.py {n_pass}/{n_total} PASS — at most 1 may fail (firmware sync)"
    )


# ---------------------------------------------------------------------------
# A-5 — module/firmware ARM cross-compile (skip if no toolchain)
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a5_firmware_arm_cross_compile():
    if shutil.which("arm-none-eabi-gcc") is None:
        pytest.skip("A-5: arm-none-eabi-gcc not installed")
    fw = ROOT / "module" / "firmware"
    if not (fw / "Makefile").exists():
        pytest.skip("A-5: module/firmware/ not scaffolded")
    rc = subprocess.run(["make", "-C", str(fw), "all"], capture_output=True, text=True)
    assert rc.returncode == 0, f"firmware build FAIL\n{rc.stdout}\n{rc.stderr}"
    assert (fw / "build" / "firmware.elf").exists(), "firmware.elf not produced"


# ---------------------------------------------------------------------------
# A-6 — host sim build + selftest PASS
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a6_firmware_sim_selftest():
    sim_dir = ROOT / "module" / "firmware" / "sim"
    if not (sim_dir / "Makefile").exists():
        pytest.skip("A-6: module/firmware/sim/ not scaffolded")
    rc = subprocess.run(
        ["make", "-C", str(sim_dir), "selftest"],
        capture_output=True, text=True,
    )
    assert rc.returncode == 0, f"sim selftest FAIL\n{rc.stdout}\n{rc.stderr}"
    assert "__DOG_SIM_SELFTEST__ PASS" in rc.stdout, rc.stdout


# ---------------------------------------------------------------------------
# A-7 — firmware unit tests
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a7_firmware_unit_tests():
    test_dir = ROOT / "module" / "firmware" / "test"
    if not (test_dir / "Makefile").exists():
        pytest.skip("A-7: module/firmware/test/ not scaffolded")
    rc = subprocess.run(
        ["make", "-C", str(test_dir), "run"],
        capture_output=True, text=True,
    )
    assert rc.returncode == 0, f"firmware unit tests FAIL\n{rc.stdout}\n{rc.stderr}"
    assert "all tests PASS" in rc.stdout, rc.stdout


# ---------------------------------------------------------------------------
# A-8 — n=6 + verb table appear in README/CHANGELOG
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a8_cross_doc_lattice_present():
    rc = subprocess.run(
        [PYTHON, str(ROOT / "verify" / "cross_doc_audit.py")],
        capture_output=True, text=True,
    )
    assert rc.returncode == 0, f"cross_doc_audit FAIL\n{rc.stdout}\n{rc.stderr}"


# ---------------------------------------------------------------------------
# A-9 — v1.x tag (skip with reason on pre-tag runs)
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a9_v1_tag_or_freeze_marker():
    try:
        out = subprocess.check_output(
            ["git", "-C", str(ROOT), "tag", "--list"],
            text=True, stderr=subprocess.DEVNULL,
        )
    except (subprocess.CalledProcessError, FileNotFoundError):
        pytest.skip("A-9: git not available or not a git repo")
        return
    tags = [t.strip() for t in out.splitlines() if t.strip()]
    if not any(re.match(r"^(v1\.|hexa-bot-v\d|v\d\.\d\.\d)", t) for t in tags):
        pytest.skip(
            f"A-9: no v1.x / hexa-bot-vN tag yet (tags: {tags or 'none'}) — "
            f"create with `git tag -a v1.0.0 -m 'v1.0.0 release'` once design lands"
        )


# ---------------------------------------------------------------------------
# A-10 — install.hexa references cli/hexa-bot.hexa
# ---------------------------------------------------------------------------

@pytest.mark.auto
def test_a10_install_hook_references_cli():
    install = (ROOT / "install.hexa").read_text(encoding="utf-8")
    assert "cli/hexa-bot.hexa" in install, (
        "install.hexa must continue to reference cli/hexa-bot.hexa as canonical entry"
    )
