# ADR-0006: Unify the Desktop-mode toolbar on QML V2 (dedicated DesktopToolbar)

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #336 (PR #342); desktop-mode steering; ADR-0001

> Recorded retroactively.

## Context

The Desktop annotation mode (transparent full-screen overlay over the desktop)
still used the legacy `UBDesktopPalette` (a `UBActionPalette`/`UBFloatingPalette`
with `QAction`s and pen/marker/eraser hold-timer property popups), while the rest
of the UI had moved to QML V2 (ADR-0001). Two ways to unify it were considered:
(A) reuse the board's `StylusPaletteV2` instance on the overlay, or (B) build a
dedicated QML toolbar for the desktop.

## Decision

Build a **dedicated `DesktopToolbar.qml`** (option B), styled like
`StylusPaletteV2` (Phosphor icons + `themeManager`), hosted in a `QQuickWidget`
parented to `mTransparentDrawingView` and wired to the `UBToolController`
singleton. Actions: Pen, Eraser, Marker, Selector, Pointer | capture zone,
capture screen | back to board. Tool buttons bind to `toolController.activeTool`;
capture / return actions call slots on `UBDesktopAnnotationController` (exposed as
a `desktopController` context object). The legacy `UBDesktopPalette` and its
property-popup machinery are removed; color/width is handled by the shared
`DrawingPropsBar`.

## Consequences

- Consistent V2 look/theming in desktop mode; large deletion of legacy QAction /
  hold-timer / property-palette code.
- The overlay's click mask (`desktopPalettePath()`) now derives from the QML
  toolbar geometry.
- Desktop-mode behavior (overlay, transparency, captures) is not unit-testable
  headless; validated on the Windows VM. See the desktop-mode steering.

## Alternatives considered

- **Option A — reuse `StylusPaletteV2` on the overlay** — rejected: the board
  palette is parented to the board container (hidden in desktop mode), carries
  board-only tools/actions, and coupling the two contexts is messier than a small
  dedicated toolbar with exactly the desktop actions.
- **Keep `UBDesktopPalette`** — rejected: leaves a legacy QAction/QWidget island
  inconsistent with the V2 UI (ADR-0001) and hard to theme.
