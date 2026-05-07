#!/usr/bin/env python3
"""
hexa-bot verify/cli.py — unified verification CLI dispatcher.

One entry point for every verifier in hexa-bot/verify/. Each verifier is run
as a subprocess so its module state stays isolated; output is captured and
re-emitted human-readable or as JSON, with aggregated exit codes.

Usage:
    python3 verify/cli.py                # all-checks, human readable
    python3 verify/cli.py all            # same
    python3 verify/cli.py n6             # n=6 lattice arithmetic
    python3 verify/cli.py verbs          # 4-verb spec presence sweep
    python3 verify/cli.py audit          # cross-doc invariant audit
    python3 verify/cli.py roadmap        # .roadmap.hexa_bot structural check
    python3 verify/cli.py firmware       # firmware sync (header ↔ docs)
    python3 verify/cli.py --json         # machine-readable summary (all)
    python3 verify/cli.py --quiet        # only verdict line per check (all)
    python3 verify/cli.py --list         # list registered checks

Exit code:
    0 = every selected check passed
    1 = one or more checks failed
    2 = invocation error (unknown subcommand, missing file)

Pattern source-of-truth: hexa-sscb/verify/cli.py (referenced 2026-05-07).
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

VERIFY_DIR = Path(__file__).resolve().parent
ROOT = VERIFY_DIR.parent

# Ordered registry: subcommand → (script, human label).
CHECKS: list[tuple[str, str, str]] = [
    ("n6",       "n6_arithmetic.py",   "n=6 lattice arithmetic identity (σ·φ = n·τ = J₂)"),
    ("verbs",    "verb_presence.py",   "4-verb spec corpus presence sweep"),
    ("audit",    "cross_doc_audit.py", "cross-doc invariant + verb-table audit"),
    ("roadmap",  "roadmap_check.py",   ".roadmap.hexa_bot sections + monotonicity"),
    ("firmware", "firmware_sync.py",   "firmware header ↔ engineering doc sync"),
]
SCRIPT_FOR = {name: script for name, script, _ in CHECKS}
LABEL_FOR  = {name: label  for name, _, label  in CHECKS}


# ---- color helpers (off when stdout isn't a TTY or NO_COLOR set) ---------

def _use_color() -> bool:
    if os.environ.get("NO_COLOR"):
        return False
    return sys.stdout.isatty()

def _c(code: str, text: str) -> str:
    return f"\033[{code}m{text}\033[0m" if _use_color() else text

def _green(t: str) -> str:  return _c("32", t)
def _red(t: str)   -> str:  return _c("31", t)
def _yellow(t: str)-> str:  return _c("33", t)
def _bold(t: str)  -> str:  return _c("1",  t)


# ---- runner ---------------------------------------------------------------

def _run_one(name: str) -> dict:
    script = VERIFY_DIR / SCRIPT_FOR[name]
    if not script.exists():
        return {
            "name": name, "label": LABEL_FOR[name],
            "exit_code": 2, "duration_s": 0.0,
            "stdout": "", "stderr": f"missing script: {script}",
            "ok": False,
        }
    t0 = time.monotonic()
    rc = subprocess.run(
        [sys.executable, str(script)],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    dt = time.monotonic() - t0
    return {
        "name": name, "label": LABEL_FOR[name],
        "exit_code": rc.returncode, "duration_s": round(dt, 3),
        "stdout": rc.stdout, "stderr": rc.stderr,
        "ok": rc.returncode == 0,
    }


def _print_human(results: list[dict], quiet: bool) -> None:
    if not quiet:
        print()
    for r in results:
        if not quiet:
            print("=" * 72)
            print(f"  ▶ {_bold(r['name'])}  —  {r['label']}")
            print("=" * 72)
            if r["stdout"]:
                print(r["stdout"].rstrip("\n"))
            if r["stderr"].strip():
                print(_yellow(r["stderr"].rstrip("\n")))
        verdict = _green("PASS") if r["ok"] else _red("FAIL")
        if not quiet:
            print()
        print(f"  [{verdict}] {r['name']:9s}  ({r['duration_s']:5.2f}s)  {r['label']}")
    print("=" * 72)
    n = len(results)
    n_ok = sum(1 for r in results if r["ok"])
    line = f"  {n_ok}/{n} checks PASS"
    print(_bold(_green(line)) if n_ok == n else _bold(_red(line)))
    if n_ok != n:
        print("  failing:")
        for r in results:
            if not r["ok"]:
                print(f"    ✗ {r['name']}  exit={r['exit_code']}  {r['label']}")
    total_dt = sum(r["duration_s"] for r in results)
    print(f"  total wall time: {total_dt:.2f}s")
    print("=" * 72)


def _print_json(results: list[dict]) -> None:
    n = len(results)
    n_ok = sum(1 for r in results if r["ok"])
    out = {
        "tool":   "hexa-bot verify/cli.py",
        "schema": "hexa-bot/verify/cli/v1",
        "checks_total": n,
        "checks_pass":  n_ok,
        "checks_fail":  n - n_ok,
        "ok":           n_ok == n,
        "results":      [
            {k: v for k, v in r.items() if k != "stderr" or v.strip()}
            for r in results
        ],
    }
    print(json.dumps(out, indent=2, ensure_ascii=False))


def _build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="verify/cli.py",
        description="hexa-bot unified verifier CLI (n=6 master identity).",
    )
    p.add_argument("target", nargs="?", default="all",
                   help="check name (n6/verbs/audit/roadmap/firmware) or 'all'.")
    p.add_argument("--json",  action="store_true")
    p.add_argument("--quiet", action="store_true")
    p.add_argument("--list",  action="store_true")
    return p


def _print_listing() -> None:
    print(_bold("hexa-bot verify/ registered checks:"))
    for name, script, label in CHECKS:
        exists = (VERIFY_DIR / script).exists()
        mark = _green("ok ") if exists else _red("MISS")
        print(f"  [{mark}] {name:9s} → verify/{script:24s}  {label}")
    print()
    present = sum(1 for _, s, _ in CHECKS if (VERIFY_DIR / s).exists())
    print(f"  total: {len(CHECKS)} check(s)  ·  present: {present}/{len(CHECKS)}")


def main(argv: list[str]) -> int:
    args = _build_parser().parse_args(argv[1:])

    if args.list:
        _print_listing()
        return 0

    if args.target == "all":
        names = [n for n, _, _ in CHECKS]
    elif args.target in SCRIPT_FOR:
        names = [args.target]
    else:
        print(_red(f"unknown check: {args.target!r}"), file=sys.stderr)
        print("known checks:", ", ".join(n for n, _, _ in CHECKS), file=sys.stderr)
        return 2

    results: list[dict] = []
    for n in names:
        results.append(_run_one(n))

    if args.json:
        _print_json(results)
    else:
        _print_human(results, args.quiet)

    return 0 if all(r["ok"] for r in results) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
