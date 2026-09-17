# ADR-0004: The release version is driven by the git tag

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** `release.yml`, `scripts/check-sbom.py`, dev-workflow steering

> Recorded retroactively.

## Context

A release version (`X.Y.Z`) must appear consistently in the binary
(`UBVERSION`), the `.deb`/`.rpm`/`.exe` metadata, the Windows installer
(`installer.iss`), and the SBOM (`sbom.spdx.json`). Hand-editing the version in
several files before each release is error-prone and produces noisy "prepare
release" commits that can drift out of sync.

## Decision

The **git tag `vX.Y.Z` is the single source of truth** for the version. Pushing
the tag triggers `release.yml`, which derives `X.Y.Z` from `GITHUB_REF_NAME`,
dispatches the platform builds **on the tag**, patches the version into
`OpenSankore.pro` / `installer.iss` / `sbom.spdx.json` **in the runner checkout
only** (never committed), and publishes the release with all assets labelled
`X.Y.Z`. Between releases, `OpenSankore.pro` and `sbom.spdx.json` carry `0.0.0`
(shown as `0.0.0-dev`); a PR/master build is not an official release.

## Consequences

- Releasing is one action: `git tag vX.Y.Z && git push origin vX.Y.Z`. No version
  bump commit, no manual file edits.
- Release builds are never reused from a branch/master build (those carry the dev
  version `0.0.0`); `release.yml` rebuilds on the tag.
- The committed SBOM must stay consistent with `OpenSankore.pro` (`sbom-check.yml`
  enforces it); resync with `python3 scripts/check-sbom.py --set-version X.Y.Z`.
- Tagging is a release action: only on the maintainer's explicit request (same
  rule as merges).

## Alternatives considered

- **Hand-bump the version in files + commit before release** — rejected:
  error-prone, noisy, easy to desync across the .pro/installer/SBOM.
- **Version from a `VERSION` file** — rejected: still a manual edit to keep in
  sync; the tag is already the canonical release marker in git.
