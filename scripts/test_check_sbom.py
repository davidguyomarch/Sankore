#!/usr/bin/env python3
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only
"""
Unit tests for scripts/check-sbom.py, focused on the --set-version mode that
makes the git tag the single source of truth for the project version.

Stdlib only (unittest), no pytest dependency, so it runs anywhere python3 is
available (CI, Docker dev image, local).

Run:
    python3 scripts/test_check_sbom.py
"""

import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
CHECK_SBOM = SCRIPTS_DIR / "check-sbom.py"
ROOT = SCRIPTS_DIR.parent

# A minimal but representative SBOM: top-level name/namespace carry the version,
# the OpenSankore package carries versionInfo, and a second package (zlib) must
# NOT be touched by a version bump.
SAMPLE_SBOM = {
    "spdxVersion": "SPDX-2.3",
    "dataLicense": "CC0-1.0",
    "SPDXID": "SPDXRef-DOCUMENT",
    "name": "Open-Sankore-Community-Edition-4.2.0",
    "documentNamespace": "https://github.com/davidguyomarch/Sankore/spdx/open-sankore-4.2.0",
    "packages": [
        {
            "name": "Open-Sankore-Community-Edition",
            "SPDXID": "SPDXRef-Package-OpenSankore",
            "versionInfo": "4.2.0",
            "downloadLocation": "https://github.com/davidguyomarch/Sankore",
            "licenseConcluded": "GPL-3.0-only",
            "licenseDeclared": "GPL-3.0-only",
        },
        {
            "name": "zlib",
            "SPDXID": "SPDXRef-Package-zlib",
            "versionInfo": "1.3.1",
            "downloadLocation": "https://zlib.net/",
            "licenseConcluded": "Zlib",
            "licenseDeclared": "Zlib",
        },
    ],
}

SAMPLE_PRO = """\
TARGET = "Open-Sankore"

VERSION_MAJ = 4
VERSION_MIN = 2
VERSION_TYPE = r # a = alpha, b = beta, r = release, other => error
VERSION_PATCH = 0

VERSION = "$${VERSION_MAJ}.$${VERSION_MIN}.$${VERSION_TYPE}.$${VERSION_PATCH}"
"""


def run_check(args, cwd):
    """Run check-sbom.py with the given args in cwd; return (rc, stdout+stderr)."""
    proc = subprocess.run(
        [sys.executable, str(CHECK_SBOM), *args],
        cwd=str(cwd),
        capture_output=True,
        text=True,
    )
    return proc.returncode, proc.stdout + proc.stderr


class SetVersionTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        # check-sbom.py resolves paths relative to its own location (ROOT), so we
        # can't just drop files in a temp dir. Instead we run it with --root
        # pointing at the temp dir, which the implementation must support.
        (self.tmp / "sbom.spdx.json").write_text(
            json.dumps(SAMPLE_SBOM, indent=2), encoding="utf-8"
        )
        (self.tmp / "OpenSankore.pro").write_text(SAMPLE_PRO, encoding="utf-8")
        # A THIRD_PARTY.md with just the two components present in the sample SBOM,
        # so drift detection stays happy.
        (self.tmp / "THIRD_PARTY.md").write_text(
            "| Component | Version |\n"
            "|-----------|---------|\n"
            "| zlib | 1.3.1 |\n",
            encoding="utf-8",
        )

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _pro(self):
        return (self.tmp / "OpenSankore.pro").read_text(encoding="utf-8")

    def _sbom(self):
        return json.loads((self.tmp / "sbom.spdx.json").read_text(encoding="utf-8"))

    def test_set_version_patches_pro(self):
        rc, out = run_check(["--root", str(self.tmp), "--set-version", "4.3.0"], self.tmp)
        self.assertEqual(rc, 0, out)
        pro = self._pro()
        self.assertIn("VERSION_MAJ = 4", pro)
        self.assertIn("VERSION_MIN = 3", pro)
        self.assertIn("VERSION_PATCH = 0", pro)

    def test_set_version_patches_sbom_top_level_and_package(self):
        rc, out = run_check(["--root", str(self.tmp), "--set-version", "4.3.0"], self.tmp)
        self.assertEqual(rc, 0, out)
        sbom = self._sbom()
        self.assertEqual(sbom["name"], "Open-Sankore-Community-Edition-4.3.0")
        self.assertEqual(
            sbom["documentNamespace"],
            "https://github.com/davidguyomarch/Sankore/spdx/open-sankore-4.3.0",
        )
        top = next(p for p in sbom["packages"] if p["SPDXID"] == "SPDXRef-Package-OpenSankore")
        self.assertEqual(top["versionInfo"], "4.3.0")

    def test_set_version_does_not_touch_other_packages(self):
        rc, out = run_check(["--root", str(self.tmp), "--set-version", "4.3.0"], self.tmp)
        self.assertEqual(rc, 0, out)
        sbom = self._sbom()
        zlib = next(p for p in sbom["packages"] if p["SPDXID"] == "SPDXRef-Package-zlib")
        self.assertEqual(zlib["versionInfo"], "1.3.1")

    def test_set_version_then_validation_passes(self):
        # After --set-version, a plain validation run must succeed (SBOM == .pro).
        rc, out = run_check(["--root", str(self.tmp), "--set-version", "4.3.0"], self.tmp)
        self.assertEqual(rc, 0, out)
        rc2, out2 = run_check(["--root", str(self.tmp)], self.tmp)
        self.assertEqual(rc2, 0, out2)

    def test_validation_fails_on_version_mismatch(self):
        # Sanity: with SBOM at 4.2.0 and .pro bumped to 4.3.0 by hand, validation fails.
        pro = self._pro().replace("VERSION_MIN = 2", "VERSION_MIN = 3")
        (self.tmp / "OpenSankore.pro").write_text(pro, encoding="utf-8")
        rc, out = run_check(["--root", str(self.tmp)], self.tmp)
        self.assertNotEqual(rc, 0, out)


if __name__ == "__main__":
    unittest.main(verbosity=2)
