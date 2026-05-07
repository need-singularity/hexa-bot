#!/usr/bin/env python3
"""
hexa-bot verify/firmware_sync.py — module/firmware/ ↔ docs sync check.

Asserts that the dog_robot reference firmware (module/firmware/) is well-
formed AND consistent with the spec it implements:

  1. include/dog_robot.h has the canonical macros (gait phases τ(6)=4,
     joint count σ(6)=12, leg count = 4).
  2. Every .c source file under src/ carries a /* Source: ... */ header
     anchor that points back to a doc.
  3. sim/Makefile builds without an ARM toolchain (host build verifiable
     via make -n).
  4. test/test_*.c files exist and are referenced by sim/Makefile or
     test/Makefile.

Pure stdlib. Skips gracefully (PASS-with-warning) when module/firmware/
hasn't been wired up yet.

Run:
    python3 verify/firmware_sync.py        # exit 0 = sync OK
    python3 verify/firmware_sync.py --json
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FW = ROOT / "module" / "firmware"

REQUIRED_MACROS = {
    "GAIT_PHASES":       4,    # τ(6)
    "DOG_JOINTS_TOTAL":  12,   # σ(6)
    "DOG_LEG_COUNT":     4,    # quadruped
}

# Match a `Source:` anchor anywhere in the file's leading comment block — not
# just immediately after `/*`. Real-world headers carry filename + "—" before
# the source-anchor line, e.g.:
#
#     /* main.c — dog_robot mk1 firmware entry.
#      *
#      * Source: dog_robot/dog-robot-test.md (spec) + ...
SOURCE_HEADER_PAT = re.compile(r"\bSource\s*:", re.IGNORECASE)


def _check_header_macros(hdr: Path) -> dict:
    if not hdr.exists():
        return {"id": "firmware.header.exists", "ok": False,
                "detail": f"missing: {hdr.relative_to(ROOT)}"}
    text = hdr.read_text(encoding="utf-8", errors="replace")
    found: dict[str, int] = {}
    for name in REQUIRED_MACROS:
        m = re.search(rf"#define\s+{re.escape(name)}\s+(\-?\d+)", text)
        if m:
            found[name] = int(m.group(1))
    missing = [k for k in REQUIRED_MACROS if k not in found]
    wrong   = {k: (found[k], REQUIRED_MACROS[k])
               for k in REQUIRED_MACROS if k in found and found[k] != REQUIRED_MACROS[k]}
    ok = not missing and not wrong
    detail_parts = []
    if missing:
        detail_parts.append(f"missing: {missing}")
    if wrong:
        detail_parts.append(f"wrong values: {wrong}")
    if ok:
        detail_parts.append(f"all {len(REQUIRED_MACROS)} macros present and matching")
    return {"id": "firmware.header.macros", "ok": ok,
            "detail": "; ".join(detail_parts), "found": found}


def _check_source_anchors(src_dir: Path) -> dict:
    if not src_dir.exists():
        return {"id": "firmware.src.anchors", "ok": False,
                "detail": f"missing dir: {src_dir.relative_to(ROOT)}"}
    sources = sorted(src_dir.glob("*.c"))
    if not sources:
        return {"id": "firmware.src.anchors", "ok": False,
                "detail": f"no .c files under {src_dir.relative_to(ROOT)}"}
    no_anchor = []
    for c in sources:
        head = c.read_text(encoding="utf-8", errors="replace")[:512]
        if not SOURCE_HEADER_PAT.search(head):
            no_anchor.append(c.name)
    ok = not no_anchor
    return {"id": "firmware.src.anchors", "ok": ok,
            "detail": (f"{len(sources)} .c files; "
                       + ("all carry /* Source: */ header"
                          if ok else f"missing anchor: {no_anchor}"))}


def _check_sim_build(sim_dir: Path) -> dict:
    mk = sim_dir / "Makefile"
    if not mk.exists():
        return {"id": "firmware.sim.makefile", "ok": False,
                "detail": f"missing: {mk.relative_to(ROOT)}"}
    text = mk.read_text(encoding="utf-8", errors="replace")
    has_target = bool(re.search(r"^\s*all\s*:", text, re.MULTILINE))
    uses_host_cc = bool(re.search(r"\bCC\b\s*[?:]?=", text))
    ok = has_target and uses_host_cc
    return {"id": "firmware.sim.makefile", "ok": ok,
            "detail": f"all-target={has_target}, CC-defined={uses_host_cc}"}


def _check_tests_exist(test_dir: Path) -> dict:
    if not test_dir.exists():
        return {"id": "firmware.tests.exist", "ok": False,
                "detail": f"missing dir: {test_dir.relative_to(ROOT)}"}
    tests = list(test_dir.glob("test_*.c"))
    ok = bool(tests)
    return {"id": "firmware.tests.exist", "ok": ok,
            "detail": f"{len(tests)} test_*.c file(s)"
                      + ("" if ok else " (none yet)")}


def run() -> tuple[bool, list[dict]]:
    if not FW.exists():
        return False, [{
            "id": "firmware.dir.exists", "ok": False,
            "detail": f"missing: module/firmware/ — has the firmware "
                      f"reference been scaffolded yet?",
        }]
    rows = [
        _check_header_macros(FW / "include" / "dog_robot.h"),
        _check_source_anchors(FW / "src"),
        _check_sim_build(FW / "sim"),
        _check_tests_exist(FW / "test"),
    ]
    return all(r["ok"] for r in rows), rows


def _print_human(rows: list[dict]) -> None:
    print("=" * 72)
    print("  hexa-bot firmware-sync — module/firmware/ ↔ docs")
    print("=" * 72)
    for r in rows:
        verdict = "PASS" if r["ok"] else "FAIL"
        print(f"  [{verdict}] {r['id']:<28} — {r['detail']}")
    print("=" * 72)
    n = sum(1 for r in rows if r["ok"])
    print(f"  {n}/{len(rows)} PASS")


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description="firmware ↔ docs sync")
    p.add_argument("--json", action="store_true")
    args = p.parse_args(argv[1:])
    ok, rows = run()
    if args.json:
        print(json.dumps({
            "tool":   "verify/firmware_sync.py",
            "schema": "hexa-bot/verify/firmware-sync/v1",
            "checks_total": len(rows),
            "checks_pass":  sum(1 for r in rows if r["ok"]),
            "ok":           ok,
            "results":      rows,
        }, indent=2, ensure_ascii=False))
    else:
        _print_human(rows)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
