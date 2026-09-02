#!/usr/bin/env python3
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only
"""
Validate sbom.spdx.json against the project, and (with --set-version) make the
git tag the single source of truth for the project version.

Checks performed (validation mode, the default):
  1. sbom.spdx.json is well-formed JSON and declares SPDX-2.3.
  2. Every package has the required SPDX fields.
  3. The top-level package version matches VERSION_MAJ.VERSION_MIN.VERSION_PATCH
     read from OpenSankore.pro.
  4. Every component named in THIRD_PARTY.md tables also appears in the SBOM,
     and vice versa (drift detection).

With --set-version X.Y.Z (release mode), the script first propagates the version
into OpenSankore.pro (VERSION_MAJ/MIN/PATCH) and sbom.spdx.json (top-level name,
documentNamespace, and the OpenSankore package versionInfo), then validates. The
release workflow derives X.Y.Z from the pushed git tag and calls this, so no
version needs to live in the repository between releases — the tag is the single
source of truth and the in-repo values are just a dev fallback.

This is intentionally a *validation* tool, not a generator: the project has no
single build manifest an automatic scanner could rely on, so the SBOM is
curated by hand and this script keeps it honest.

Exit code 0 = OK, 1 = problem found.
"""

import argparse
import json
import re
import sys
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parent.parent

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


def read_pro_version(pro_path):
    text = pro_path.read_text(encoding="utf-8")

    def field(name):
        m = re.search(rf"^{name}\s*=\s*(\d+)", text, re.MULTILINE)
        return m.group(1) if m else None

    maj, minor, patch = field("VERSION_MAJ"), field("VERSION_MIN"), field("VERSION_PATCH")
    if None in (maj, minor, patch):
        errors.append("Could not parse VERSION_MAJ/MIN/PATCH from OpenSankore.pro")
        return None
    return f"{maj}.{minor}.{patch}"


def third_party_component_names(third_party_path):
    """Extract first-column values from the markdown tables in THIRD_PARTY.md."""
    names = set()
    for line in third_party_path.read_text(encoding="utf-8").splitlines():
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


VERSION_RE = re.compile(r"^\d+\.\d+\.\d+$")


def set_pro_version(pro_path, version):
    """Rewrite VERSION_MAJ/MIN/PATCH in OpenSankore.pro to match version."""
    maj, minor, patch = version.split(".")
    text = pro_path.read_text(encoding="utf-8")
    text = re.sub(r"(?m)^VERSION_MAJ\s*=.*$", f"VERSION_MAJ = {maj}", text)
    text = re.sub(r"(?m)^VERSION_MIN\s*=.*$", f"VERSION_MIN = {minor}", text)
    text = re.sub(r"(?m)^VERSION_PATCH\s*=.*$", f"VERSION_PATCH = {patch}", text)
    pro_path.write_text(text, encoding="utf-8")


def set_sbom_version(sbom_path, version):
    """Rewrite the version-bearing fields of sbom.spdx.json in place.

    Only the document-level name/namespace and the OpenSankore package's
    versionInfo carry the project version; third-party package versions are
    left untouched.

    Uses targeted textual substitution rather than a JSON round-trip so the
    file's existing formatting (compact externalRefs, non-ASCII accents) is
    preserved and the diff stays minimal. The result is validated as JSON by
    the caller.
    """
    text = sbom_path.read_text(encoding="utf-8")

    text, n_name = re.subn(
        r'("name":\s*")Open-Sankore-Community-Edition-[0-9][^"]*(")',
        rf"\g<1>Open-Sankore-Community-Edition-{version}\g<2>",
        text,
        count=1,
    )
    text, n_ns = re.subn(
        r'("documentNamespace":\s*"https://github\.com/davidguyomarch/Sankore/spdx/open-sankore-)[0-9][^"]*(")',
        rf"\g<1>{version}\g<2>",
        text,
        count=1,
    )
    # The OpenSankore package's versionInfo sits a few lines below its SPDXID.
    text, n_ver = re.subn(
        r'("SPDXID":\s*"SPDXRef-Package-OpenSankore",\s*"versionInfo":\s*")[^"]*(")',
        rf"\g<1>{version}\g<2>",
        text,
        count=1,
    )

    if not (n_name and n_ns and n_ver):
        errors.append(
            "set_sbom_version: expected to patch name/documentNamespace/versionInfo "
            f"(matched name={n_name}, namespace={n_ns}, versionInfo={n_ver})"
        )
        return

    sbom_path.write_text(text, encoding="utf-8")


def main(argv=None):
    parser = argparse.ArgumentParser(description="Validate (and optionally version-stamp) the SBOM.")
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_ROOT,
        help="Project root containing sbom.spdx.json, OpenSankore.pro, THIRD_PARTY.md (default: repo root).",
    )
    parser.add_argument(
        "--set-version",
        metavar="X.Y.Z",
        help="Before validating, propagate this version into OpenSankore.pro and sbom.spdx.json. "
        "The release workflow passes the version derived from the git tag.",
    )
    args = parser.parse_args(argv)

    root = args.root
    sbom_path = root / "sbom.spdx.json"
    pro_path = root / "OpenSankore.pro"
    third_party_path = root / "THIRD_PARTY.md"

    if not sbom_path.exists():
        print(f"ERROR: {sbom_path} does not exist")
        return 1

    if args.set_version is not None:
        if not VERSION_RE.match(args.set_version):
            print(f"ERROR: --set-version expects X.Y.Z (got {args.set_version!r})")
            return 1
        set_pro_version(pro_path, args.set_version)
        set_sbom_version(sbom_path, args.set_version)
        print(f"Stamped version {args.set_version} into OpenSankore.pro and sbom.spdx.json")

    try:
        sbom = json.loads(sbom_path.read_text(encoding="utf-8"))
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
    pro_version = read_pro_version(pro_path)
    top = next((p for p in packages if p.get("SPDXID") == "SPDXRef-Package-OpenSankore"), None)
    if top is None:
        errors.append("No top-level package SPDXRef-Package-OpenSankore in SBOM")
    elif pro_version and top.get("versionInfo") != pro_version:
        errors.append(
            f"SBOM top-level version {top.get('versionInfo')!r} != OpenSankore.pro version {pro_version!r}"
        )

    # Drift detection between THIRD_PARTY.md and the SBOM.
    sbom_names = {p.get("name") for p in packages if p.get("SPDXID") != "SPDXRef-Package-OpenSankore"}
    tp_names = third_party_component_names(third_party_path)
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
