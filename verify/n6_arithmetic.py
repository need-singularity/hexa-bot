#!/usr/bin/env python3
"""
hexa-bot verify/n6_arithmetic.py — n=6 lattice arithmetic identity check.

Asserts the four lattice identifiers and the closing identity:

    σ(6) · φ(6) = n · τ(6) = J₂ = 24
       12  ·  2  =  6 ·  4  = 24

Source-of-truth: .roadmap.hexa_bot §A.1 (table) + README.md §"Why (n=6 robot
substrate)" + RELEASE_NOTES_v1.0.0.md (badge).

This is the only verifier in hexa-bot/verify/ that holds the lattice numerics
inline — every other check pulls them from here via the LATTICE dict so a
single edit propagates. Pure stdlib.

Run:
    python3 verify/n6_arithmetic.py        # exit 0 = identity holds
    python3 verify/n6_arithmetic.py --json # machine-readable

Authority: own 1 (n=6 master identity, projected onto robot axes).
"""
from __future__ import annotations

import argparse
import json
import sys
from math import gcd

# Canonical lattice (sole source of truth in this file).
LATTICE = {
    "n":     6,
    "sigma": 12,   # σ(6) — sum of divisors of 6 = 1+2+3+6
    "tau":   4,    # τ(6) — count of divisors  of 6 = {1,2,3,6}
    "phi":   2,    # φ(6) — Euler totient of 6 = #{1,5}
    "J2":    24,   # Mathieu/Leech-2 closing identity
}


def _divisors(n: int) -> list[int]:
    return sorted(d for d in range(1, n + 1) if n % d == 0)


def _euler_phi(n: int) -> int:
    return sum(1 for k in range(1, n + 1) if gcd(k, n) == 1)


def derive_from_n(n: int) -> dict:
    """Recompute σ/τ/φ from first principles to guard against typo drift."""
    divs = _divisors(n)
    return {
        "n":     n,
        "sigma": sum(divs),
        "tau":   len(divs),
        "phi":   _euler_phi(n),
    }


CHECKS = [
    ("n6.lattice.canonical-σ",   "σ(6) declared = 12",
     lambda L: L["sigma"] == 12),
    ("n6.lattice.canonical-τ",   "τ(6) declared = 4",
     lambda L: L["tau"]   == 4),
    ("n6.lattice.canonical-φ",   "φ(6) declared = 2",
     lambda L: L["phi"]   == 2),
    ("n6.lattice.canonical-J₂",  "J₂ declared = 24",
     lambda L: L["J2"]    == 24),
    ("n6.identity.sigma·phi",    "σ(6)·φ(6) = J₂",
     lambda L: L["sigma"] * L["phi"] == L["J2"]),
    ("n6.identity.n·tau",        "n·τ(6) = J₂",
     lambda L: L["n"]     * L["tau"] == L["J2"]),
    ("n6.derive-σ",              "derived σ(6) matches declared",
     lambda L: derive_from_n(L["n"])["sigma"] == L["sigma"]),
    ("n6.derive-τ",              "derived τ(6) matches declared",
     lambda L: derive_from_n(L["n"])["tau"]   == L["tau"]),
    ("n6.derive-φ",              "derived φ(6) matches declared",
     lambda L: derive_from_n(L["n"])["phi"]   == L["phi"]),
]


def run() -> tuple[int, list[dict]]:
    rows = []
    for cid, desc, fn in CHECKS:
        ok = bool(fn(LATTICE))
        rows.append({"id": cid, "desc": desc, "ok": ok})
    n_pass = sum(1 for r in rows if r["ok"])
    return n_pass, rows


def _print_human(rows: list[dict]) -> None:
    print("=" * 72)
    print("  hexa-bot n=6 arithmetic — lattice identity")
    print("=" * 72)
    print(f"  declared: n={LATTICE['n']}  σ={LATTICE['sigma']}  "
          f"τ={LATTICE['tau']}  φ={LATTICE['phi']}  J₂={LATTICE['J2']}")
    print(f"  identity: σ·φ = {LATTICE['sigma']*LATTICE['phi']}  ≟  "
          f"n·τ = {LATTICE['n']*LATTICE['tau']}  ≟  J₂ = {LATTICE['J2']}")
    print("-" * 72)
    for r in rows:
        mark = "PASS" if r["ok"] else "FAIL"
        print(f"  [{mark}] {r['id']:30s} — {r['desc']}")
    print("=" * 72)
    n_pass = sum(1 for r in rows if r["ok"])
    print(f"  {n_pass}/{len(rows)} PASS")


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description="n=6 lattice arithmetic identity")
    p.add_argument("--json", action="store_true", help="JSON output")
    args = p.parse_args(argv[1:])

    n_pass, rows = run()

    if args.json:
        print(json.dumps({
            "tool":   "verify/n6_arithmetic.py",
            "schema": "hexa-bot/verify/n6/v1",
            "lattice": LATTICE,
            "checks_total": len(rows),
            "checks_pass":  n_pass,
            "ok":           n_pass == len(rows),
            "results":      rows,
        }, indent=2, ensure_ascii=False))
    else:
        _print_human(rows)

    return 0 if n_pass == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
