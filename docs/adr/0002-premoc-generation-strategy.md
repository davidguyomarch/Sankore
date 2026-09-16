# ADR-0002: Pre-generate and commit moc files for the test target (premoc)

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #229, #244, #297; steering `moc-premoc.md`

> Recorded retroactively. Captures the standing strategy behind `tests/premoc/`.

## Context

On Linux, the system `moc` cannot parse GCC system headers when qmake passes it
`--include moc_predefs.h` (it pulls `<type_traits>` etc.), emitting "No relevant
classes found" or crashing. This breaks only the **test target**
(`tests/tests.pro`), which compiles `QObject`s under test plus test classes with
`private slots`. The application build is unaffected.

## Decision

We will pre-generate the moc for every `QObject` in the test target **without**
`moc_predefs.h`, write it to `tests/premoc/moc_*.cpp`, and **commit those files
to git**. `scripts/docker-build.sh` is the single source of truth that generates
them locally (it also strips system `-I` paths from the moc commands in the
Makefile). The CI consumes the committed `tests/premoc/` files rather than
regenerating them.

## Consequences

- The CI compiles exactly what was generated and verified locally — deterministic,
  no "works on my machine".
- **Trap:** a new `QObject` test whose premoc is not committed passes locally
  (docker-build.sh regenerates it) but fails CI (missing/stale moc → link error).
  `tests/premoc/` is therefore tracked in git and must never be gitignored. The
  full procedure is in steering `moc-premoc.md`.
- Follow-up (separate issue): homogenize by making CI regenerate **nothing**
  (today it still regenerates `UBFileSystemUtils` and `UBOEmbedParser`), and add
  a CI guard that runs the generator and `git diff --exit-code tests/premoc/` to
  catch a stale/missing committed premoc.

## Alternatives considered

- **Regenerate all moc in the CI** — rejected: the CI Linux environment is
  exactly where moc fails; reproducing the docker-build workaround in every
  workflow is fragile and duplicated, for a purely derived, deterministic artifact.
- **Do nothing / rely on qmake auto-moc** — rejected: that is what fails on Linux.
- **Switch the whole build to CMake** — out of scope; the project uses qmake
  (`.pro`/`.pri`), and this would be a much larger decision of its own.
