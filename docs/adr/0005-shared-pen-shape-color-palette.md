# ADR-0005: Shared color palette between the pen and shape tools

- **Status:** Accepted
- **Date:** 2026-09-16
- **Deciders:** maintainer (David Guyomarch)
- **Related:** #319; drawing-model steering; `DrawingPropsBar.qml`, `UBToolController`

> Recorded retroactively.

## Context

The QML V2 `DrawingPropsBar` shows colors/widths for the active tool. Shapes
(the Drawing tool) and the pen both need a color choice. We had to decide whether
shapes get their own dedicated palette or reuse the pen's.

## Decision

Shapes and the pen **share the same color palette** (no dedicated shape palette).
`DrawingPropsBar` binds the tool-aware `UBToolController` properties
(`currentColors`, `currentColorIndex`, `currentWidthIndex`); in Drawing mode
`currentColors` returns `penColors()` and the index routes to
`shapeFactory().setStrokeColor(...)`. Palette index 0 is pure black (`#000000`)
on a light background and pure white (`#FFFFFF`) on a dark one, which pairs with
`UBInkColors::recoloredDefaultInk` so a freshly drawn shape follows the
background.

## Consequences

- One consistent color set across pen and shapes; less UI and less state.
- Shape color/width edits target the **next** shape (stored in the factory and
  read by `instanciateCurrentShape`), since a shape is deselected after drawing
  (#319). Editing an existing shape requires re-selecting it with the Selector.
- Do **not** reintroduce an "apply to last drawn shape" fallback in the factory
  setters — it silently recolored the previous shape (bug fixed in #319).
- Shapes store a single color (not a light/dark pair); night/day handling relies
  on `recoloredDefaultInk` flipping only the pure black/white default.

## Alternatives considered

- **A dedicated shape color palette** — rejected: more UI and duplicated state
  for no clear pedagogical benefit; users expect one color chooser.
- **Apply palette changes to the last drawn shape** — rejected: caused silent
  recoloring of the previous shape (#319).
