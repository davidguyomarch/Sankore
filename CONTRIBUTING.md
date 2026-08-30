# Contributing to Open-Sankoré Community Edition

Thank you for your interest in contributing.

## Getting started

1. Check the [issue tracker](https://github.com/davidguyomarch/Sankore/issues)
   for open issues or create a new one describing your proposed change.
2. Fork the repository and create a branch from `master`:
   - `fix/<issue-id>-<description>` for bug fixes
   - `feat/<issue-id>-<description>` for new features
3. Make your changes and validate locally (see below).
4. Open a pull request against `master`.

## Development setup

The project builds with C++17, Qt 6, and qmake. Development is done on macOS
or Linux using Docker for compilation:

```bash
# Build the dev image (once)
docker build -f Dockerfile.dev -t sankore-dev .

# Full validation: build + tests + coverage
./scripts/docker-build.sh
```

See [`.kiro/steering/dev-workflow.md`](.kiro/steering/dev-workflow.md) for the
complete workflow.

## Before submitting a pull request

- **Build passes** — `./scripts/docker-build.sh --build-only` succeeds
- **Tests pass** — `./scripts/docker-build.sh --test-only` shows 0 failures
- **No coverage regression** — coverage stays above 80% on tested modules
- **Commit messages** reference the issue: `fix(#123): description` or
  `feat(#123): description`

## Code style

- Follow the existing code conventions (Qt naming, camelCase methods, PascalCase
  classes).
- New source files should include the SPDX header:
  ```
  SPDX-License-Identifier: GPL-3.0-only
  ```
- QML V2 UI uses [Phosphor Icons](https://phosphoricons.com) (MIT) — always use
  existing icons from `resources/icons/phosphor/` before adding new ones.

## Language

All GitHub content (issues, PRs, commits, comments) is written in **English**.

## License

By contributing, you agree that your contributions are licensed under the
[GNU General Public License v3](LICENSE.md).
