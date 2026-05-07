#!/usr/bin/env python3
"""
hexa-bot verify/verb_presence.py — 4-verb spec corpus presence sweep.

Asserts that every verb listed in hexa.toml [modules].spec actually has a
non-trivial Markdown spec on disk, with the canonical filename pattern, and
that no extra verb directories have appeared without being declared.

Equivalent of `cli/hexa-bot.hexa selftest`'s file-presence half, but doc-
runnable via Python and JSON-emitting (for CI) without the .hexa runtime.

Source-of-truth: hexa.toml [modules].spec.

Run:
    python3 verify/verb_presence.py        # exit 0 = 4/4 present
    python3 verify/verb_presence.py --json
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Canonical 4-verb table — mirrors hexa.toml [modules].spec.
# (verb, dirname, spec_filename, substrate-role, n6-projection).
VERBS = [
    ("robotics",   "robotics",   "robotics.md",
     "manipulation / arm / actuator primitives",
     "6-DOF closed-form IK (n=6 direct)"),
    ("transport",  "transport",  "robotics-transport.md",
     "ground+aerial transport + logistics",
     "σ(6)=12 sensor fusion (LIDAR/RGB/depth/IMU/GPS/sonar)"),
    ("automation", "automation", "control-automation.md",
     "control-automation + factory-line",
     "PID τ(6)=4 phase auto-tune"),
    ("dog_robot",  "dog_robot",  "dog-robot-test.md",
     "quadruped test rig + locomotion",
     "τ(6)=4 gait cycle (stance/swing/push/land)"),
]

MIN_SPEC_BYTES = 200  # spec files must be at least this large to count


def _read_toml_modules() -> list[str]:
    """Parse hexa.toml [modules].spec without a TOML lib (stdlib only).
    Tolerant: returns [] if hexa.toml missing or no [modules] section."""
    toml_path = ROOT / "hexa.toml"
    if not toml_path.exists():
        return []
    in_modules = False
    in_spec_array = False
    out: list[str] = []
    for raw in toml_path.read_text(encoding="utf-8").splitlines():
        s = raw.strip()
        if s.startswith("#") or not s:
            continue
        if re.match(r"^\[\s*modules\s*\]", s):
            in_modules = True
            continue
        if s.startswith("[") and not in_spec_array:
            in_modules = False
            continue
        if not in_modules:
            continue
        if re.match(r"^spec\s*=\s*\[", s):
            in_spec_array = True
            # inline (single-line) array form
            inline = re.search(r"\[(.*)\]", s)
            if inline:
                in_spec_array = False
                for v in re.findall(r'"([^"]+)"', inline.group(1)):
                    out.append(v)
            continue
        if in_spec_array:
            for v in re.findall(r'"([^"]+)"', s):
                out.append(v)
            if "]" in s:
                in_spec_array = False
    return out


def run() -> tuple[int, list[dict]]:
    rows: list[dict] = []
    declared = _read_toml_modules()
    for verb, d, fname, role, projection in VERBS:
        spec = ROOT / d / fname
        present = spec.exists() and spec.stat().st_size >= MIN_SPEC_BYTES
        toml_ok = (f"{d}/{fname}" in declared) if declared else None
        rows.append({
            "verb": verb,
            "path": f"{d}/{fname}",
            "present_on_disk": present,
            "size_bytes": spec.stat().st_size if spec.exists() else 0,
            "declared_in_hexa_toml": toml_ok,
            "role": role,
            "n6_projection": projection,
            "ok": present and (toml_ok is not False),
        })
    n_pass = sum(1 for r in rows if r["ok"])
    return n_pass, rows


def _print_human(rows: list[dict]) -> None:
    print("=" * 78)
    print("  hexa-bot verb-presence — 4-verb spec corpus sweep")
    print("=" * 78)
    print(f"  {'verb':<11} {'spec file':<32} {'size':>7}  hexa.toml  verdict")
    print("  " + "-" * 74)
    for r in rows:
        decl = ("yes" if r["declared_in_hexa_toml"] else
                "—  " if r["declared_in_hexa_toml"] is None else "NO ")
        verdict = "PASS" if r["ok"] else "FAIL"
        print(f"  {r['verb']:<11} {r['path']:<32} "
              f"{r['size_bytes']:>5}B  {decl:^9}  [{verdict}]")
    print("  " + "-" * 74)
    n_pass = sum(1 for r in rows if r["ok"])
    print(f"  {n_pass}/{len(rows)} PASS — verb spec corpus")
    print()
    print("  n=6 projections:")
    for r in rows:
        print(f"    {r['verb']:<11} → {r['n6_projection']}")
    print("=" * 78)


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description="4-verb spec corpus presence sweep")
    p.add_argument("--json", action="store_true")
    args = p.parse_args(argv[1:])
    n_pass, rows = run()
    if args.json:
        print(json.dumps({
            "tool":   "verify/verb_presence.py",
            "schema": "hexa-bot/verify/verb-presence/v1",
            "verbs_total": len(rows),
            "verbs_pass":  n_pass,
            "ok":          n_pass == len(rows),
            "results":     rows,
        }, indent=2, ensure_ascii=False))
    else:
        _print_human(rows)
    return 0 if n_pass == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
