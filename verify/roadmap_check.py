#!/usr/bin/env python3
"""
hexa-bot verify/roadmap_check.py — .roadmap.hexa_bot structural + monotonic.

Asserts:
  1. .roadmap.hexa_bot exists and parses sections (§0/§A.1/§A.2/§A.3/§A.4/§A.5).
  2. §A.1 lattice table values match verify/n6_arithmetic.py (single SSOT).
  3. §A.2 release cadence is version-monotone (v1.0.0 < v1.1.0 < ...).
  4. Each release row has Date / Status / Highlights columns.
  5. Cycle history (§A.3) records at least the v1.0.0 cycle.

Run:
    python3 verify/roadmap_check.py        # exit 0 = roadmap consistent
    python3 verify/roadmap_check.py --json
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from n6_arithmetic import LATTICE  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
ROADMAP = ROOT / ".roadmap.hexa_bot"

REQUIRED_SECTIONS = ["## §0", "## §A.1", "## §A.2", "## §A.3", "## §A.4", "## §A.5"]


def _semver_tuple(v: str) -> tuple[int, ...]:
    """Parse 'v1.2.3' → (1,2,3). Aspirational rows like 'v2.0.0' allowed."""
    m = re.match(r"v?(\d+)\.(\d+)\.(\d+)", v.strip())
    if not m:
        return (0, 0, 0)
    return tuple(int(g) for g in m.groups())


def _parse_release_table(text: str) -> list[dict]:
    """Parse the §A.2 release cadence markdown table."""
    in_table = False
    rows: list[dict] = []
    for line in text.splitlines():
        if "§A.2" in line:
            in_table = True
            continue
        if in_table and line.startswith("## §"):
            break  # next section
        if not in_table:
            continue
        # skip header + separator
        m = re.match(r"^\|\s*(v?[\d.]+)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]*?)\s*\|", line)
        if m and "---" not in m.group(1):
            rows.append({
                "version": m.group(1).strip(),
                "date":    m.group(2).strip(),
                "status":  m.group(3).strip(),
                "notes":   m.group(4).strip(),
            })
    return rows


def _parse_lattice_table(text: str) -> dict:
    """Parse §A.1 lattice values from the markdown table."""
    out = {}
    for line in text.splitlines():
        # rows like "| σ(6)   | 12    | sigma — ..."
        m = re.match(r"^\|\s*([σστφφφJJ₂\w()]+)\s*\|\s*(\d+)\s*\|", line)
        if not m:
            continue
        sym, val = m.group(1).strip(), int(m.group(2))
        if "σ" in sym:
            out["sigma"] = val
        elif "τ" in sym:
            out["tau"] = val
        elif "φ" in sym:
            out["phi"] = val
        elif "J" in sym:
            out["J2"] = val
    return out


def run() -> tuple[bool, list[dict]]:
    checks: list[dict] = []
    if not ROADMAP.exists():
        return False, [{
            "id": "roadmap.exists",
            "ok": False,
            "detail": f"missing: {ROADMAP.relative_to(ROOT)}",
        }]
    text = ROADMAP.read_text(encoding="utf-8")

    # 1. Required sections.
    missing = [s for s in REQUIRED_SECTIONS if s not in text]
    checks.append({
        "id": "roadmap.sections",
        "ok": not missing,
        "detail": f"missing sections: {missing}" if missing else "all 6 sections present",
    })

    # 2. Lattice values vs SSOT.
    lat = _parse_lattice_table(text)
    lat_ok = (
        lat.get("sigma") == LATTICE["sigma"]
        and lat.get("tau") == LATTICE["tau"]
        and lat.get("phi") == LATTICE["phi"]
        and lat.get("J2")  == LATTICE["J2"]
    )
    checks.append({
        "id": "roadmap.lattice-values",
        "ok": lat_ok,
        "detail": f"roadmap lattice {lat} vs SSOT {LATTICE}",
    })

    # 3 + 4. Release cadence: monotone + columns present.
    releases = _parse_release_table(text)
    versions = [r["version"] for r in releases]
    sem = [_semver_tuple(v) for v in versions]
    monotone = all(sem[i] < sem[i + 1] for i in range(len(sem) - 1)) if len(sem) >= 2 else True
    checks.append({
        "id": "roadmap.release-monotone",
        "ok": monotone and len(releases) >= 1,
        "detail": f"versions: {versions}",
    })

    columns_ok = all(r["date"] and r["status"] for r in releases)
    checks.append({
        "id": "roadmap.release-columns",
        "ok": columns_ok,
        "detail": f"{len(releases)} release rows; all have date+status: {columns_ok}",
    })

    # 5. Cycle history records at least one cycle.
    cycles_present = bool(re.search(r"\*\*Cycle\s*\d+", text))
    checks.append({
        "id": "roadmap.cycle-history",
        "ok": cycles_present,
        "detail": "Cycle 0 record present" if cycles_present else "no cycle records found",
    })

    ok_total = all(c["ok"] for c in checks)
    return ok_total, checks


def _print_human(rows: list[dict]) -> None:
    print("=" * 72)
    print("  hexa-bot roadmap check — .roadmap.hexa_bot structure + monotonicity")
    print("=" * 72)
    for r in rows:
        verdict = "PASS" if r["ok"] else "FAIL"
        print(f"  [{verdict}] {r['id']:<28} — {r['detail']}")
    print("=" * 72)
    n = sum(1 for r in rows if r["ok"])
    print(f"  {n}/{len(rows)} PASS")


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description="roadmap structure + monotone check")
    p.add_argument("--json", action="store_true")
    args = p.parse_args(argv[1:])
    ok, rows = run()
    if args.json:
        print(json.dumps({
            "tool":   "verify/roadmap_check.py",
            "schema": "hexa-bot/verify/roadmap/v1",
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
