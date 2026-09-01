#!/usr/bin/env python3
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only
"""
Validate sbom.spdx.json against the project.

Checks performed:
  1. sbom.spdx.json is well-formed JSON and declares SPDX-2.3.
  2. Every package has the required SPDX fields.
  3. The top-level package version matches VERSION_MAJ.VERSION_MIN.VERSION_PATCH
     read from OpenSankore.pro.
  4. Every component named in THIRD_PARTY.md tables also appears in the SBOM,
     and vice versa (drift detection).

This is intentionally a *validation* tool, not a generator: the project has no
single build manifest an automatic scanner could rely on, so the SBOM is
curated by hand and this script keeps it honest.

Exit code 0 = OK, 1 = problem found.
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SBOM_PATH = ROOT / "sbom.spdx.json"
PRO_PATH = ROOT / "OpenSankore.pro"
THIRD_PARTY_PATH = ROOT / "THIRD_PARTY.md"

REQUIRED_PACKAGE_FIELDS = [
    "name",
    "SPDXID",
    "versionInfo",
    "downloadLocation",
    "licenseConcluded",
    "licenseDeclared",
]

# Human-readable component names in THIRD_PARTY.md tables mapped to the
# canonical SBOM package "name" field. Kept explicit so a rename in either
# place is caught rather than silently accepted.
NAME_MAP = {
    "Qt 6": "Qt",
    "OpenSSL": "OpenSSL",
    "QuaZip": "QuaZip",
    "zlib": "zlib",
    "pdf-merger": "pdf-merger",
    "Zinnia": "Zinnia",
    "Phosphor Icons (regular)": "Phosphor-Icons",
    "Font Awesome 4": "Font-Awesome-4",
    "Andika Basic": "Andika-Basic",
    "Écolier Court": "Ecolier-Court",
    "Écolier Lignes Court": "Ecolier-Lignes-Court",
    "Marelle (8 variants)": "Marelle",
    "URW Base35 (14 files)": "URW-Base35",
}

errors = []


def read_pro_version():
    text = PRO_PATH.read_text(encoding="utf-8")

    def field(name):
        m = re.search(rf"^{name}\s*=\s*(\d+)", text, re.MULTILINE)
        return m.group(1) if m else None

    maj, minor, patch = field("VERSION_MAJ"), field("VERSION_MIN"), field("VERSION_PATCH")
    if None in (maj, minor, patch):
        errors.append("Could not parse VERSION_MAJ/MIN/PATCH from OpenSankore.pro")
        return None
    return f"{maj}.{minor}.{patch}"


def third_party_component_names():
    """Extract first-column values from the markdown tables in THIRD_PARTY.md."""
    names = set()
    for line in THIRD_PARTY_PATH.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        if not cells:
            continue
        first = cells[0]
        # Skip header/separator rows and non-component tables.
        if first in ("Component", "File", "") or set(first) <= set("-: "):
            continue
        if first in NAME_MAP:
            names.add(first)
    return names


def main():
    if not SBOM_PATH.exists():
        print(f"ERROR: {SBOM_PATH} does not exist")
        return 1

    try:
        sbom = json.loads(SBOM_PATH.read_text(encoding="utf-8"))
    except json.JSONDecodeError as e:
        print(f"ERROR: sbom.spdx.json is not valid JSON: {e}")
        return 1

    if sbom.get("spdxVersion") != "SPDX-2.3":
        errors.append(f"spdxVersion is {sbom.get('spdxVersion')!r}, expected 'SPDX-2.3'")

    packages = sbom.get("packages", [])
    if not packages:
        errors.append("SBOM has no packages")

    for pkg in packages:
        for f in REQUIRED_PACKAGE_FIELDS:
            if not pkg.get(f):
                errors.append(f"Package {pkg.get('name', '?')!r} missing required field {f!r}")

    # Top-level package version must match OpenSankore.pro.
    pro_version = read_pro_version()
    top = next((p for p in packages if p.get("SPDXID") == "SPDXRef-Package-OpenSankore"), None)
    if top is None:
        errors.append("No top-level package SPDXRef-Package-OpenSankore in SBOM")
    elif pro_version and top.get("versionInfo") != pro_version:
        errors.append(
            f"SBOM top-level version {top.get('versionInfo')!r} != OpenSankore.pro version {pro_version!r}"
        )

    # Drift detection between THIRD_PARTY.md and the SBOM.
    sbom_names = {p.get("name") for p in packages if p.get("SPDXID") != "SPDXRef-Package-OpenSankore"}
    tp_names = third_party_component_names()
    expected_from_tp = {NAME_MAP[n] for n in tp_names}

    missing_in_sbom = expected_from_tp - sbom_names
    for n in sorted(missing_in_sbom):
        errors.append(f"Component in THIRD_PARTY.md but missing from SBOM: {n}")

    known_sbom = set(NAME_MAP.values())
    extra_in_sbom = (sbom_names & known_sbom) - expected_from_tp
    for n in sorted(extra_in_sbom):
        errors.append(f"Component in SBOM but missing from THIRD_PARTY.md tables: {n}")

    if errors:
        print("SBOM check FAILED:")
        for e in errors:
            print(f"  - {e}")
        return 1

    print(f"SBOM check OK: {len(packages)} packages, version {top.get('versionInfo')}, no drift.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
