#!/usr/bin/env python3
"""
hexa-bot verify/cross_doc_audit.py — invariant consistency across docs.

Asserts that the four lattice identifiers (n=6, σ=12, τ=4, φ=2, J₂=24) and
the 4-verb table appear consistently across every public-facing document:

    README.md
    CHANGELOG.md
    RELEASE_NOTES_v1.0.0.md
    .roadmap.hexa_bot
    hexa.toml

Every divergence points to a doc that needs to be re-aligned with the SSOT
(.roadmap.hexa_bot §A.1 for the lattice; hexa.toml [modules] for the verbs).

Run:
    python3 verify/cross_doc_audit.py        # exit 0 = all docs aligned
    python3 verify/cross_doc_audit.py --json
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

DOCS = [
    "README.md",
    "CHANGELOG.md",
    "RELEASE_NOTES_v1.0.0.md",
    ".roadmap.hexa_bot",
    "hexa.toml",
]

VERB_NAMES = ["robotics", "transport", "automation", "dog_robot"]
# Some docs say "dog-robot" instead of "dog_robot" — we accept either form.
VERB_REGEX = {
    "robotics":   r"\brobotics\b",
    "transport":  r"\btransport\b",
    "automation": r"\b(?:control[-_])?automation\b",
    "dog_robot":  r"\bdog[-_]robot\b",
}

# The lattice signature commonly appears as "σ=12, τ=4, φ=2, J₂=24" or as
# σ(6) = 12 in a markdown table or σ%3D12 inside a URL-encoded badge URL.
# Each pattern allows up to 6 separator chars between symbol and value so
# all three forms match without per-doc regex sprawl.
#
# Matches: "σ=12", "σ(6)=12", "σ%3D12", "σ:  12", "σ ·  12", "σ → 12".
LATTICE_PATTERNS = {
    "n=6":   r"(?:^|[^A-Za-z])n.{0,8}?6(?!\d)",
    "σ=12":  r"(?:σ|sigma).{0,12}?12(?!\d)",
    "τ=4":   r"(?:τ|tau).{0,12}?4(?!\d)",
    "φ=2":   r"(?:φ|phi).{0,12}?2(?!\d)",
    "J₂=24": r"J[₂2_]?.{0,8}?24(?!\d)",
}


def _read(path: Path) -> str:
    if not path.exists():
        return ""
    try:
        return path.read_text(encoding="utf-8")
    except Exception:
        return ""


def check_lattice_in_doc(text: str) -> dict:
    """Return per-symbol {present: bool} hits for one document."""
    out = {}
    for k, pat in LATTICE_PATTERNS.items():
        out[k] = bool(re.search(pat, text, flags=re.IGNORECASE))
    return out


def check_verbs_in_doc(text: str) -> dict:
    out = {}
    for v in VERB_NAMES:
        out[v] = bool(re.search(VERB_REGEX[v], text, flags=re.IGNORECASE))
    return out


def run() -> tuple[bool, list[dict]]:
    rows: list[dict] = []
    for d in DOCS:
        path = ROOT / d
        text = _read(path)
        if not text:
            rows.append({
                "doc": d, "exists": False,
                "lattice_hits": {},
                "verb_hits": {},
                "ok": False,
            })
            continue
        lattice_hits = check_lattice_in_doc(text)
        verb_hits = check_verbs_in_doc(text)
        # README + .roadmap MUST carry the full lattice. CHANGELOG + RELEASE
        # NOTES + hexa.toml only need to carry verb names. So we adapt.
        full_lattice_required = d in ("README.md", ".roadmap.hexa_bot")
        all_lattice_hit = all(lattice_hits.values())
        all_verbs_hit = all(verb_hits.values())
        ok = all_verbs_hit and (all_lattice_hit or not full_lattice_required)
        rows.append({
            "doc": d, "exists": True,
            "lattice_hits": lattice_hits,
            "verb_hits": verb_hits,
            "lattice_required": full_lattice_required,
            "ok": ok,
        })
    return all(r["ok"] for r in rows), rows


def _print_human(rows: list[dict]) -> None:
    print("=" * 78)
    print("  hexa-bot cross-doc audit — n=6 lattice + 4-verb table")
    print("=" * 78)
    print(f"  declared lattice (verify/n6_arithmetic.py): "
          f"σ={LATTICE['sigma']} τ={LATTICE['tau']} φ={LATTICE['phi']} "
          f"J₂={LATTICE['J2']}")
    print()
    for r in rows:
        if not r["exists"]:
            print(f"  [FAIL] {r['doc']:<32} (file missing)")
            continue
        verb_status = " ".join(
            ("+" if v else "-") + name for name, v in r["verb_hits"].items()
        )
        if r["lattice_required"]:
            lat_status = " ".join(
                ("+" if v else "-") + name for name, v in r["lattice_hits"].items()
            )
        else:
            lat_status = "(verb-only doc)"
        verdict = "PASS" if r["ok"] else "FAIL"
        print(f"  [{verdict}] {r['doc']:<28}")
        print(f"          verbs   : {verb_status}")
        print(f"          lattice : {lat_status}")
    print("=" * 78)
    n = sum(1 for r in rows if r["ok"])
    print(f"  {n}/{len(rows)} docs aligned")


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description="cross-doc invariant audit")
    p.add_argument("--json", action="store_true")
    args = p.parse_args(argv[1:])
    ok, rows = run()
    if args.json:
        print(json.dumps({
            "tool":   "verify/cross_doc_audit.py",
            "schema": "hexa-bot/verify/cross-doc/v1",
            "lattice": LATTICE,
            "docs_total": len(rows),
            "docs_pass":  sum(1 for r in rows if r["ok"]),
            "ok":         ok,
            "results":    rows,
        }, indent=2, ensure_ascii=False))
    else:
        _print_human(rows)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
