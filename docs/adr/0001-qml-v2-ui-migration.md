# ADR-0001: Migrate the UI to a QML V2 interface

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #121 (QML V2 UI), #131 (legacy inventory), #154 (palette removal)

> Recorded retroactively. This decision predates the ADR system (#346); it is
> captured here because it is the directing architectural choice of the current
> codebase.

## Context

Open-Sankoré was a Qt 4.8 application whose UI was built from QWidget-based
floating and dock palettes (`UBStylusPalette`, `UBDrawingPalette`,
`UBNavigatorPalette`, `UBDesktopPalette`, `UBLeftPalette`/`UBRightPalette`…),
driven by a web of `QAction`/`QActionGroup` and controllers like
`UBDrawingController`. After the migration to Qt 6, that UI layer was hard to
theme, hard to test, and carried a lot of dead Qt4-era code.

## Decision

We will rebuild the interactive UI as a **QML V2 layer** (v4.1.0+), hosted in
`QQuickWidget`s and driven by dedicated QML controllers
(`UBToolController`, `UBPageController`, `UBAppController`, `UBThemeManager`),
with QML files under `src/qml/` (`StylusPaletteV2`, `TopBar`, `PageNavigator`,
`DrawingPropsBar`, `ShapesPaletteV2`, …). Tool state binds directly to the
controllers (no `QAction`/`QActionGroup`). Legacy QWidget palettes are removed
as each area is ported.

## Consequences

- Theming is centralized in `UBThemeManager` (see ADR on UI theming / #297) and
  bindings re-evaluate on theme change.
- Controllers are `QObject`s exposed to QML via `setContextProperty`, and their
  logic is unit-testable headless (subject to the moc/premoc workaround).
- The port is **incremental**: QML and legacy widgets coexist until each area is
  migrated. Some areas remain on the old design or are only partially wired
  (e.g. the Library view, #258; Documents view, #285).
- Removing legacy palettes is ongoing cleanup work (#154, #343).

## Alternatives considered

- **Keep the QWidget UI, port it 1:1 to Qt 6** — rejected: preserves the hard-to-
  theme, hard-to-test QAction-driven design and its dead code.
- **Full rewrite in another toolkit** — rejected: throws away a large, working
  codebase and the Qt investment for no proportional benefit.
