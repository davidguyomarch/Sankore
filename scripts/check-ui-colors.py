#!/usr/bin/env python3
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only
"""
#297 guard: prevent NEW hard-coded UI color literals from creeping into the
widget / QML layer while the existing ones are migrated to UBThemeManager,
lot by lot.

It counts hard-coded color literals (hex `#RGB`/`#RRGGBB`, `rgb(...)`,
`rgba(...)`) under the UI directories and compares the total to a committed
baseline. The check FAILS only if the count goes UP. As lots land, lower the
baseline with `--update-baseline` so it ratchets down and can never grow again.

Genuine non-theme literals (file-type magic numbers, Qt version guards, SVG
document content) live outside these UI dirs or are excluded by path below.

Usage:
  scripts/check-ui-colors.py                 # check against baseline
  scripts/check-ui-colors.py --update-baseline
  scripts/check-ui-colors.py --list          # show every match (for triage)
"""

import argparse
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASELINE_PATH = os.path.join(ROOT, "scripts", "ui-colors-baseline.json")

# UI layers where colors must come from the theme.
SCAN_DIRS = ["src/gui", "src/qml", "src/board", "src/document"]
EXTENSIONS = (".cpp", ".h", ".qml")

# Files intentionally excluded (theme source of truth, or documented non-theme).
EXCLUDE_SUBPATHS = [
    os.path.join("src", "qml", "UBThemeManager.cpp"),
    os.path.join("src", "qml", "UBThemeManager.h"),
    # SVG export writes document content colors, not chrome:
    os.path.join("src", "adaptors"),  # not in SCAN_DIRS anyway; defensive
]

# Color literal patterns. Word-ish boundaries keep it simple and predictable.
# Hex requires 3 or 6 digits with at least one letter a-f OR being a plausible
# color, but to keep it robust we simply strip comments first (see strip_comment)
# so issue refs like "#297" or "was #FFB3C8" in comments are never counted.
PATTERNS = [
    re.compile(r"#[0-9a-fA-F]{6}\b"),        # #RRGGBB
    re.compile(r"#[0-9a-fA-F]{3}\b"),        # #RGB
    re.compile(r"\brgba?\s*\(", re.IGNORECASE),  # rgb( / rgba(
]

# Strip line/inline comments so color literals mentioned in comments (issue refs
# like "#297", or "was #FFB3C8") are not counted as real UI color literals.
_LINE_COMMENT = re.compile(r"//.*$")

def strip_comment(line: str) -> str:
    # Drop C++/JS line comments and QML/C comment bodies. This is a heuristic
    # (does not track multi-line /* */ state) but is enough: color literals we
    # care about live in real code (setStyleSheet, QColor, QML color: ...).
    line = _LINE_COMMENT.sub("", line)
    # Also drop the body after a leading '*' (inside /* ... */ blocks) or '#'
    # markdown-ish comment lines are not code files, so ignore.
    stripped = line.lstrip()
    if stripped.startswith("*"):
        return ""
    return line


def is_excluded(path: str) -> bool:
    return any(sub in path for sub in EXCLUDE_SUBPATHS)


def count_matches(list_mode: bool = False) -> int:
    total = 0
    for d in SCAN_DIRS:
        base = os.path.join(ROOT, d)
        for dirpath, _dirs, files in os.walk(base):
            for fn in files:
                if not fn.endswith(EXTENSIONS):
                    continue
                full = os.path.join(dirpath, fn)
                if is_excluded(full):
                    continue
                rel = os.path.relpath(full, ROOT)
                try:
                    with open(full, "r", encoding="utf-8", errors="ignore") as fh:
                        for lineno, line in enumerate(fh, 1):
                            code = strip_comment(line)
                            hits = sum(len(p.findall(code)) for p in PATTERNS)
                            if hits:
                                total += hits
                                if list_mode:
                                    print(f"{rel}:{lineno}: {line.rstrip()}")
                except OSError:
                    continue
    return total


def load_baseline() -> int:
    if not os.path.exists(BASELINE_PATH):
        return None
    with open(BASELINE_PATH, "r", encoding="utf-8") as fh:
        return int(json.load(fh).get("count", 0))


def save_baseline(count: int) -> None:
    with open(BASELINE_PATH, "w", encoding="utf-8") as fh:
        json.dump({"count": count,
                   "note": "#297 ratchet: hard-coded UI color literals under "
                           "src/{gui,qml,board,document}. Never increase; lower "
                           "as lots migrate to UBThemeManager."},
                  fh, indent=2)
        fh.write("\n")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--update-baseline", action="store_true")
    ap.add_argument("--list", action="store_true")
    args = ap.parse_args()

    count = count_matches(list_mode=args.list)

    if args.update_baseline:
        save_baseline(count)
        print(f"Baseline updated: {count} UI color literals.")
        return 0

    baseline = load_baseline()
    if baseline is None:
        save_baseline(count)
        print(f"Baseline created: {count} UI color literals.")
        return 0

    print(f"UI color literals: {count} (baseline {baseline}).")
    if count > baseline:
        print(f"ERROR: {count - baseline} new hard-coded UI color literal(s) "
              f"introduced. Use UBThemeManager roles instead (issue #297). "
              f"Run scripts/check-ui-colors.py --list to locate them.",
              file=sys.stderr)
        return 1
    if count < baseline:
        print(f"Nice — {baseline - count} fewer than baseline. "
              f"Run --update-baseline to lock in the progress.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
