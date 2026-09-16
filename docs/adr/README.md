# Architecture Decision Records

This directory records the **structural decisions** of Open-Sankoré: choices that
constrain the future and that a contributor should be able to find and understand
months later. Format is [MADR](https://adr.github.io/madr/)-lite, one Markdown
file per decision.

## Conventions

- One decision per file: `docs/adr/NNNN-title-in-kebab-case.md` (sequential `NNNN`).
- Copy [`0000-template.md`](0000-template.md) to start a new ADR.
- **Status:** `Proposed` → `Accepted` → optionally `Superseded by ADR-XXXX`.
- ADRs are **immutable once Accepted**. To reverse a decision, write a *new* ADR
  and mark the old one `Superseded by ADR-XXXX` — never delete or rewrite it.
- Language: **English** (like issues, PRs, and commit messages).

## When to write an ADR

Structural decisions only: architecture or technology choices, file-format or
public-API changes, dropping/replacing a module, deliberate perf/security
tradeoffs. Not implementation details. See the workflow in
`.kiro/steering/adr.md`.

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [0001](0001-qml-v2-ui-migration.md) | Migrate the UI to a QML V2 interface | Accepted |
| [0002](0002-premoc-generation-strategy.md) | Pre-generate and commit moc files for the test target | Accepted |
